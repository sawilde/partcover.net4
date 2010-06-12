#include "StdAfx.h"
#include "interface.h"
#include "il_instrumentedbody.h"
#include "il_instrumentator.h"
#include "instrumented_results.h"
#include "logging.h"
#include "rules.h"
#include "corerror.h"
#include "helpers.h"
#include "corhelper.h"

//#define DUMP_TYPEDEFS
//#define DUMP_METHODDEFS
//#define DUMP_SYM_SEQUENCE_POINTS
//#define DUMP_INSTRUMENT_RESULT

void DumpTypeDef(DriverLog& log, DWORD typeDefFlags, LPCTSTR typedefName);
void DumpMethodDef(DriverLog& log, DWORD flags, LPCTSTR methoddefName, DWORD implFlag);
void DumpSymSequencePoints(ULONG32 cPoints, ULONG32 offsets[], ISymUnmanagedDocument *documents[], ULONG32 lines[], ULONG32 columns[], ULONG32 endLines[], ULONG32 endColumns[]);

Instrumentator::Instrumentator(Rules& rules) : m_rules(rules), m_nextDomainIndex(1) {}
Instrumentator::~Instrumentator(void) {}

struct CompareNoCase {
    bool operator () (const String& left, const String& right) const {
        return _wcsicmp(left.c_str(), right.c_str()) < 0;
    }
};

typedef std::map<String, ULONG32> FileUrlMap;
typedef std::pair<String, ULONG32> FileUrlMapPair;

FileUrlMap FileMap;

ULONG32 Instrumentator::GetFileUrlId(const String& url) {
    FileUrlMap::iterator it = FileMap.find(url);
    if (it != FileMap.end())
        return it->second;
    ULONG32 fileId = (ULONG32)(FileMap.size() + 1);
    FileMap.insert(FileUrlMapPair(url, fileId));
    return fileId;
}

bool Instrumentator::IsAssemblyAcceptable(const String& assemblyName) const
{
	return m_rules.IsAssemblyIncludedInRules(assemblyName);
}

void Instrumentator::AddSkippedAssembly(const String& assemblyName)
{
	SkippedTypedef skipped;
	skipped.assemblyName = assemblyName;
	m_skippedItems.push_back(skipped);
}

void Instrumentator::AddSkippedTypedef(const String& assemblyName, const String& typedefName)
{
	SkippedTypedef skipped;
	skipped.assemblyName = assemblyName;
	skipped.typedefName = typedefName;
	m_skippedItems.push_back(skipped);
}

void Instrumentator::InstrumentModule(ModuleID module, const String& moduleName, ICorProfilerInfo* profilerInfo, ISymUnmanagedBinder2* binder) {
    LockGuard g = Lock();

    ULONG bufferSize;
    HRESULT hr;

    ModuleDescriptor desc;
    desc.module = module;
    desc.loaded = true;

    if(FAILED(hr = profilerInfo->GetModuleInfo(module, NULL, 0, NULL, NULL, &desc.assembly))) {
		LOGINFO3(METHOD_INSTRUMENT, "Cannot instrument module %X '%s' (error %X on get module info)", module, moduleName.c_str(), hr);
        return;
    }

	desc.moduleName = CorHelper::GetModuleName(profilerInfo, desc.module);
	desc.assemblyName = CorHelper::GetAssemblyName(profilerInfo, desc.assembly);
    if (desc.assemblyName.length() == 0) {
		LOGINFO3(METHOD_INSTRUMENT, "Cannot instrument module %X '%s' (no assembly name)", module, moduleName.c_str(), hr);
        return;
    }

	AppDomainID appDomain;
	if(SUCCEEDED(hr = profilerInfo->GetAssemblyInfo(desc.assembly, 0, NULL, NULL, &appDomain, NULL))) {
		desc.domain = GetAppDomainIndex(appDomain);
		desc.domainName = CorHelper::GetAppDomainName(profilerInfo, appDomain);
	}

    CComPtr<IMetaDataImport> mdImport;
	hr = profilerInfo->GetModuleMetaData(module, ofRead, IID_IMetaDataImport, (IUnknown**) &mdImport);
    if(S_OK != hr) 
	{
		LOGINFO3(METHOD_INSTRUMENT, "Cannot instrument module %X '%s' (error %X on get meta data import)", module, moduleName.c_str(), hr);
        return;
    }
	
    if(binder) {
        ULONG searchPolicy = 
			AllowRegistryAccess |
			AllowSymbolServerAccess |
			AllowOriginalPathAccess |
			AllowReferencePathAccess;
        binder->GetReaderForFile2(mdImport, moduleName.c_str(), NULL, searchPolicy, &desc.symReader);
    }

    InstrumentHelper helper;
    helper.profilerInfo = profilerInfo;
    helper.module       = &desc;
    helper.mdImport     = mdImport;

    HCORENUM hEnum = 0;
    mdTypeDef typeDef;
    while(SUCCEEDED(hr = mdImport->EnumTypeDefs(&hEnum, &typeDef, 1, &bufferSize)) && bufferSize > 0) {
		InstrumentTypedef(desc.module, typeDef, helper);
    }

    mdImport->CloseEnum(hEnum);
    if (desc.typeDefs.size() == 0) {
		LOGINFO3(METHOD_INSTRUMENT, "In module %X '%s' instrumented %d items. Skip it", module, moduleName.c_str(), desc.typeDefs.size());
        return;
    }

    m_descriptors.push_back(desc);

	LOGINFO3(METHOD_INSTRUMENT, "Module %X '%s' instrumented with %d items.", module, moduleName.c_str(), desc.typeDefs.size());
}

