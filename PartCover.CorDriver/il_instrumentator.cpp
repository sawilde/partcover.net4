#include "StdAfx.h"
#include "interface.h"
#include "message.h"
#include "message_pipe.h"
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

Instrumentator::Instrumentator(Rules& rules) : m_rules(rules) {}
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

void Instrumentator::InstrumentModule(ModuleID module, const String& moduleName, ICorProfilerInfo* profilerInfo, ISymUnmanagedBinder2* binder) {
    Lock();
    ULONG bufferSize;
    HRESULT hr;

    ModuleDescriptor desc;
    desc.module = module;
    desc.moduleName = moduleName;
    desc.loaded = true;

    if(FAILED(hr = profilerInfo->GetModuleInfo(module, NULL, 0, NULL, NULL, &desc.assembly))) {
        LOGINFO3(METHOD_INSTRUMENT, "Cannot instrument module %X '%s' (error %X on get module info)", module, moduleName, hr);
        Unlock();
        return;
    }

    desc.assemblyName = CorHelper::GetAssemblyName(profilerInfo, desc.assembly);
    if (desc.assemblyName.length() == 0) {
        LOGINFO3(METHOD_INSTRUMENT, "Cannot instrument module %X '%s' (no assembly name)", module, moduleName, hr);
        Unlock();
        return;
    }

    CComPtr<IMetaDataImport> mdImport;
	hr = profilerInfo->GetModuleMetaData(module, ofRead, IID_IMetaDataImport, (IUnknown**) &mdImport);
    if(S_OK != hr) 
	{
        LOGINFO3(METHOD_INSTRUMENT, "Cannot instrument module %X '%s' (error %X on get meta data import)", module, moduleName, hr);
        Unlock();
        return;
    }
	
    if(binder) {
        ULONG searchPolicy = AllowRegistryAccess|AllowSymbolServerAccess|AllowOriginalPathAccess|AllowReferencePathAccess;
        binder->GetReaderForFile2(mdImport, desc.moduleName.c_str(), NULL, searchPolicy, &desc.symReader);
    }

    InstrumentHelper helper;
    helper.profilerInfo = profilerInfo;
    helper.module = &desc;
    helper.mdImport = mdImport;

    HCORENUM hEnum = 0;
    mdTypeDef typeDef;
    while(SUCCEEDED(hr = mdImport->EnumTypeDefs(&hEnum, &typeDef, 1, &bufferSize)) && bufferSize > 0) {
        InstrumentTypedef(typeDef, helper);
    }

    mdImport->CloseEnum(hEnum);
    if (desc.typeDefs.size() == 0) {
        LOGINFO3(METHOD_INSTRUMENT, "In module %X '%s' instrumented %d items. Skip it", module, moduleName, desc.typeDefs.size());
        Unlock();
        return;
    }

    m_descriptors.push_back(desc);
    Unlock();
}

void Instrumentator::UnloadModule(ModuleID module) {
    Lock();
    for(ModuleDescriptors::iterator it = m_descriptors.begin(); it != m_descriptors.end(); ++it) {
        if (module == it->module)
            it->loaded = false;
    }
    Unlock();
}

ModuleDescriptor* Instrumentator::GetModuleDescriptor(ModuleID module) {
    for(ModuleDescriptors::iterator it = m_descriptors.begin(); it != m_descriptors.end(); ++it) {
        if (module == it->module && it->loaded)
            return &*it;
    }
    return 0;
}

void Instrumentator::InstrumentTypedef(mdTypeDef typeDef, InstrumentHelper& helper) 
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

    const String& assName = helper.module->assemblyName;
    if (!m_rules.IsItemValidForReport(assName, typedefName, typeDef, helper.mdImport)) {
        LOGINFO2(SKIP_BY_RULES, "  Instrumentation skipped for class [%s]%s (by rules)", assName.c_str(), typedefName.c_str());
        return;
    }

    String typeDefNamespace = RulesHelpers::ExtractNamespace(typedefName);
    if (typeDefNamespace.length() == 0) {
        LOGINFO1(SKIP_BY_STATE, "  Instrumentation skipped for class '%s' (have no namespace)", typedefName.c_str());
        return;
    }

    TypeDef typeDefDescriptor;
    typeDefDescriptor.typeDefName.swap(typedefName);
    typeDefDescriptor.typeDefNamespace.swap(typeDefNamespace);
    typeDefDescriptor.typeDef = typeDef;
    typeDefDescriptor.flags = typeDefFlags;

    HCORENUM enumHandle = 0;
    ULONG count;
    mdMethodDef method;
    while(SUCCEEDED(hr = helper.mdImport->EnumMethods(&enumHandle, typeDef, &method, 1, &count)) && count > 0) {
        InstrumentMethod(typeDefDescriptor, method, helper);
    }
    helper.mdImport->CloseEnum(enumHandle);

    if (typeDefDescriptor.methodDefs.size() == 0) {
        LOGINFO1(SKIP_BY_STATE, "  Instrumentation skipped for class '%s' (no methods insturmented)", typeDefDescriptor.typeDefName.c_str());
        return;
    }    

    typedefMap[typeDef].swap(typeDefDescriptor);
}

