#pragma once

struct LoadedClassInfo;
class Rules;

struct MethodDef {
    MethodDef() {
        bodyUpdated = false;
        methodDef = 0;
        instrumentedBody = 0;
    }   

    bool                bodyUpdated;
    mdMethodDef         methodDef;
    String        methodDefName;

    String        methodSig;
    DWORD               flags;
    DWORD               implFlags;

    InstrumentedILBody* instrumentedBody;
};
typedef stdext::hash_map<mdMethodDef, MethodDef> MethodDefMap;
typedef std::pair<mdMethodDef, MethodDef> MethodDefMapPair;

struct TypeDef {
    mdTypeDef    typeDef;
    String typeDefName;
    String typeDefNamespace;

    DWORD flags;

    MethodDefMap methodDefs;

    void swap(TypeDef& source) {
        typeDef = source.typeDef;
		flags = source.flags;
        typeDefName.swap(source.typeDefName);
        typeDefNamespace.swap(source.typeDefNamespace);
        methodDefs.swap(source.methodDefs);
    }
};
typedef stdext::hash_map<mdTypeDef, TypeDef> TypedefDescriptorMap;
typedef std::pair<mdTypeDef, TypeDef> TypedefDescriptorMapPair;

struct ModuleDescriptor {
    ModuleID     module;
    String moduleName;
    AssemblyID   assembly;
    String assemblyName;
    bool         loaded;

    CComPtr<ISymUnmanagedReader> symReader;

    TypedefDescriptorMap typeDefs;
};
typedef std::vector<ModuleDescriptor> ModuleDescriptors;

struct InstrumentHelper {
    ICorProfilerInfo* profilerInfo;
    ModuleDescriptor* module;
    IMetaDataImport* mdImport;
};

class InstrumentResults;

class Instrumentator
{
    CCriticalSection m_cs;

    void Lock() { m_cs.Enter(); }
    void Unlock() { m_cs.Leave(); }

    Rules& m_rules;

    ModuleDescriptors m_descriptors;

    void InstrumentTypedef(mdTypeDef typeDef, InstrumentHelper& helper);
    void InstrumentMethod(TypeDef& typeDef, mdMethodDef methodDef, InstrumentHelper& helper);

    ULONG32 GetFileUrlId(const String&);

    ModuleDescriptor* GetModuleDescriptor(ModuleID assembly);

public:
    Instrumentator(Rules& rules);
    ~Instrumentator(void);

    void InstrumentModule(ModuleID module, const String& moduleName, ICorProfilerInfo* profilerInfo, ISymUnmanagedBinder2* binder);
    void UnloadModule(ModuleID module);

    void UpdateClassCode(ClassID classId, ICorProfilerInfo* profilerInfo, ISymUnmanagedBinder2* binder);

    void StoreResults(InstrumentResults&);
};