void Instrumentator::UnloadModule(ModuleID module) {
	LockGuard l = Lock();

    for(ModuleDescriptors::iterator it = m_descriptors.begin(); it != m_descriptors.end(); ++it) {
		if (module == it->module) {
			it->loaded = false;
		}
    }
}

ModuleDescriptor* Instrumentator::GetModuleDescriptor(ModuleID module) {
    for(ModuleDescriptors::iterator it = m_descriptors.begin(); it != m_descriptors.end(); ++it) {
        if (module == it->module/* && it->loaded */)
            return &*it;
    }
    return 0;
}

void Instrumentator::InstrumentTypedef(ModuleID module, mdTypeDef typeDef, InstrumentHelper& helper) 
{
    DriverLog& log = DriverLog::get();
    HRESULT hr;

    DWORD typeDefFlags;
    String typedefName = CorHelper::GetTypedefFullName(helper.mdImport, typeDef, &typeDefFlags);
    if (typedefName.length() == 0) {
        LOGERROR1("Instrumentator", "InstrumentTypedef", "GetTypedefFullName failed. Typedef %d skipped", typeDef);
        return;
    }

    TypedefDescriptorMap& typedefMap = helper.module->typeDefs;
#ifdef DUMP_TYPEDEFS
    DumpTypeDef(log, typeDefFlags, typedefName.c_str());
#endif

    if (!IsTdClass(typeDefFlags)) {
        LOGINFO1(SKIP_BY_STATE, "  Instrumentation skipped for typedef '%s' (not a class)", typedefName.c_str());
        return;
    }

    if( typedefMap.find( typeDef ) != typedefMap.end() ) {
        LOGINFO1(SKIP_BY_RULES, "  Instrumentation skipped for class '%s' (already instrumented)", typedefName.c_str());
        return;
    }

	const String& assName = CorHelper::GetAssemblyName(helper.profilerInfo, helper.module->assembly);
    if (!m_rules.IsItemValidForReport(assName, typedefName, typeDef, helper.mdImport)) {
		AddSkippedTypedef(assName, typedefName);

        LOGINFO2(SKIP_BY_RULES, "  Instrumentation skipped for class [%s]%s (by rules)", assName.c_str(), typedefName.c_str());
        return;
    }

    TypeDef typeDefDescriptor;
    typeDefDescriptor.typeDef = typeDef;
	typeDefDescriptor.fullName = CorHelper::GetTypedefFullName(helper.mdImport, typeDef, NULL);

    HCORENUM enumHandle = 0;
    ULONG count;
    mdMethodDef method;
    while(SUCCEEDED(hr = helper.mdImport->EnumMethods(&enumHandle, typeDef, &method, 1, &count)) && count > 0) {
		InstrumentMethod(module, typeDefDescriptor, method, helper, typedefName);
    }
    helper.mdImport->CloseEnum(enumHandle);

    if (typeDefDescriptor.methodDefs.size() == 0) {
        LOGINFO1(SKIP_BY_STATE, "  Instrumentation skipped for class '%s' (no methods insturmented)", typedefName.c_str());
        return;
    }    

    typedefMap[typeDef].swap(typeDefDescriptor);
}

