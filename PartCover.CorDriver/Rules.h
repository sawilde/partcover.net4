#pragma once

typedef std::list<std::wstring> StringArray;

namespace RulesHelpers {
    std::wstring ExtractNamespace(const std::wstring& className);
}

class Rules : IResultContainer
{
    bool enable_coverage_profile;
    bool enable_call_tree_profile;

    ProfilerMode profiler_level;

    StringArray m_excludeItems;
    StringArray m_includeItems;

    StringArray m_includeRules;
    StringArray m_excludeRules;

    void PopArray(Message::byte_array::iterator& it, StringArray&);
    void PushArray(Message::byte_array& values, StringArray&);

public:
    Rules(void);
    ~Rules(void);

    void SendResults(MessageCenter&);
    bool ReceiveResults(Message&);

    void EnableMode(const ProfilerMode& mode);
    bool IsEnabledMode(const ProfilerMode& mode) const;

    void IncludeItem(const std::wstring& item);
    void ExcludeItem(const std::wstring& item);

    void Dump() const;

    bool IsItemValidForReport(const std::wstring& assembly, const std::wstring& className) const;

    void PrepareItemRules();

    static bool CreateRuleFromItem(const std::wstring& item, std::wstring* rule);
};
