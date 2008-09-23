#include "StdAfx.h"
#include "cor.h"
#include "corsym.h"
#include "corprof.h"
#include "interface.h"
#include "message.h"
#include "message_pipe.h"
#include "function_map.h"
#include "helpers.h"

bool FunctionMap::SendData(MessagePipe& pipe)
{
	if(!pipe.write(m_data.size()))
		return false;

	bool result = true;
    for(FunctionInfoIterator it = m_data.begin(); result && it != m_data.end(); ++it) 
	{
		result = pipe.write(it->first)
			&& pipe.write(it->second.className) 
			&& pipe.write(it->second.functionName);
    }

	return result;
}

bool FunctionMap::ReceiveData(MessagePipe& pipe) 
{
	return ReceiveData(pipe, 0);
}

bool FunctionMap::ReceiveData(MessagePipe& pipe, IConnectorActionCallback* callback) 
{
	if (callback) callback->FunctionsReceiveBegin();

	FunctionInfoArray::size_type func_count;
	if(!pipe.read(&func_count))
		return false;

	if (callback) callback->FunctionsCount(func_count);

	size_t step = max(1, func_count / 100);

	bool result = true;
    for(FunctionInfoArray::size_type i = 0; result && i < func_count; ++i) 
	{
        FunctionInfo func_info;
		result = pipe.read(&func_info.functionId)
			&& pipe.read(&func_info.className) 
			&& pipe.read(&func_info.functionName);
		m_data[func_info.functionId] = func_info;

		if (i % step == 0 && callback) callback->FunctionsReceiveStat(i);
    }

	if (callback) callback->FunctionsReceiveEnd();

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