void Instrumentator::InstrumentMethod(ModuleID module, TypeDef& typeDef, mdMethodDef methodDef, InstrumentHelper& helper, const String& typedefName) 
{
    DriverLog& log = DriverLog::get();

    const String& methodName = CorHelper::GetMethodName(helper.mdImport, methodDef, NULL, NULL);
	if (methodName.length() == 0) {
		LOGERROR1("Instrumentator", "InstrumentMethod", "GetMethodName failed. MethodDef %d skipped", methodDef);
        return;
	}

#ifdef DUMP_METHODDEFS
    DumpMethodDef(log, attrs, methodName.c_str(), implFlags);
#endif

    MethodDefMap& methods = typeDef.methodDefs;
    if (methods.find(methodDef) != methods.end()) {
        log.WriteLine(_T("  Instrumentation skipped for method '%s.%s' (already introduced)"), typedefName.c_str(), methodName.c_str());
        return;
    }

	ULONG methodSize = 0;
	helper.profilerInfo->GetILFunctionBody(module, methodDef, NULL, &methodSize);

    LOGINFO1(METHOD_INNER, "Store new body (have %d items already)", (int)methods.size());
    MethodDef method;
    method.methodDef = methodDef;
	method.bodyUpdated = false;
	method.methodName = CorHelper::GetMethodName(helper.mdImport, method.methodDef, NULL, NULL);
	method.bodySize = methodSize;
	CorHelper::ParseMethodSig(helper.profilerInfo, helper.mdImport, method.methodDef, &method.methodSig);

    methods.insert(MethodDefMapPair(method.methodDef, method));

    LOGINFO4(METHOD_INSTRUMENT, "      Asm %X: Method %s.%s (0x%X) was introduced", helper.module->assembly, typedefName.c_str(), methodName.c_str(), method.methodDef);
}



void Instrumentator::UpdateFunctionCode(FunctionID funcId, ICorProfilerInfo2* info, ISymUnmanagedBinder2* binder) 
{
    ClassID funcClass;
	ModuleID funcModule;
	mdToken funcToken;
	ULONG32 numTypes=0;
	
	if(FAILED(info->GetFunctionInfo2(funcId, NULL, &funcClass, &funcModule, &funcToken, 0, NULL, NULL))) {
			return;
	}

	String funcPath = CorHelper::GetMethodPath(info, funcId);

	ModuleDescriptor* module = GetModuleDescriptor(funcModule);
	if (module == 0) 
	{
		// dont log an error - it's happens very often
		//LOGERROR1("Instrumentator", "UpdateFunctionCode", "GetModuleDescriptor failed for %s", funcPath.c_str());
		return;
	}

	mdTypeDef typeDef;
	ModuleID moduleID;
	ClassID parentClassID;
	if(FAILED(info->GetClassIDInfo2(funcClass, &moduleID, &typeDef, &parentClassID, 0, NULL, NULL))) 
		{
			LOGERROR1("Instrumentator", "UpdateFunctionCode", "GetClassIDInfo2 failed for %s", funcPath.c_str());
			return;
		}
	
    TypedefDescriptorMap::iterator typeDefinitionIt = module->typeDefs.find(typeDef);
    if (typeDefinitionIt == module->typeDefs.end()) {
		// dont log an error - it's happens very often
		//LOGERROR1("Instrumentator", "UpdateFunctionCode", "No typedef found for %s", funcPath.c_str());
        return;
    }

    TypeDef& defDescriptor = typeDefinitionIt->second;

    MethodDefMap::iterator methodIt = defDescriptor.methodDefs.find(funcToken);
	if (typeDefinitionIt == module->typeDefs.end()) 
	{
		LOGERROR1("Instrumentator", "UpdateFunctionCode", "No methoddef found for %s", funcPath.c_str());
		return;
	}

	ReplaceCode(*module, defDescriptor, methodIt->second, info);
}

