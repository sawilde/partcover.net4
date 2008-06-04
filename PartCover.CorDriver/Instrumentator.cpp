#include "StdAfx.h"
#include "interface.h"
#include "MessageCenter.h"
#include "instrumentedilbody.h"
#include "instrumentator.h"
#include "instrumentResults.h"
#include "DriverLog.h"
#include "Rules.h"
#include "corerror.h"
#include "helpers.h"
#include "corhelper.h"

//#define DUMP_TYPEDEFS
//#define DUMP_METHODDEFS
//#define DUMP_SYM_SEQUENCE_POINTS
//#define DUMP_INSTRUMENT_RESULT

void DumpTypeDef(DriverLog& log, DWORD typeDefFlags, LPCWSTR typedefName);
void DumpMethodDef(DriverLog& log, DWORD flags, LPCWSTR methoddefName, DWORD implFlag);
void DumpSymSequencePoints(ULONG32 cPoints, ULONG32 offsets[], ISymUnmanagedDocument *documents[], ULONG32 lines[], ULONG32 columns[], ULONG32 endLines[], ULONG32 endColumns[]);

Instrumentator::Instrumentator(Rules& rules) : m_rules(rules) {}
Instrumentator::~Instrumentator(void) {}

struct CompareNoCase {
    bool operator () (const std::wstring& left, const std::wstring& right) const {
        return _wcsicmp(left.c_str(), right.c_str()) < 0;
    }
};

typedef std::map<std::wstring, ULONG32> FileUrlMap;
typedef std::pair<std::wstring, ULONG32> FileUrlMapPair;

FileUrlMap FileMap;

ULONG32 Instrumentator::GetFileUrlId(const std::wstring& url) {
    FileUrlMap::iterator it = FileMap.find(url);
    if (it != FileMap.end())
        return it->second;
    ULONG32 fileId = (ULONG32)(FileMap.size() + 1);
    FileMap.insert(FileUrlMapPair(url, fileId));
    return fileId;
}