void Instrumentator::InstrumentMethod(TypeDef& typeDef, mdMethodDef methodDef, InstrumentHelper& helper) {
    DriverLog& log = DriverLog::get();
    HRESULT hr;
    DWORD attrs, implFlags;
    ULONG bufferSize;
    if(FAILED(hr = helper.mdImport->GetMethodProps(methodDef, NULL, NULL, 0, &bufferSize, &attrs, NULL, NULL, NULL, &implFlags))) {
        LOGERROR2("Instrumentator", "InstrumentMethod", "GetMethodProps failed for methodDef %d with code 0x%X", methodDef, hr);
        return;
    }

	DynamicArray<TCHAR> buffer(bufferSize + 1);
    helper.mdImport->GetMethodProps(methodDef, NULL, buffer, bufferSize + 1, &bufferSize, NULL, NULL, NULL, NULL, NULL);
    String methodName = buffer;

#ifdef DUMP_METHODDEFS
    DumpMethodDef(log, attrs, methodName.c_str(), implFlags);
#endif

    MethodDefMap& methods = typeDef.methodDefs;
    if (methods.find(methodDef) != methods.end()) {
        log.WriteLine(_T("  Instrumentation skipped for method '%s.%s' (already instrumented)"), typeDef.typeDefName.c_str(), methodName.c_str());
        return;
    }

    LPCBYTE methodHeader = 0;
    ULONG methodSize = 0;
    if(FAILED(hr = helper.profilerInfo->GetILFunctionBody(helper.module->module, methodDef, &methodHeader, &methodSize))) {
        if(hr == CORPROF_E_FUNCTION_NOT_IL) {
            LOGINFO1(METHOD_INNER, "GetILFunctionBody failed for method '%s' (not il-method)", methodName.c_str());
        } else {
            LOGINFO2(METHOD_INNER, "GetILFunctionBody failed for method '%s' with code 0x%X", methodName.c_str(), hr);
        }
        return;
    }

    try {
        LOGINFO2(METHOD_INNER, "Starting instrumentation for methodDef %s.%s", typeDef.typeDefName.c_str(), methodName.c_str());
        std::auto_ptr<InstrumentedILBody> body(new InstrumentedILBody(methodHeader, methodSize));

        if(!body.get()) {
            LOGERROR2("Instrumentator", "InstrumentMethod", "Cannot create il body for %s.%s", typeDef.typeDefName.c_str(), methodName.c_str());
            return;
        }

        if(!body->IsBodyParsed()) {
            LOGERROR2("Instrumentator", "InstrumentMethod", "Cannot parse body for methodDef %s.%s. Skip methodDef", typeDef.typeDefName.c_str(), methodName.c_str());
            return;
        }

        if (helper.module->symReader) {
            CComPtr<ISymUnmanagedMethod> symMethod;
            if(SUCCEEDED(hr = helper.module->symReader->GetMethod( methodDef, &symMethod ))) {
                ULONG32 points;
                if(SUCCEEDED(hr = symMethod->GetSequencePointCount( &points ))) {
                    ULONG32 pointsInRes;
                    DynamicArray<ULONG32> cPoints(points);
                    DynamicArray<ISymUnmanagedDocument*> pDocuments(points);
                    DynamicArray<ULONG32> lines(points);
                    DynamicArray<ULONG32> columns(points);
                    DynamicArray<ULONG32> endLines(points);
                    DynamicArray<ULONG32> endColumns(points);

                    if(SUCCEEDED(hr = symMethod->GetSequencePoints( points, &pointsInRes, cPoints, pDocuments, lines, columns, endLines, endColumns ))) {
#ifdef DUMP_SYM_SEQUENCE_POINTS
                        DumpSymSequencePoints(pointsInRes, cPoints, pDocuments, lines, columns, endLines, endColumns);
#endif
                        //pointsInRes = pointsInRes <= 1 ? pointsInRes : pointsInRes - 1;

                        LOGINFO(METHOD_INNER, "Insert block counters from source code");
                        // here we may remove last seq point, because usually it points to exit point of method. do we really need it?
                        body->CreateSequenceCounters(pointsInRes, cPoints);

                        InstrumentedBlocks& blocks = body->GetInstrumentedBlocks();
                        for(ULONG32 i = 0; i < pointsInRes; ++i) {
                            InstrumentedBlock& block = blocks[i];

                            ULONG32 urlSize;
                            if (FAILED(hr = pDocuments[i]->GetURL(0, &urlSize, NULL))) {
                                continue;
                            }

                            DynamicArray<WCHAR> url(urlSize);
                            if (FAILED(hr = pDocuments[i]->GetURL(urlSize + 1, &urlSize, url))) {
                                continue;
                            }

                            block.fileId = (ULONG32) GetFileUrlId(String(url));
                            block.startLine = lines[i];
                            block.startColumn = columns[i];
                            block.endLine = endLines[i];
                            block.endColumn = endColumns[i];
                        }
                    } else {
                        LOGINFO3(METHOD_INNER, "Cannot exec GetSequencePoints for methodDef %s.%s. Error %X", typeDef.typeDefName.c_str(), methodName.c_str(), hr);
                    }
                } else {
                    LOGINFO3(METHOD_INNER, "Cannot exec GetSequencePointCount for methodDef %s.%s. Error %X", typeDef.typeDefName.c_str(), methodName.c_str(), hr);
                }
            } else {
                LOGINFO3(METHOD_INNER, "Cannot get symMethod for methodDef %s.%s. Error %X", typeDef.typeDefName.c_str(), methodName.c_str(), hr);
            }
        } 
        
        if( body->GetInstrumentedBlocks().size() == 0 ) {
            LOGINFO(METHOD_INNER, "Insert block counters from il-body");
            body->CreateSequenceCountersFromCode();
        }

        LOGINFO1(METHOD_INNER, "Store new body (have %d items already)", (int)methods.size());
        MethodDef method;
        method.methodDefName.swap(methodName);
        method.methodDef = methodDef;
        CorHelper::GetMethodSig(helper.profilerInfo, helper.mdImport, methodDef, &method.methodSig);
        method.flags = attrs;
        method.implFlags = implFlags;
        method.instrumentedBody = body.release();
        methods.insert(MethodDefMapPair(method.methodDef, method));

        LOGINFO4(METHOD_INSTRUMENT, "      Asm %X: Method %s.%s (0x%X) was instrumented and stored", helper.module->assembly, typeDef.typeDefName.c_str(), method.methodDefName.c_str(), method.methodDef);
    } catch(std::exception& ex) {
        LOGERROR1("Instrumentator", "InstrumentMethod", "Instrumentation breaked. %s", ex.what());
    }
}