void Instrumentator::UpdateClassCode(ClassID classId, ICorProfilerInfo* profilerInfo, ISymUnmanagedBinder2* binder) {
    ModuleID moduleId;
    mdTypeDef typeDef;
    if (FAILED(profilerInfo->GetClassIDInfo(classId, &moduleId, &typeDef))) {
        LOGINFO(METHOD_INSTRUMENT, "Cannot update code for class (no typedef)");
        return;
    }

    AssemblyID assembly;
    if (FAILED(profilerInfo->GetModuleInfo(moduleId, NULL, 0, NULL, NULL, &assembly))) {
        LOGINFO(METHOD_INSTRUMENT, "Cannot update code for class (no assembly)");
        return;
    }

    LockGuard l = Lock();

    ModuleDescriptor* module = GetModuleDescriptor(moduleId);
    if (module == 0) {
        LOGINFO(METHOD_INSTRUMENT, "Cannot update code for class (no module)");
        return;
    }

	CComPtr<IMetaDataImport> mdImport;
	if (S_OK != profilerInfo->GetModuleMetaData(module->module, ofRead, IID_IMetaDataImport, (IUnknown**) &mdImport))
	{
        LOGINFO(METHOD_INSTRUMENT, "Cannot update code for class (no module metadata)");
		return;
	}

    TypedefDescriptorMap::iterator typeDefinitionIt = module->typeDefs.find(typeDef);
    if (typeDefinitionIt == module->typeDefs.end()) {
        LOGINFO(METHOD_INSTRUMENT, "Cannot update code for class (no data)");
        return;
    }

    TypeDef& defDescriptor = typeDefinitionIt->second;

    MethodDefMap::iterator methodIt = defDescriptor.methodDefs.begin();
    while(methodIt != defDescriptor.methodDefs.end()) {
        MethodDef& method = (methodIt++)->second;
		ReplaceCode(*module, defDescriptor, method, profilerInfo);
    }
}

void Instrumentator::ReplaceCode(ModuleDescriptor& module, TypeDef& defDescriptor, MethodDef& method, ICorProfilerInfo* profilerInfo)
{
	HRESULT hr;
	String& methodName = defDescriptor.fullName + _T("::") + method.methodName;

    if (!method.bodyUpdated) {
		GenerateILCode(module, defDescriptor, method, profilerInfo);
    }

	if (method.bodyBytes.size() == 0) 
	{
        LOGERROR2("Instrumentator", "ReplaceCode", "GenerateILCode failed for method '%s' in module %X", methodName.c_str(), module.module);
		return;
	}

    CComPtr<IMethodMalloc> methodAllocator;
    LOGINFO(METHOD_INNER, "Get IL Function Body Allocator");
	if(FAILED(hr = profilerInfo->GetILFunctionBodyAllocator(module.module, &methodAllocator)))  
	{
        LOGERROR2("Instrumentator", "ReplaceCode", "GetILFunctionBodyAllocator failed for method '%s' in module %X", methodName.c_str(), module.module);
        return;
    }

    LOGINFO(METHOD_INNER, "Allocate method body");
	void* newBody = methodAllocator->Alloc(static_cast<ULONG>(method.bodyBytes.size()) );

    LOGINFO(METHOD_INNER, "Copy method body");
	memcpy(newBody, method.bodyBytes, method.bodyBytes.size() );

    LOGINFO(METHOD_INNER, "Set new IL Function Body");
    if (SUCCEEDED(hr = profilerInfo->SetILFunctionBody(module.module, method.methodDef, (LPCBYTE) newBody))) 
	{
        LOGINFO3(METHOD_INSTRUMENT, "Asm %X: Method %s (0x%X) il body updated", module.assembly, methodName.c_str(), method.methodDef);
	} 
	else 
	{
        LOGERROR3("Instrumentator", "ReplaceCode", "SetILFunctionBody failed for method '%s' in module %X with error %X", methodName.c_str(), module.module, hr);
    }
}

