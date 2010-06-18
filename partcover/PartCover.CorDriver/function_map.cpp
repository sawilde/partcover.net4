#include "StdAfx.h"
#include "cor.h"
#include "corsym.h"
#include "corprof.h"
#include "interface.h"
#include "function_map.h"
#include "helpers.h"

void FunctionMap::Register(FunctionID func, ICorProfilerInfo* info) {
    CComPtr<IMetaDataImport> pMDImport;
    mdToken token;
    if(SUCCEEDED(info->GetTokenAndMetaDataFromFunction(func, IID_IMetaDataImport, (IUnknown**) &pMDImport, &token))) {
        ULONG nameSize = 0;
        FunctionInfo functionInfo;

        ClassID classID;
        if(SUCCEEDED(info->GetFunctionInfo( func, &classID, NULL, NULL ))) 
        {
            mdToken classToken;
            info->GetClassIDInfo( classID, NULL, &classToken );
            if (classToken != mdTypeDefNil && SUCCEEDED(pMDImport->GetTypeDefProps( classToken, NULL, 0, &nameSize, NULL, NULL ))) {
				DynamicArray<TCHAR> buffer(nameSize + 1);
                pMDImport->GetTypeDefProps( classToken, buffer, nameSize + 1, &nameSize, NULL, NULL );
                functionInfo.className = buffer;
            }
        }

        if (SUCCEEDED(pMDImport->GetMethodProps((mdMethodDef) token, NULL, NULL, 0, &nameSize, NULL, NULL, NULL, NULL, NULL))) {
			DynamicArray<TCHAR> buffer(nameSize + 1);
            pMDImport->GetMethodProps((mdMethodDef) token, NULL, buffer, nameSize + 1, &nameSize, NULL, NULL, NULL, NULL, NULL);
            functionInfo.functionName = buffer;
        }

        m_data[func] = functionInfo;
    }
}

void FunctionMap::Walk(IFunctionMapWalker* walker) {
    for(FunctionInfoIterator it = m_data.begin(); it != m_data.end(); ++it) {
        FunctionInfo& info = it->second;
        CComBSTR className = info.className.c_str();
        CComBSTR functionName = info.functionName.c_str();
        if(FAILED(walker->Function(info.functionId, className, functionName)))
            return;
    }
}