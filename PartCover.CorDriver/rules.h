#pragma once

typedef std::list<String> StringArray;

namespace RulesHelpers {
    String ExtractNamespace(const String& className);
}

class Rules : public ITransferrable
{
    bool enable_coverage_profile;
    bool enable_call_tree_profile;

    ProfilerMode profiler_level;

    StringArray m_excludeItems;
    StringArray m_includeItems;

    StringArray m_includeRules;
    StringArray m_excludeRules;

public:
    Rules(void);
    ~Rules(void);

	MessageType getType() const { return Messages::C_Rules; }
	void visit(ITransferrableVisitor& visitor) { visitor.on(*this); }
    bool SendData(MessagePipe&);
    bool ReceiveData(MessagePipe&);

    void EnableMode(const ProfilerMode& mode);
    bool IsEnabledMode(const ProfilerMode& mode) const;

    void IncludeItem(const String& item);
    void ExcludeItem(const String& item);

    void Dump() const;

    bool IsItemValidForReport(const String& assembly, const String& className, const mdTypeDef typeDef, IMetaDataImport *mdImport) const;

    void PrepareItemRules();

    static bool CreateRuleFromItem(const String& item, String* rule);
};