void Instrumentator::GenerateILCode(ModuleDescriptor& module, TypeDef& defDescriptor, MethodDef& method, ICorProfilerInfo* profilerInfo)
{
	HRESULT hr;
	String& methodName = method.methodName;

    LPCBYTE methodHeader = 0;
	ULONG methodSize = 0;
	if(FAILED(hr = profilerInfo->GetILFunctionBody(module.module, method.methodDef, &methodHeader, &methodSize))) {
		if(hr == CORPROF_E_FUNCTION_NOT_IL) {
			LOGINFO1(METHOD_INNER, "GetILFunctionBody failed for method '%s' (not il-method)", methodName.c_str());
		} else {
			LOGINFO2(METHOD_INNER, "GetILFunctionBody failed for method '%s' with code 0x%X", methodName.c_str(), hr);
		}
        return;
	}


	LOGINFO1(METHOD_INNER, "Starting instrumentation for methodDef %s", methodName.c_str());
    InstrumentedILBody ilbody(methodHeader, methodSize, m_allocator);

    if(!ilbody.IsBodyParsed()) {
        LOGERROR1("Instrumentator", "GenerateILCode", "Cannot parse body for methodDef %s. Skip methodDef", methodName.c_str());
        return;
    }

    if (module.symReader) 
	{
        CComPtr<ISymUnmanagedMethod> symMethod;
		if(SUCCEEDED(hr = module.symReader->GetMethod( method.methodDef, &symMethod ))) 
		{
            ULONG32 points;
            if(SUCCEEDED(hr = symMethod->GetSequencePointCount( &points ))) 
			{
                ULONG32 pointsInRes;
                DynamicArray<ULONG32> cPoints(points);
                DynamicArray<ISymUnmanagedDocument*> pDocuments(points);
                DynamicArray<ULONG32> lines(points);
                DynamicArray<ULONG32> columns(points);
                DynamicArray<ULONG32> endLines(points);
                DynamicArray<ULONG32> endColumns(points);

                if(SUCCEEDED(hr = symMethod->GetSequencePoints( points, &pointsInRes, cPoints, pDocuments, lines, columns, endLines, endColumns ))) 
				{
#ifdef DUMP_SYM_SEQUENCE_POINTS
                    DumpSymSequencePoints(pointsInRes, cPoints, pDocuments, lines, columns, endLines, endColumns);
#endif
                    //pointsInRes = pointsInRes <= 1 ? pointsInRes : pointsInRes - 1;

                    LOGINFO(METHOD_INNER, "Insert block counters from source code");
                    // here we may remove last seq point, because usually it points to exit point of method. do we really need it?
                    ilbody.CreateSequenceCounters(pointsInRes, cPoints);

                    InstrumentedBlocks& blocks = ilbody.GetInstrumentedBlocks();
                    for(ULONG32 i = 0; i < pointsInRes; ++i) 
					{
                        InstrumentedBlock& block = blocks[i];

                        ULONG32 urlSize;
                        if (FAILED(hr = pDocuments[i]->GetURL(0, &urlSize, NULL))) 
						{
                            continue;
                        }

                        DynamicArray<TCHAR> url(urlSize);
                        if (FAILED(hr = pDocuments[i]->GetURL(urlSize + 1, &urlSize, url))) 
						{
                            continue;
                        }

                        block.fileId = (ULONG32) GetFileUrlId(String(url));
                        block.startLine = lines[i];
                        block.startColumn = columns[i];
                        block.endLine = endLines[i];
                        block.endColumn = endColumns[i];
                    }
                } 
				else 
				{
                    LOGINFO2(METHOD_INNER, "Cannot exec GetSequencePoints for methodDef %s. Error %X", methodName.c_str(), hr);
                }
            } 
			else 
			{
                LOGINFO2(METHOD_INNER, "Cannot exec GetSequencePointCount for methodDef %s. Error %X", methodName.c_str(), hr);
            }
        } 
		else 
		{
            LOGINFO2(METHOD_INNER, "Cannot get symMethod for methodDef %s. Error %X", methodName.c_str(), hr);
        }
    } 
    
    if( ilbody.GetInstrumentedBlocks().size() == 0 ) 
	{
        LOGINFO(METHOD_INNER, "Insert block counters from il-body");
        ilbody.CreateSequenceCountersFromCode();
    }

    LOGINFO(METHOD_INNER, "Construct new body");
    ilbody.ConstructNewBody();

    ilbody.DumpNewBody();

	// save new body created
	method.bodyUpdated = true;
	method.bodyBlocks = ilbody.GetInstrumentedBlocks();
	method.bodyBytes.allocate(ilbody.GetInstrumentedBodySize());
	memcpy_s(method.bodyBytes, method.bodyBytes.size(), ilbody.GetInstrumentedBody(), method.bodyBytes.size());
}

void Instrumentator::ActivateAppDomain(AppDomainID domain)
{
	LockGuard l = Lock();
	m_domains[domain] = m_nextDomainIndex++;
}