void Instrumentator::UpdateClassCode(ClassID classId, ICorProfilerInfo* profilerInfo, ISymUnmanagedBinder2* binder) {
    HRESULT hr;

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

    Lock();

    ModuleDescriptor* descriptor = GetModuleDescriptor(moduleId);
    if (descriptor == 0) {
        LOGINFO(METHOD_INSTRUMENT, "Cannot update code for class (no module)");
        Unlock();
        return;
    }

    TypedefDescriptorMap::iterator typeDefinitionIt = descriptor->typeDefs.find(typeDef);
    if (typeDefinitionIt == descriptor->typeDefs.end()) {
        LOGINFO(METHOD_INSTRUMENT, "Cannot update code for class (no data)");
        Unlock();
        return;
    }

    TypeDef& defDescriptor = typeDefinitionIt->second;
    MethodDefMap::iterator methodIt = defDescriptor.methodDefs.begin();
    while(methodIt != defDescriptor.methodDefs.end()) {
        MethodDef& method = (methodIt++)->second;
        if (method.bodyUpdated) {
            LOGINFO(METHOD_INSTRUMENT, "Cannot update code for class (il body already updated)");
            continue;
        }

        if (method.instrumentedBody == 0) {
            LOGINFO(METHOD_INSTRUMENT, "Cannot update code for class (no il body)");
            continue;
        }

        InstrumentedILBody& body = *method.instrumentedBody;

        LOGINFO(METHOD_INNER, "Construct new body");
        body.ConstructNewBody();

        body.DumpNewBody();

        CComPtr<IMethodMalloc> methodAllocator;
        LOGINFO(METHOD_INNER, "Get IL Function Body Allocator");
        if(FAILED(hr = profilerInfo->GetILFunctionBodyAllocator(moduleId, &methodAllocator)))  {
            LOGERROR2("Instrumentator", "InstrumentMethod", "GetILFunctionBodyAllocator failed for method '%s' in module %X", method.methodDefName.c_str(), moduleId);
            continue;
        }

        LOGINFO(METHOD_INNER, "Allocate method body");
        void* newBody = methodAllocator->Alloc( body.GetInstrumentedBodySize() );

        LOGINFO(METHOD_INNER, "Copy method body");
        memcpy(newBody, body.GetInstrumentedBody(), body.GetInstrumentedBodySize() );

        LOGINFO(METHOD_INNER, "Set new IL Function Body");
        if (SUCCEEDED(hr = profilerInfo->SetILFunctionBody(moduleId, method.methodDef, (LPCBYTE) newBody))) {
            LOGINFO4(METHOD_INSTRUMENT, "Asm %X: Method %s.%s (0x%X) il body updated", descriptor->assembly, defDescriptor.typeDefName.c_str(), method.methodDefName.c_str(), method.methodDef);
            method.bodyUpdated = true;
        } else {
            LOGERROR3("Instrumentator", "InstrumentMethod", "SetILFunctionBody failed for method '%s' in module %X with error %X", method.methodDefName.c_str(), moduleId, hr);
        }
    }

    Unlock();
}

