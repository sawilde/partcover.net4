#pragma once

#include "helpers.h"

struct ModuleAllocator : public ILHelpers::Allocator 
{
	DWORD* NewDword() { 
		DWORD* res = m_allocator.Alloc(1);
		*res = 0;
		return res;
	}

private:
	FieldAllocator<DWORD, ForwardAllocatorChunk> m_allocator;
};

struct SkippedTypedef {
    String assemblyName;
	String typedefName;
};
typedef std::vector<SkippedTypedef> SkippedTypedefs;

struct LoadedClassInfo;
class Rules;

struct MethodDef {
	MethodDef() : bodyBytes(0), bodySize(0), bodySeqCount(0), bodyLineCount(0), symbolEntryFound(false)
	{
        bodyUpdated = false;
        methodDef = 0;
    }   

    mdMethodDef         methodDef;

    bool                bodyUpdated;
	int                 bodySize;
    int                 bodySeqCount;
    int                 bodyLineCount;
	InstrumentedBlocks  bodyBlocks;
	DynamicArray<BYTE>  bodyBytes;
    bool                symbolEntryFound;

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

    bool            loaded;

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

	LockGuard(const LockGuard& cs) : m_cs(cs.m_cs) 
	{
		m_cs.Enter();
	}

	~LockGuard() 
	{
		m_cs.Leave();
	}
};

typedef std::map<String, ULONG32> FileUrlMap;
typedef std::pair<String, ULONG32> FileUrlMapPair;

class Instrumentator
{
	ModuleAllocator m_allocator;

    LockGuard Lock() { 
		static CCriticalSection m_cs;
		return LockGuard(m_cs); 
	}

    Rules& m_rules;
    ModuleDescriptors m_descriptors;
	SkippedTypedefs m_skippedItems;

	AppDomainIndexMap m_domains;
	int m_nextDomainIndex;

    void InstrumentTypedef(ModuleID module, mdTypeDef typeDef, InstrumentHelper& helper);
    void InstrumentMethod(ModuleID module, TypeDef& typeDef, mdMethodDef methodDef, InstrumentHelper& helper, const String& typedefName);

    ULONG32 GetFileUrlId(const String&);
    ModuleDescriptor* GetModuleDescriptor(ModuleID assembly);

	void ReplaceCode(ModuleDescriptor& module, TypeDef& defDescriptor, MethodDef& method, ICorProfilerInfo* profilerInfo);
	void GenerateILCode(ModuleDescriptor& module, TypeDef& defDescriptor, MethodDef& method, ICorProfilerInfo* profilerInfo);

	FileUrlMap FileMap;

public:
    Instrumentator(Rules& rules);
    ~Instrumentator(void);

	bool IsAssemblyAcceptable(const String& assemblyName) const;

    void InstrumentModule(ModuleID module, const String& moduleName, ICorProfilerInfo* profilerInfo, ISymUnmanagedBinder2* binder);
    void UnloadModule(ModuleID module);

    //void UpdateClassCode(ClassID classId, ICorProfilerInfo* profilerInfo, ISymUnmanagedBinder2* binder);
	void UpdateFunctionCode(FunctionID classId, ICorProfilerInfo2* profilerInfo, ISymUnmanagedBinder2* binder);

    void StoreResults(InstrumentResults&, ICorProfilerInfo* info);

	void AddSkippedAssembly(const String& assemblyName);
	void AddSkippedTypedef(const String& assemblyName, const String& typedefName);

	void ActivateAppDomain(AppDomainID domain);
	void DeactivateAppDomain(AppDomainID domain);
	int GetAppDomainIndex(AppDomainID domain);
};