void Instrumentator::DeactivateAppDomain(AppDomainID domain)
{
	LockGuard l = Lock();
	m_domains.erase(domain);
}

int Instrumentator::GetAppDomainIndex(AppDomainID domain)
{
	LockGuard l = Lock();

	AppDomainIndexMap::iterator it = m_domains.find(domain);
	if (it == m_domains.end()) 
		return -1;
	return it->second;
}

struct MethodBlockResultGatherer 
{
    InstrumentResults::MethodBlocks& results;
    MethodBlockResultGatherer(InstrumentResults::MethodBlocks& _results) : results(_results) {}

    void operator() (const InstrumentedBlock& block) 
	{
        InstrumentResults::MethodBlock blockResult;

        blockResult.visitCount = *block.counter;
        blockResult.position = block.position;
        blockResult.blockLength = block.length;

#ifdef DUMP_INSTRUMENT_RESULT
        LOGINFO4(DUMP_RESULTS,"         [%5d] (%X) offset %4X, length %d", 
            blockResult.visitCount, block.counter, blockResult.position, blockResult.blockLength);
#endif

        if (block.fileId != 0 && block.startLine != 0xFEEFEE) {
            blockResult.haveSource = true;
            blockResult.sourceFileId = block.fileId;
            blockResult.startLine = block.startLine;
            blockResult.endLine = block.endLine;
            blockResult.startColumn = block.startColumn;
            blockResult.endColumn = block.endColumn;
#ifdef DUMP_INSTRUMENT_RESULT
            LOGINFO5(DUMP_RESULTS,"                 %d [ %d; %d ] - [ %d; %d ]", 
                blockResult.sourceFileId, blockResult.startLine, blockResult.startColumn, blockResult.endLine, blockResult.endColumn);
#endif
        } else {
            blockResult.haveSource = false;
        }

        results.push_back(blockResult);
    }
};

struct MethodResultsGatherer 
{
    InstrumentResults::MethodResults& results;
	StoreHelper& helper;

    MethodResultsGatherer(InstrumentResults::MethodResults& _results, StoreHelper& _helper) 
		: results(_results), helper(_helper)
	{
	}

    void operator() (const MethodDefMapPair& itPair) 
	{
        const MethodDef& method = itPair.second;

        InstrumentResults::MethodResult methodResult;

		//methodResult.name = CorHelper::GetMethodName(helper.mdImport, method.methodDef, &methodResult.flags, &methodResult.implFlags);
		//CorHelper::GetMethodSig(helper.profilerInfo, helper.mdImport, method.methodDef, &methodResult.sig);

		methodResult.name = method.methodName;
		methodResult.sig = method.methodSig;
		methodResult.bodySize = method.bodySize;

		if (method.bodyBlocks.size() == 0) {
#ifdef DUMP_INSTRUMENT_RESULT
            LOGINFO2(DUMP_RESULTS,"      Method %s : %s, no instrumented body", method.methodDefName.c_str(), method.methodSig.c_str());
#endif
        } else {
			const InstrumentedBlocks& blocks = method.bodyBlocks;
#ifdef DUMP_INSTRUMENT_RESULT
            LOGINFO3(DUMP_RESULTS,"      Method %s : %s, %d instrumented blocks", method.methodDefName.c_str(), method.methodSig.c_str(), blocks.size());
#endif
            std::for_each(blocks.begin(), blocks.end(), 
				MethodBlockResultGatherer(methodResult.blocks));
        }

        results.push_back(methodResult);
    }
};

struct TypedefResultsGatherer {
    InstrumentResults::TypedefResults& results;
	StoreHelper& helper;

    TypedefResultsGatherer(InstrumentResults::TypedefResults& _results, StoreHelper& _helper) 
		: results(_results), helper(_helper) {}

    void operator() (const TypedefDescriptorMapPair& it) {
        const TypeDef& type = it.second;
#ifdef DUMP_INSTRUMENT_RESULT
        LOGINFO1(DUMP_RESULTS,"    Type %s", type.typeDefName.c_str());
#endif

        InstrumentResults::TypedefResult typedefResult;
		typedefResult.fullName = type.fullName;

        std::for_each(type.methodDefs.begin(), type.methodDefs.end(), 
			MethodResultsGatherer(typedefResult.methods, helper));

        results.push_back(typedefResult);
    }
};

struct AssemblyResultsGatherer {
    InstrumentResults::AssemblyResults& results;
	ICorProfilerInfo* info;