struct MethodBlockResultGatherer {
    InstrumentResults::MethodBlocks& results;
    MethodBlockResultGatherer(InstrumentResults::MethodBlocks& _results) : results(_results) {}

    void operator() (InstrumentedBlock& block) {
        InstrumentResults::MethodBlock blockResult;

        blockResult.visitCount = *block.counter;
        blockResult.position = block.position;
        blockResult.blockLength = block.length;

#ifdef DUMP_INSTRUMENT_RESULT
        LOGINFO4(DUMP_RESULTS,"         [%5d] (%X) offset %4X, length %d", 
            blockResult.visitCount, block.counter, blockResult.position, blockResult.blockLength);
#endif

        delete block.counter;
        block.counter = 0;

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

struct MethodResultsGatherer {
    InstrumentResults::MethodResults& results;
    MethodResultsGatherer(InstrumentResults::MethodResults& _results) : results(_results) {}

    void operator() (const MethodDefMapPair& itPair) {
        const MethodDef& method = itPair.second;

        InstrumentResults::MethodResult methodResult;
        methodResult.name = method.methodDefName;
        methodResult.sig = method.methodSig;
        methodResult.flags = method.flags;
        methodResult.implFlags = method.implFlags;

        if (method.instrumentedBody == 0) {
#ifdef DUMP_INSTRUMENT_RESULT
            LOGINFO2(DUMP_RESULTS,"      Method %s : %s, no instrumented body", method.methodDefName.c_str(), method.methodSig.c_str());
#endif
        } else {
            InstrumentedBlocks& blocks = method.instrumentedBody->GetInstrumentedBlocks();
#ifdef DUMP_INSTRUMENT_RESULT
            LOGINFO3(DUMP_RESULTS,"      Method %s : %s, %d instrumented blocks", method.methodDefName.c_str(), method.methodSig.c_str(), blocks.size());
#endif
            std::for_each(blocks.begin(), blocks.end(), MethodBlockResultGatherer(methodResult.blocks));
        }

        results.push_back(methodResult);
    }
};

struct TypedefResultsGatherer {
    InstrumentResults::TypedefResults& results;
    TypedefResultsGatherer(InstrumentResults::TypedefResults& _results) : results(_results) {}

    void operator() (const TypedefDescriptorMapPair& it) {
        const TypeDef& type = it.second;
#ifdef DUMP_INSTRUMENT_RESULT
        LOGINFO1(DUMP_RESULTS,"    Type %s", type.typeDefName.c_str());
#endif

        InstrumentResults::TypedefResult typedefResult;
        typedefResult.fullName = type.typeDefName;
        typedefResult.flags = type.flags;

        std::for_each(type.methodDefs.begin(), type.methodDefs.end(), MethodResultsGatherer(typedefResult.methods));

        results.push_back(typedefResult);
    }
};

struct AssemblyResultsGatherer {
    InstrumentResults::AssemblyResults& results;
    AssemblyResultsGatherer(InstrumentResults::AssemblyResults& _results) : results(_results) {}

    void operator() (ModuleDescriptor& module) {
#ifdef DUMP_INSTRUMENT_RESULT
        LOGINFO1(DUMP_RESULTS,"  Assembly %s", module.assemblyName.c_str());
#endif
        InstrumentResults::AssemblyResult assemblyResult;
        assemblyResult.name.swap(module.assemblyName);
        assemblyResult.moduleName.swap(module.moduleName);

        std::for_each(module.typeDefs.begin(), module.typeDefs.end(), TypedefResultsGatherer(assemblyResult.types));

        if (assemblyResult.types.size() > 0)
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

void Instrumentator::StoreResults(InstrumentResults& results) {
    InstrumentResults::AssemblyResults instrumentResults;
    InstrumentResults::FileItems fileTable;

#ifdef DUMP_INSTRUMENT_RESULT
    LOGINFO(DUMP_RESULTS, "");
    LOGINFO(DUMP_RESULTS, "Dump results for intrumentator");
#endif

    std::for_each(m_descriptors.begin(), m_descriptors.end(), AssemblyResultsGatherer(instrumentResults));
    std::for_each(FileMap.begin(), FileMap.end(), FileMapGatherer(fileTable));

    results.Assign( instrumentResults );
    results.Assign( fileTable );
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
