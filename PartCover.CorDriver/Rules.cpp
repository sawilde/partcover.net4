#include "StdAfx.h"
#include "interface.h"
#include "MessageCenter.h"
#include "rules.h"
#include "DriverLog.h"

#include <atlrx.h>
typedef CAtlRECharTraitsW RegExpCharTraits;

namespace RulesHelpers {
    std::wstring ExtractNamespace(const std::wstring& className) {
        std::wstring::size_type namespaceEnd = className.find_last_of(L'.');
        if (namespaceEnd == std::wstring::npos)
            return std::wstring();

        std::wstring namespaceName = className.substr(0, namespaceEnd);
        if (namespaceName.length() == 0)
            return std::wstring();

        return namespaceName;
    }
}

Rules::Rules(void)
{
    enable_coverage_profile = false;
    enable_call_tree_profile = false;
    profiler_level = COVERAGE_USE_ASSEMBLY_LEVEL;
}

Rules::~Rules(void)
{
}

void Rules::SendResults(MessageCenter& center) {
    Message message;
    message.code = eResult;
    Message::byte_array& value = message.value;
    value.push_back(eRules);

    Message::push(value, enable_coverage_profile);
    Message::push(value, enable_call_tree_profile);
    Message::push(value, profiler_level);

    PushArray(value, m_includeItems);
    PushArray(value, m_excludeItems);

    ATLTRACE("PartCoverConnector::StartTarget - sending rules");
    center.SendOption(message);
}

bool Rules::ReceiveResults(Message& message) {
    typedef unsigned char* lpubyte;
    if (message.value.size() == 0 || message.value.front() != eRules)
        return false;

    Message::byte_array::iterator it = message.value.begin();
    size_t skipSize = 1;
    while(skipSize-- > 0) ++it;

    it = Message::pop(it, &enable_coverage_profile);
    it = Message::pop(it, &enable_call_tree_profile);
    it = Message::pop(it, &profiler_level);

    PopArray(it, m_includeItems);
    PopArray(it, m_excludeItems);

    PrepareItemRules();

    return true;
}

void Rules::PopArray(Message::byte_array::iterator& it, StringArray& array) {
    size_t arrSize;
    it = Message::pop(it, &arrSize);
    while(arrSize-- > 0) {
        std::wstring buffer = (const std::wstring::value_type*) &(*it);
        array.push_back(buffer);
        it += (buffer.length() + 1) * sizeof(std::wstring::value_type);
    }
}

void Rules::PushArray(Message::byte_array& values, StringArray& array) {
    typedef const Message::byte_array::value_type *value_ptr;
    Message::push(values, array.size());
    for(StringArray::const_iterator it = array.begin(); it != array.end(); ++it) {
        const std::wstring& buffer = *it;
        values.insert(values.end(), (value_ptr)buffer.c_str(), (value_ptr) buffer.c_str() + sizeof(std::wstring::value_type) * (buffer.length() + 1));
    }
}

void Rules::Dump() const {
    DriverLog& log = DriverLog::get();
    log.WriteLine(_T("  Count Coverage - %s"), enable_coverage_profile ? _T("ON") : _T("OFF"));
    log.WriteLine(_T("  Count Call Tree - %s"), enable_call_tree_profile ? _T("ON") : _T("OFF"));

    for(StringArray::const_iterator it = m_excludeItems.begin(); it != m_excludeItems.end(); ++it )
        log.WriteLine(_T("  Exclude %S"), it->c_str());
    for(StringArray::const_iterator it = m_includeItems.begin(); it != m_includeItems.end(); ++it )
        log.WriteLine(_T("  Include %S"), it->c_str());
    log.WriteLine(_T(""));
}

void Rules::EnableMode(const ProfilerMode& mode) {
    switch(mode) {
//        case COUNT_CALL_DIAGRAM: enable_call_tree_profile = true; break;
        case COUNT_COVERAGE: enable_coverage_profile = true; break;
        case COVERAGE_USE_ASSEMBLY_LEVEL:
        case COVERAGE_USE_CLASS_LEVEL: 
            profiler_level = mode; break;
        default: ATLTRACE("Mode %d is invalid", mode);
    }
}

bool Rules::IsEnabledMode(const ProfilerMode& mode) const {
    switch(mode) {
//        case COUNT_CALL_DIAGRAM: return enable_call_tree_profile;
        case COUNT_COVERAGE: return enable_coverage_profile;
        case COVERAGE_USE_ASSEMBLY_LEVEL: 
        case COVERAGE_USE_CLASS_LEVEL: 
            return profiler_level == mode;
        default: ATLTRACE("Mode %d is invalid", mode); return false;
    }
}

void Rules::IncludeItem(const std::wstring& item) {
    m_includeItems.push_back(item);
}

void Rules::ExcludeItem(const std::wstring& item) {
    m_excludeItems.push_back(item);
}

bool Rules::CreateRuleFromItem(const std::wstring& item, std::wstring* rule) {
    const wchar_t* metas = L".[]^?+(){}\\$|!";
    std::wstring res;
    for(std::wstring::const_iterator it = item.begin(); it != item.end(); ++it) {
        if (L'*' != *it) {
            if (NULL != wcschr(metas, *it))
                res += L'\\';
            res += *it;
        } else {
            res += L"(.*)";
        }
    }
    if (res.length() == 0)
        return false;

    res.insert(res.begin(), L'^');
    res.insert(res.end(), L'$');

    CAtlRegExp<RegExpCharTraits> re;
    if (REPARSE_ERROR_OK != re.Parse( res.c_str() ))
        return false;
    if (rule) 
        rule->swap(res);
    return rule ? rule->length() > 0 : res.length() > 0;
}

void Rules::PrepareItemRules() {
    std::wstring rule;
    for(StringArray::const_iterator it = m_includeItems.begin(); it != m_includeItems.end(); ++it) {
        if (CreateRuleFromItem(*it, &rule))
            m_includeRules.push_back(rule);
    }
    for(StringArray::const_iterator it = m_excludeItems.begin(); it != m_excludeItems.end(); ++it) {
        if (CreateRuleFromItem(*it, &rule))
            m_excludeRules.push_back(rule);
    }
}

bool IsTargetForRules(const std::wstring& target, const StringArray& rules) {
    for(StringArray::const_iterator it = rules.begin(); it != rules.end(); ++it) {
        CAtlRegExp<RegExpCharTraits> reg;
        if( REPARSE_ERROR_OK != reg.Parse( it->c_str() ) ) 
            continue;
        CAtlREMatchContext<RegExpCharTraits> mcUrl;
        if( reg.Match( target.c_str(), &mcUrl ) )
            return true;
    }
    return false;
}

bool Rules::IsItemValidForReport(const std::wstring& assembly, const std::wstring& className) const {
    std::wstring ruleTarget = L"[";
    ruleTarget += assembly;
    ruleTarget += L"]";
    ruleTarget += className;

    if( IsTargetForRules(ruleTarget, m_excludeRules) ) return false;
    if( IsTargetForRules(ruleTarget, m_includeRules) ) return true;
    return false;
}

