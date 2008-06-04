#include "StdAfx.h"
#include "cor.h"
#include "corsym.h"
#include "corprof.h"
#include "interface.h"
#include "messagecenter.h"
#include "functionmap.h"

FunctionMap::FunctionMap(void)
{
}

FunctionMap::~FunctionMap(void)
{
}

void FunctionMap::SendResults(MessageCenter& center)
{
    typedef const unsigned char* lpubyte;

    Message message;
    message.code = eResult;
    Message::byte_array& value = message.value;

    value.push_back(eFunctionMapResult);
    Message::push(value, m_data.size());
    for(FunctionInfoIterator it = m_data.begin(); it != m_data.end(); ++it) {
        FunctionID funcId = it->first;
        FunctionInfo& info = it->second;
        value.insert(value.end(), (lpubyte)&funcId, (lpubyte)&funcId + sizeof FunctionID);
        lpubyte className = (lpubyte)info.className.c_str();
        value.insert(value.end(), className, className + (info.className.length() + 1)* sizeof(std::wstring::value_type));
        lpubyte funcName = (lpubyte)info.functionName.c_str();
        value.insert(value.end(), funcName, funcName + (info.functionName.length() + 1)* sizeof(std::wstring::value_type));
    }

    ATLTRACE("FunctionMap::SendResults - send eResult");
    center.SendOption(message);
}

bool FunctionMap::ReceiveResults(Message& message) {
    typedef unsigned char* lpubyte;
    if (message.value.size() == 0 || message.value.front() != eFunctionMapResult)
        return false;
    Message::byte_array& value = message.value;
    Message::byte_array::iterator it = value.begin();

    size_t skipSize = 1;
    while(skipSize-- > 0) ++it;

    size_t  dataSize = 0;
    it = Message::pop(it, &dataSize);

    for(size_t i = 0; i < dataSize; ++i) {
        FunctionInfo info;
        lpubyte funcIdPtr = (lpubyte) &info.functionId;
        for(size_t ptrSize = 0; ptrSize < sizeof(FunctionID); ++ptrSize) 
            *funcIdPtr++ = *it++;
        info.className = (const std::wstring::value_type*) &*it;
        it += (info.className.length() + 1) * sizeof(std::wstring::value_type);
        info.functionName = (const std::wstring::value_type*) &*it;
        it += (info.functionName.length() + 1) * sizeof(std::wstring::value_type);

        m_data[info.functionId] = info;
    }
    return true;
}

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
                LPWSTR buffer = new WCHAR[nameSize + 1];
                pMDImport->GetTypeDefProps( classToken, buffer, nameSize + 1, &nameSize, NULL, NULL );
                functionInfo.className = buffer;
                delete[] buffer;
            }
        }

        if (SUCCEEDED(pMDImport->GetMethodProps((mdMethodDef) token, NULL, NULL, 0, &nameSize, NULL, NULL, NULL, NULL, NULL))) {
            LPWSTR buffer = new WCHAR[nameSize + 1];
            pMDImport->GetMethodProps((mdMethodDef) token, NULL, buffer, nameSize + 1, &nameSize, NULL, NULL, NULL, NULL, NULL);
            functionInfo.functionName = buffer;
            delete[] buffer;
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