void Instrumentator::InstrumentModule(ModuleID module, LPCWSTR moduleName, ICorProfilerInfo* profilerInfo, ISymUnmanagedBinder2* binder) {
    Lock();
    ULONG bufferSize;
    HRESULT hr;

    ModuleDescriptor desc;
    desc.module = module;
    desc.moduleName = moduleName;
    desc.loaded = true;

    if(FAILED(hr = profilerInfo->GetModuleInfo(module, NULL, 0, NULL, NULL, &desc.assembly))) {
        LOGINFO3(INSTRUMENT_METHOD, "Cannot instrument module %X '%S' (error %X on get module info)", module, moduleName, hr);
        Unlock();
        return;
    }

    desc.assemblyName = CorHelper::GetAssemblyName(profilerInfo, desc.assembly);
    if (desc.assemblyName.length() == 0) {
        LOGINFO3(INSTRUMENT_METHOD, "Cannot instrument module %X '%S' (no assembly name)", module, moduleName, hr);
        Unlock();
        return;
    }

    CComPtr<IMetaDataImport> mdImport;
	hr = profilerInfo->GetModuleMetaData(module, ofRead, IID_IMetaDataImport, (IUnknown**) &mdImport);
    if(S_OK != hr) 
	{
        LOGINFO3(INSTRUMENT_METHOD, "Cannot instrument module %X '%S' (error %X on get meta data import)", module, moduleName, hr);
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
        LOGINFO3(INSTRUMENT_METHOD, "In module %X '%S' instrumented %d items. Skip it", module, moduleName, desc.typeDefs.size());
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
    std::wstring typedefName = CorHelper::GetTypedefFullName(helper.mdImport, typeDef, &typeDefFlags);
    if (typedefName.length() == 0) {
        LOGERROR1("Instrumentator", "InstrumentTypedef", "GetTypedefFullName failed. Typedef %d skipped", typeDef);
        return;
    }
    TypedefDescriptorMap& typedefMap = helper.module->typeDefs;
#ifdef DUMP_TYPEDEFS
    DumpTypeDef(log, typeDefFlags, typedefName.c_str());
#endif

    if (!IsTdClass(typeDefFlags)) {
        LOGINFO1(SKIP_BY_STATE, "  Instrumentation skipped for typedef '%S' (not a class)", typedefName.c_str());
        return;
    }

    if( typedefMap.find( typeDef ) != typedefMap.end() ) {
        LOGINFO1(SKIP_BY_RULES, "  Instrumentation skipped for class '%S' (already instrumented)", typedefName.c_str());
        return;
    }

    const std::wstring& assName = helper.module->assemblyName;
    if (!m_rules.IsItemValidForReport(assName, typedefName)) {
        LOGINFO2(SKIP_BY_RULES, "  Instrumentation skipped for class [%S]%S (by rules)", assName.c_str(), typedefName.c_str());
        return;
    }

    std::wstring typeDefNamespace = RulesHelpers::ExtractNamespace(typedefName);
    if (typeDefNamespace.length() == 0) {
        LOGINFO1(SKIP_BY_STATE, "  Instrumentation skipped for class '%S' (have no namespace)", typedefName.c_str());
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
        LOGINFO1(SKIP_BY_STATE, "  Instrumentation skipped for class '%S' (no methods insturmented)", typeDefDescriptor.typeDefName.c_str());
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

    LPWSTR buffer = new WCHAR[bufferSize + 1];
    helper.mdImport->GetMethodProps(methodDef, NULL, buffer, bufferSize + 1, &bufferSize, NULL, NULL, NULL, NULL, NULL);
    std::wstring methodName = buffer;
    delete[] buffer;

#ifdef DUMP_METHODDEFS
    DumpMethodDef(log, attrs, methodName.c_str(), implFlags);
#endif

    MethodDefMap& methods = typeDef.methodDefs;
    if (methods.find(methodDef) != methods.end()) {
        log.WriteLine("  Instrumentation skipped for method '%S.%S' (already instrumented)", typeDef.typeDefName.c_str(), methodName.c_str());
        return;
    }

    LPCBYTE methodHeader = 0;
    ULONG methodSize = 0;
    if(FAILED(hr = helper.profilerInfo->GetILFunctionBody(helper.module->module, methodDef, &methodHeader, &methodSize))) {
        if(hr == CORPROF_E_FUNCTION_NOT_IL) {
            LOGINFO1(METHOD_INNER, "GetILFunctionBody failed for method '%S' (not il-method)", methodName.c_str());
        } else {
            LOGINFO2(METHOD_INNER, "GetILFunctionBody failed for method '%S' with code 0x%X", methodName.c_str(), hr);
        }
        return;
    }

    try {
        LOGINFO2(METHOD_INNER, "Starting instrumentation for methodDef %S.%S", typeDef.typeDefName.c_str(), methodName.c_str());
        std::auto_ptr<InstrumentedILBody> body(new InstrumentedILBody(methodHeader, methodSize));

        if(!body.get()) {
            LOGERROR2("Instrumentator", "InstrumentMethod", "Cannot create il body for %S.%S", typeDef.typeDefName.c_str(), methodName.c_str());
            return;
        }

        if(!body->IsBodyParsed()) {
            LOGERROR2("Instrumentator", "InstrumentMethod", "Cannot parse body for methodDef %S.%S. Skip methodDef", typeDef.typeDefName.c_str(), methodName.c_str());
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

                            block.fileId = (ULONG32) GetFileUrlId(url.data);
                            block.startLine = lines[i];
                            block.startColumn = columns[i];
                            block.endLine = endLines[i];
                            block.endColumn = endColumns[i];
                        }
                    } else {
                        LOGINFO3(METHOD_INNER, "Cannot exec GetSequencePoints for methodDef %S.%S. Error %X", typeDef.typeDefName.c_str(), methodName.c_str(), hr);
                    }
                } else {
                    LOGINFO3(METHOD_INNER, "Cannot exec GetSequencePointCount for methodDef %S.%S. Error %X", typeDef.typeDefName.c_str(), methodName.c_str(), hr);
                }
            } else {
                LOGINFO3(METHOD_INNER, "Cannot get symMethod for methodDef %S.%S. Error %X", typeDef.typeDefName.c_str(), methodName.c_str(), hr);
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

        LOGINFO4(INSTRUMENT_METHOD, "      Asm %X: Method %S.%S (0x%X) was instrumented and stored", helper.module->assembly, typeDef.typeDefName.c_str(), method.methodDefName.c_str(), method.methodDef);
    } catch(std::exception& ex) {
        LOGERROR1("Instrumentator", "InstrumentMethod", "Instrumentation breaked. %s", ex.what());
    }
}

