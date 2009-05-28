#pragma once

struct SkippedTypedef {
    String assemblyName;
	String typedefName;
};
typedef std::vector<SkippedTypedef> SkippedTypedefs;

struct LoadedClassInfo;
class Rules;

struct MethodDef {
    MethodDef() {
        bodyUpdated = false;
        methodDef = 0;
    }   

    mdMethodDef         methodDef;
    bool                bodyUpdated;
	InstrumentedBlocks  bodyBlocks;

	String				methodName;
	String				methodSig;
};

typedef stdext::hash_map<mdMethodDef, MethodDef> MethodDefMap;
typedef std::pair<mdMethodDef, MethodDef> MethodDefMapPair;

struct TypeDef {
    mdTypeDef    typeDef;
    MethodDefMap methodDefs;

	String       fullName;

    void swap(TypeDef& source) {
        typeDef = source.typeDef;
		fullName = source.fullName;
        methodDefs.swap(source.methodDefs);
    }
};

typedef stdext::hash_map<mdTypeDef, TypeDef> TypedefDescriptorMap;
typedef stdext::hash_map<AppDomainID, int> AppDomainIndexMap;
typedef std::pair<mdTypeDef, TypeDef> TypedefDescriptorMapPair;

struct ModuleDescriptor {
    ModuleID     module;
	String       moduleName;

    AssemblyID   assembly;
	String       assemblyName;

	int			 domain;
	String       domainName;

    bool         loaded;

    CComPtr<ISymUnmanagedReader> symReader;

    TypedefDescriptorMap typeDefs;
};
typedef std::vector<ModuleDescriptor> ModuleDescriptors;

struct InstrumentHelper {
    ICorProfilerInfo* profilerInfo;
    ModuleDescriptor* module;
    IMetaDataImport*  mdImport;
};

struct StoreHelper {
    ICorProfilerInfo* profilerInfo;
    ModuleDescriptor* module;
};

class InstrumentResults;

struct LockGuard 
{
	CCriticalSection& m_cs;

	LockGuard(CCriticalSection& cs) : m_cs(cs) 
	{
		m_cs.Enter();
	}

	~LockGuard() 
	{
		m_cs.Leave();
	}
};

class Instrumentator
{
    CCriticalSection m_cs;

    LockGuard Lock() { return LockGuard(m_cs); }

    Rules& m_rules;
    ModuleDescriptors m_descriptors;
	SkippedTypedefs m_skippedItems;

	AppDomainIndexMap m_domains;
	int m_nextDomainIndex;

    void InstrumentTypedef(mdTypeDef typeDef, InstrumentHelper& helper);
    void InstrumentMethod(TypeDef& typeDef, mdMethodDef methodDef, InstrumentHelper& helper, const String& typedefName);

    ULONG32 GetFileUrlId(const String&);
    ModuleDescriptor* GetModuleDescriptor(ModuleID assembly);

public:
    Instrumentator(Rules& rules);
    ~Instrumentator(void);

	bool IsAssemblyAcceptable(const String& assemblyName) const;

    void InstrumentModule(ModuleID module, const String& moduleName, ICorProfilerInfo* profilerInfo, ISymUnmanagedBinder2* binder);
    void UnloadModule(ModuleID module);

    void UpdateClassCode(ClassID classId, ICorProfilerInfo* profilerInfo, ISymUnmanagedBinder2* binder);

    void StoreResults(InstrumentResults&, ICorProfilerInfo* info);

	void AddSkippedAssembly(const String& assemblyName);
	void AddSkippedTypedef(const String& assemblyName, const String& typedefName);

	void ActivateAppDomain(AppDomainID domain);
	void DeactivateAppDomain(AppDomainID domain);
	int GetAppDomainIndex(AppDomainID domain);
};