    AssemblyResultsGatherer(ICorProfilerInfo* _info, InstrumentResults::AssemblyResults& _results) 
		: results(_results), info(_info) {}

    void operator() (ModuleDescriptor& module) {
		//const String& assemblyName = CorHelper::GetAssemblyName(info, module.assembly);
		//const String& moduleName = CorHelper::GetModuleName(info, module.module);

#ifdef DUMP_INSTRUMENT_RESULT
		LOGINFO1(DUMP_RESULTS,"  Assembly %S (module %S)", assemblyName.c_str(), moduleName.c_str());
#endif

        InstrumentResults::AssemblyResult assemblyResult;
        assemblyResult.name = module.assemblyName;
        assemblyResult.module = module.moduleName;
		assemblyResult.domain = module.domainName;
		assemblyResult.domainIndex = module.domain;

		StoreHelper helper;
		helper.module = &module;
		helper.profilerInfo = info;

        std::for_each(
			module.typeDefs.begin(), 
			module.typeDefs.end(), 
			TypedefResultsGatherer(assemblyResult.types, helper));

        results.push_back(assemblyResult);
    }
};

struct FileMapGatherer {
    InstrumentResults::FileItems& results;
    FileMapGatherer(InstrumentResults::FileItems& _results) : results(_results) {}

    void operator() (const FileUrlMapPair& file) {
#ifdef DUMP_INSTRUMENT_RESULT
        LOGINFO2(DUMP_RESULTS,"  File %d, Url %s", file.second, file.first.c_str());
#endif
        InstrumentResults::FileItem item;
        item.fileId = file.second;
        item.fileUrl = file.first;
        results.push_back(item);
    }
};

struct SkippedResultsGatherer
{
    InstrumentResults::SkippedItems& results;
	SkippedResultsGatherer(InstrumentResults::SkippedItems& _results) : results(_results) {}

	void operator() (const SkippedTypedef& skipped) {
#ifdef DUMP_INSTRUMENT_RESULT
        LOGINFO2(DUMP_RESULTS,"  Assembly %d, Typedef %s", skipped.assemblyItem.c_str(), skipped.typedefName.c_str());
#endif
		InstrumentResults::SkippedItem item;
		item.assemblyName = skipped.assemblyName;
		item.typedefName = skipped.typedefName;
        results.push_back(item);
    }
};

void Instrumentator::StoreResults(InstrumentResults& results, ICorProfilerInfo* info) 
{
    InstrumentResults::AssemblyResults instrumentResults;
    InstrumentResults::FileItems fileTable;
	InstrumentResults::SkippedItems skippedResults;

#ifdef DUMP_INSTRUMENT_RESULT
    LOGINFO(DUMP_RESULTS, "");
    LOGINFO(DUMP_RESULTS, "Dump results for intrumentator");
#endif

    std::for_each(
		m_descriptors.begin(), 
		m_descriptors.end(), 
		AssemblyResultsGatherer(info, instrumentResults));

    std::for_each(
		FileMap.begin(), 
		FileMap.end(), 
		FileMapGatherer(fileTable));

    std::for_each(
		m_skippedItems.begin(),
		m_skippedItems.end(),
		SkippedResultsGatherer(skippedResults));

    results.Assign( instrumentResults );
    results.Assign( fileTable );
	results.Assign( skippedResults );
}


void DumpTypeDef(DriverLog& log, DWORD typeDefFlags, LPCTSTR typedefName) {
    String flagString;
    if (IsTdClass(typeDefFlags)) flagString += _T(", class");
    if (IsTdInterface(typeDefFlags)) flagString += _T(", interface");
    if (IsTdNotPublic(typeDefFlags)) flagString += _T(", not public");
    if (IsTdPublic(typeDefFlags)) flagString += _T(", public");
    if (IsTdNestedPublic(typeDefFlags)) flagString += _T(", nested public");
    if (IsTdNestedPrivate(typeDefFlags)) flagString += _T(", nested private");
    if (IsTdNestedFamily(typeDefFlags)) flagString += _T(", nested family");
    if (IsTdNestedAssembly(typeDefFlags)) flagString += _T(", nested assembly");
    if (IsTdNestedFamANDAssem(typeDefFlags)) flagString += _T(", nested family and assembly");
    if (IsTdNestedFamORAssem(typeDefFlags)) flagString += _T(", nested family or assembly");
    if (IsTdAbstract(typeDefFlags)) flagString += _T(", abstract");
    if (IsTdSealed(typeDefFlags)) flagString += _T(", sealed");
    if (IsTdSpecialName(typeDefFlags)) flagString += _T(", special name");
    if (IsTdImport(typeDefFlags)) flagString += _T(", import");
    if (IsTdAnsiClass(typeDefFlags)) flagString += _T(", ansi class");
    if (IsTdUnicodeClass(typeDefFlags)) flagString += _T(", unicode class");
    if (IsTdAutoClass(typeDefFlags)) flagString += _T(", auto class");
    if (IsTdRTSpecialName(typeDefFlags)) flagString += _T(", rt special name");
    if (IsTdHasSecurity(typeDefFlags)) flagString += _T(", has security");
    log.WriteLine(_T("  Typedef '%s'%s"), typedefName, flagString.c_str());
}

