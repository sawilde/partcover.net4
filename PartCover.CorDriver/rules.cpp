#include "StdAfx.h"
#include "interface.h"
#include "message.h"
#include "message_pipe.h"
#include "rules.h"
#include "logging.h"
#include "corhelper.h"

#include <atlrx.h>
typedef CAtlRECharTraitsW RegExpCharTraits;
#define ATTRIBUTE_RULE_TAG _T("attribute:")

namespace RulesHelpers {
    String ExtractNamespace(const String& className) {
        String::size_type namespaceEnd = className.find_last_of(_T('.'));
        if (namespaceEnd == String::npos)
            return String();

        String namespaceName = className.substr(0, namespaceEnd);
        if (namespaceName.length() == 0)
            return String();

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

bool ReadArray(MessagePipe& pipe, StringArray& array) {
    size_t arrSize;
	if (!pipe.read(&arrSize))
		return false;

	for(size_t i = 0; i < arrSize; ++i)
	{
		String str;
		if (!pipe.read(&str))
			return false;
		array.push_back(str);
	}

	return true;
}

bool WriteArray(MessagePipe& pipe, StringArray& array) {
	if (!pipe.write(array.size()))
		return false;

    for(StringArray::const_iterator it = array.begin(); it != array.end(); ++it) {
		if (!pipe.write(*it))
			return false;
    }
	return true;
}

bool Rules::SendData(MessagePipe& pipe)
{
    ATLTRACE("PartCoverConnector::StartTarget - sending rules");

	int profiler_level = this->profiler_level;

	return 
		pipe.write(enable_coverage_profile) &&
		pipe.write(enable_call_tree_profile) &&
		pipe.write(profiler_level) &&
		WriteArray(pipe, m_includeItems) &&
		WriteArray(pipe, m_excludeItems);
}

bool Rules::ReceiveData(MessagePipe& pipe) {
	int profiler_level;

	bool result = 
		pipe.read(&enable_coverage_profile) &&
		pipe.read(&enable_call_tree_profile) &&
		pipe.read(&profiler_level) &&
		ReadArray(pipe, m_includeItems) &&
		ReadArray(pipe, m_excludeItems);


	if (!result) {
		return false;
	}
 
	this->profiler_level = static_cast<ProfilerMode>(profiler_level);
    PrepareItemRules();
    return true;
}


void Rules::Dump() const {
    DriverLog& log = DriverLog::get();
    log.WriteLine(_T("  Count Coverage - %s"), enable_coverage_profile ? _T("ON") : _T("OFF"));
    log.WriteLine(_T("  Count Call Tree - %s"), enable_call_tree_profile ? _T("ON") : _T("OFF"));

    for(StringArray::const_iterator it = m_excludeItems.begin(); it != m_excludeItems.end(); ++it )
        log.WriteLine(_T("  Exclude %s"), it->c_str());
    for(StringArray::const_iterator it = m_includeItems.begin(); it != m_includeItems.end(); ++it )
        log.WriteLine(_T("  Include %s"), it->c_str());
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

void Rules::IncludeItem(const String& item) {
    m_includeItems.push_back(item);
}

void Rules::ExcludeItem(const String& item) {
    m_excludeItems.push_back(item);
}

bool Rules::CreateRuleFromItem(const String& item, String* rule) {
	if (item.find(ATTRIBUTE_RULE_TAG) == 0)
	{
		String whole = item.substr();
		if (rule)
			rule->swap(whole);
		return true;
	}

    LPCTSTR metas = _T(".[]^?+(){}\\$|!");
    String res;
	for(String::const_iterator it = item.begin(); it != item.end(); ++it) {
        if (_T('*') != *it) {
            if (NULL != wcschr(metas, *it))
                res += _T('\\');
            res += *it;
        } else {
            res += _T("(.*)");
        }
    }
    if (res.length() == 0)
        return false;

    res.insert(res.begin(), _T('^'));
    res.insert(res.end(), _T('$'));

    CAtlRegExp<RegExpCharTraits> re;
    if (REPARSE_ERROR_OK != re.Parse( res.c_str() ))
        return false;
    if (rule) 
        rule->swap(res);
    return rule ? rule->length() > 0 : res.length() > 0;
}

void Rules::PrepareItemRules() {
    String rule;
    for(StringArray::const_iterator it = m_includeItems.begin(); it != m_includeItems.end(); ++it) {
        if (CreateRuleFromItem(*it, &rule))
            m_includeRules.push_back(rule);
    }
    for(StringArray::const_iterator it = m_excludeItems.begin(); it != m_excludeItems.end(); ++it) {
        if (CreateRuleFromItem(*it, &rule))
            m_excludeRules.push_back(rule);
    }
}

String GetAttributeTypeDefName(const mdToken attrib, IMetaDataImport *mdImport)
{
	mdToken attribDef;
	if (SUCCEEDED(mdImport->GetCustomAttributeProps(attrib, NULL, &attribDef, NULL, NULL)))
	{
		ULONG pchMethodName;
		DWORD flags;
		PCCOR_SIGNATURE sig;
		ULONG sigSize;
		ULONG codeRva;
		DWORD implFlags;
		mdToken attribTypeDef;
		switch (TypeFromToken(attribDef))
		{
			case mdtMethodDef:
				if (SUCCEEDED(mdImport->GetMethodProps(attribDef, &attribTypeDef, NULL, 0, &pchMethodName, 
					&flags, &sig, &sigSize, &codeRva, &implFlags)))
				{
					return CorHelper::GetTypedefFullName(mdImport, attribTypeDef, NULL);
				}
				break;
			case mdtMemberRef:
				CComPtr<IMetaDataImport> refImport;
				if (SUCCEEDED(mdImport->GetMemberRefProps(attribDef, &attribTypeDef, NULL, 0, 
					&pchMethodName, &sig, &sigSize)))
				{
					switch (TypeFromToken(attribTypeDef))
					{
						case mdtTypeRef:
							// TODO: does this work correctly with attributes that are inner classes?
							return CorHelper::TypeRefName(mdImport, attribTypeDef);
						case mdtTypeDef:
							return CorHelper::GetTypedefFullName(mdImport, attribTypeDef, NULL);
					}
				}
				break;
		}
	}
	return String();
}

bool DoesAttributeMatch(const mdToken attribDef, const String& rule, IMetaDataImport *mdImport)
{
	String attribTypeDefName = GetAttributeTypeDefName(attribDef, mdImport);
	return (rule.compare(attribTypeDefName) == 0);
}

bool DoAttributesMatch(const String& rule, const mdTypeDef typeDef, IMetaDataImport *mdImport)
{
	HCORENUM hEnum = 0;
	mdCustomAttribute attrib;
	ULONG bufferSize;
	bool found = false;
    while (SUCCEEDED(mdImport->EnumCustomAttributes(&hEnum, typeDef, 0, &attrib, 1, &bufferSize)) && bufferSize > 0)
	{
		if (DoesAttributeMatch(attrib, rule, mdImport))
		{
			found = true;
			break;
		}
    }
	mdImport->CloseEnum(hEnum);
	return found;
}

bool IsTargetForRules(const String& target, const StringArray& rules, 
					  const mdTypeDef typeDef, IMetaDataImport *mdImport) {
	// Process the non-attribute rules first, as they are faster to match.
    for (StringArray::const_iterator it = rules.begin(); it != rules.end(); ++it) {
		if (it->find(ATTRIBUTE_RULE_TAG) == 0)
			continue;
        CAtlRegExp<RegExpCharTraits> reg;
        if( REPARSE_ERROR_OK != reg.Parse( it->c_str() ) ) 
            continue;
        CAtlREMatchContext<RegExpCharTraits> mcUrl;
        if( reg.Match( target.c_str(), &mcUrl ) )
            return true;
    }
	for (StringArray::const_iterator it = rules.begin(); it != rules.end(); ++it) {
		if (it->find(ATTRIBUTE_RULE_TAG) == 0)
		{
			if (DoAttributesMatch(it->substr(wcslen(ATTRIBUTE_RULE_TAG)), typeDef, mdImport))
				return true;
		}
    }
    return false;
}

bool Rules::IsItemValidForReport(const String& assembly, const String& className, 
								 const mdTypeDef typeDef, IMetaDataImport *mdImport) const {
    String ruleTarget = _T("[");
    ruleTarget += assembly;
    ruleTarget += _T("]");
    ruleTarget += className;

    if( IsTargetForRules(ruleTarget, m_excludeRules, typeDef, mdImport) ) return false;
    if( IsTargetForRules(ruleTarget, m_includeRules, typeDef, mdImport) ) return true;
    return false;
}

