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
};

typedef stdext::hash_map<mdMethodDef, MethodDef> MethodDefMap;
typedef std::pair<mdMethodDef, MethodDef> MethodDefMapPair;

struct TypeDef {
    mdTypeDef    typeDef;
    MethodDefMap methodDefs;

    void swap(TypeDef& source) {
        typeDef = source.typeDef;
        methodDefs.swap(source.methodDefs);
    }
};

typedef stdext::hash_map<mdTypeDef, TypeDef> TypedefDescriptorMap;
typedef std::pair<mdTypeDef, TypeDef> TypedefDescriptorMapPair;

struct ModuleDescriptor {
    ModuleID     module;
    AssemblyID   assembly;
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

class InstrumentResults;

class Instrumentator
{
    CCriticalSection m_cs;

    void Lock() { m_cs.Enter(); }
    void Unlock() { m_cs.Leave(); }

    Rules& m_rules;
    ModuleDescriptors m_descriptors;
	SkippedTypedefs m_skippedItems;

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
};