void Instrumentator::UpdateClassCode(ClassID classId, ICorProfilerInfo* profilerInfo, ISymUnmanagedBinder2* binder) {
    HRESULT hr;

    ModuleID moduleId;
    mdTypeDef typeDef;
    if (FAILED(profilerInfo->GetClassIDInfo(classId, &moduleId, &typeDef))) {
        LOGINFO(INSTRUMENT_METHOD, "Cannot update code for class (no typedef)");
        return;
    }

    AssemblyID assembly;
    if (FAILED(profilerInfo->GetModuleInfo(moduleId, NULL, 0, NULL, NULL, &assembly))) {
        LOGINFO(INSTRUMENT_METHOD, "Cannot update code for class (no assembly)");
        return;
    }

    Lock();

    ModuleDescriptor* descriptor = GetModuleDescriptor(moduleId);
    if (descriptor == 0) {
        LOGINFO(INSTRUMENT_METHOD, "Cannot update code for class (no module)");
        Unlock();
        return;
    }

    TypedefDescriptorMap::iterator typeDefinitionIt = descriptor->typeDefs.find(typeDef);
    if (typeDefinitionIt == descriptor->typeDefs.end()) {
        LOGINFO(INSTRUMENT_METHOD, "Cannot update code for class (no data)");
        Unlock();
        return;
    }

    TypeDef& defDescriptor = typeDefinitionIt->second;
    MethodDefMap::iterator methodIt = defDescriptor.methodDefs.begin();
    while(methodIt != defDescriptor.methodDefs.end()) {
        MethodDef& method = (methodIt++)->second;
        if (method.bodyUpdated) {
            LOGINFO(INSTRUMENT_METHOD, "Cannot update code for class (il body already updated)");
            continue;
        }

        if (method.instrumentedBody == 0) {
            LOGINFO(INSTRUMENT_METHOD, "Cannot update code for class (no il body)");
            continue;
        }

        InstrumentedILBody& body = *method.instrumentedBody;

        LOGINFO(METHOD_INNER, "Construct new body");
        body.ConstructNewBody();

        body.DumpNewBody();

        CComPtr<IMethodMalloc> methodAllocator;
        LOGINFO(METHOD_INNER, "Get IL Function Body Allocator");
        if(FAILED(hr = profilerInfo->GetILFunctionBodyAllocator(moduleId, &methodAllocator)))  {
            LOGERROR2("Instrumentator", "InstrumentMethod", "GetILFunctionBodyAllocator failed for method '%S' in module %X", method.methodDefName.c_str(), moduleId);
            continue;
        }

        LOGINFO(METHOD_INNER, "Allocate method body");
        void* newBody = methodAllocator->Alloc( body.GetInstrumentedBodySize() );

        LOGINFO(METHOD_INNER, "Copy method body");
        memcpy(newBody, body.GetInstrumentedBody(), body.GetInstrumentedBodySize() );

        LOGINFO(METHOD_INNER, "Set new IL Function Body");
        if (SUCCEEDED(hr = profilerInfo->SetILFunctionBody(moduleId, method.methodDef, (LPCBYTE) newBody))) {
            LOGINFO4(INSTRUMENT_METHOD, "Asm %X: Method %S.%S (0x%X) il body updated", descriptor->assembly, defDescriptor.typeDefName.c_str(), method.methodDefName.c_str(), method.methodDef);
            method.bodyUpdated = true;
        } else {
            LOGERROR3("Instrumentator", "InstrumentMethod", "SetILFunctionBody failed for method '%S' in module %X with error %X", method.methodDefName.c_str(), moduleId, hr);
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
            LOGINFO2(DUMP_RESULTS,"      Method %S : %S, no instrumented body", method.methodDefName.c_str(), method.methodSig.c_str());
#endif
        } else {
            InstrumentedBlocks& blocks = method.instrumentedBody->GetInstrumentedBlocks();
#ifdef DUMP_INSTRUMENT_RESULT
            LOGINFO3(DUMP_RESULTS,"      Method %S : %S, %d instrumented blocks", method.methodDefName.c_str(), method.methodSig.c_str(), blocks.size());
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
        LOGINFO1(DUMP_RESULTS,"    Type %S", type.typeDefName.c_str());
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
        LOGINFO1(DUMP_RESULTS,"  Assembly %S", module.assemblyName.c_str());
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
        LOGINFO2(DUMP_RESULTS,"  File %d, Url %S", file.second, file.first.c_str());
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


void DumpTypeDef(DriverLog& log, DWORD typeDefFlags, LPCWSTR typedefName) {
    std::wstring flagString;
    if (IsTdClass(typeDefFlags)) flagString += L", class";
    if (IsTdInterface(typeDefFlags)) flagString += L", interface";
    if (IsTdNotPublic(typeDefFlags)) flagString += L", not public";
    if (IsTdPublic(typeDefFlags)) flagString += L", public";
    if (IsTdNestedPublic(typeDefFlags)) flagString += L", nested public";
    if (IsTdNestedPrivate(typeDefFlags)) flagString += L", nested private";
    if (IsTdNestedFamily(typeDefFlags)) flagString += L", nested family";
    if (IsTdNestedAssembly(typeDefFlags)) flagString += L", nested assembly";
    if (IsTdNestedFamANDAssem(typeDefFlags)) flagString += L", nested family and assembly";
    if (IsTdNestedFamORAssem(typeDefFlags)) flagString += L", nested family or assembly";
    if (IsTdAbstract(typeDefFlags)) flagString += L", abstract";
    if (IsTdSealed(typeDefFlags)) flagString += L", sealed";
    if (IsTdSpecialName(typeDefFlags)) flagString += L", special name";
    if (IsTdImport(typeDefFlags)) flagString += L", import";
    if (IsTdAnsiClass(typeDefFlags)) flagString += L", ansi class";
    if (IsTdUnicodeClass(typeDefFlags)) flagString += L", unicode class";
    if (IsTdAutoClass(typeDefFlags)) flagString += L", auto class";
    if (IsTdRTSpecialName(typeDefFlags)) flagString += L", rt special name";
    if (IsTdHasSecurity(typeDefFlags)) flagString += L", has security";
    log.WriteLine("  Typedef '%S'%S", typedefName, flagString.c_str());
}

void DumpMethodDef(DriverLog& log, DWORD flags, LPCWSTR methoddefName, DWORD implFlag) {
    std::wstring flagString;
    if (IsMdPrivateScope(flags)) flagString += L", private scope";
    if (IsMdPrivate(flags)) flagString += L", private";
    if (IsMdFamANDAssem(flags)) flagString += L", family and assembly";
    if (IsMdAssem(flags)) flagString += L", assembly";
    if (IsMdFamily(flags)) flagString += L", family";
    if (IsMdFamORAssem(flags)) flagString += L", family or assembly";
    if (IsMdPublic(flags)) flagString += L", public";
    if (IsMdStatic(flags)) flagString += L", static";
    if (IsMdFinal(flags)) flagString += L", sealed";
    if (IsMdVirtual(flags)) flagString += L", virtual";
    if (IsMdHideBySig(flags)) flagString += L", hide by sig";
    if (IsMdAbstract(flags)) flagString += L", abstract";
    if (IsMdSpecialName(flags)) flagString += L", special name";
    if (IsMdPinvokeImpl(flags)) flagString += L", pinvoke";
    if (IsMdUnmanagedExport(flags)) flagString += L", unmanaged export";
    if (IsMdRTSpecialName(flags)) flagString += L", rt special name";
    if (IsMdHasSecurity(flags)) flagString += L", has security";
    if (IsMdRequireSecObject(flags)) flagString += L", require secondary object";
    if (IsMdClassConstructorW(flags, methoddefName)) flagString += L", static constructor";
    if (IsMdInstanceInitializerW(flags, methoddefName)) flagString += L", constructor";

    if (IsMiIL(implFlag)) flagString += L", il";
    if (IsMiNative(implFlag)) flagString += L", native";
    if (IsMiOPTIL(implFlag)) flagString += L", optil";
    if (IsMiUnmanaged(implFlag)) flagString += L", unmanaged";
    if (IsMiManaged(implFlag)) flagString += L", managed";
    if (IsMiForwardRef(implFlag)) flagString += L", forward";
    if (IsMiPreserveSig(implFlag)) flagString += L", preserve sig";
    if (IsMiInternalCall(implFlag)) flagString += L", internal call";
    if (IsMiSynchronized(implFlag)) flagString += L", synchronized";
    if (IsMiNoInlining(implFlag)) flagString += L", no inline";

    log.WriteLine("    Methoddef '%S'%S", methoddefName, flagString.c_str());
}

void DumpSymSequencePoints(ULONG32 points, ULONG32 offsets[], ISymUnmanagedDocument *documents[], ULONG32 lines[], ULONG32 columns[], ULONG32 endLines[], ULONG32 endColumns[]) {
    LOGINFO(METHOD_INNER, "Method Sequence Points");
    LOGINFO(METHOD_INNER, "Offset        Line        Col         EndLine     EndCol    ");
    LOGINFO(METHOD_INNER, "============  ==========  ==========  ==========  ==========" );
    for (ULONG i=0; i < points; i++) {
        LOGINFO6(METHOD_INNER, "%4d(0x%04X)  %10d  %10d  %10d  %10d", offsets[i], offsets[i], lines[i], columns[i], endLines[i], endColumns[i]);
    }
}