void DumpMethodDef(DriverLog& log, DWORD flags, LPCTSTR methoddefName, DWORD implFlag) {
    String flagString;
    if (IsMdPrivateScope(flags)) flagString += _T(", private scope");
    if (IsMdPrivate(flags)) flagString += _T(", private");
    if (IsMdFamANDAssem(flags)) flagString += _T(", family and assembly");
    if (IsMdAssem(flags)) flagString += _T(", assembly");
    if (IsMdFamily(flags)) flagString += _T(", family");
    if (IsMdFamORAssem(flags)) flagString += _T(", family or assembly");
    if (IsMdPublic(flags)) flagString += _T(", public");
    if (IsMdStatic(flags)) flagString += _T(", static");
    if (IsMdFinal(flags)) flagString += _T(", sealed");
    if (IsMdVirtual(flags)) flagString += _T(", virtual");
    if (IsMdHideBySig(flags)) flagString += _T(", hide by sig");
    if (IsMdAbstract(flags)) flagString += _T(", abstract");
    if (IsMdSpecialName(flags)) flagString += _T(", special name");
    if (IsMdPinvokeImpl(flags)) flagString += _T(", pinvoke");
    if (IsMdUnmanagedExport(flags)) flagString += _T(", unmanaged export");
    if (IsMdRTSpecialName(flags)) flagString += _T(", rt special name");
    if (IsMdHasSecurity(flags)) flagString += _T(", has security");
    if (IsMdRequireSecObject(flags)) flagString += _T(", require secondary object");
    if (IsMdClassConstructorW(flags, methoddefName)) flagString += _T(", static constructor");
    if (IsMdInstanceInitializerW(flags, methoddefName)) flagString += _T(", constructor");

    if (IsMiIL(implFlag)) flagString += _T(", il");
    if (IsMiNative(implFlag)) flagString += _T(", native");
    if (IsMiOPTIL(implFlag)) flagString += _T(", optil");
    if (IsMiUnmanaged(implFlag)) flagString += _T(", unmanaged");
    if (IsMiManaged(implFlag)) flagString += _T(", managed");
    if (IsMiForwardRef(implFlag)) flagString += _T(", forward");
    if (IsMiPreserveSig(implFlag)) flagString += _T(", preserve sig");
    if (IsMiInternalCall(implFlag)) flagString += _T(", internal call");
    if (IsMiSynchronized(implFlag)) flagString += _T(", synchronized");
    if (IsMiNoInlining(implFlag)) flagString += _T(", no inline");

    log.WriteLine(_T("    Methoddef '%s'%s"), methoddefName, flagString.c_str());
}

void DumpSymSequencePoints(ULONG32 points, ULONG32 offsets[], ISymUnmanagedDocument *documents[], ULONG32 lines[], ULONG32 columns[], ULONG32 endLines[], ULONG32 endColumns[]) {
    LOGINFO(METHOD_INNER, "Method Sequence Points");
    LOGINFO(METHOD_INNER, "Offset        Line        Col         EndLine     EndCol    ");
    LOGINFO(METHOD_INNER, "============  ==========  ==========  ==========  ==========" );
    for (ULONG i=0; i < points; i++) {
        LOGINFO6(METHOD_INNER, "%4d(0x%04X)  %10d  %10d  %10d  %10d", offsets[i], offsets[i], lines[i], columns[i], endLines[i], endColumns[i]);
    }
}
