#pragma once

//[
//    object
//    ,uuid("9EAA2FB5-F2D0-4474-A403-5F4CE56FBE9C")
//    ,helpstring("CorDriver.IFunctionMapWalker interface")
//    ,library_block
//]
__interface IFunctionMapWalker : IUnknown {
    HRESULT Function(FunctionID funcId, BSTR className, BSTR functionName);
};

interface IConnectorActionCallback;

#pragma pack(push)
#pragma pack(1)

class FunctionMap
{
    struct FunctionInfo {
        FunctionID  functionId;
        String functionName;
        String className;
    };

    typedef stdext::hash_map<FunctionID, FunctionInfo> FunctionInfoArray;
    typedef FunctionInfoArray::iterator FunctionInfoIterator;

    FunctionInfoArray m_data;

	IConnectorActionCallback* m_callback;

public:

    void Register(FunctionID func, ICorProfilerInfo* info);
	void SetCallback(IConnectorActionCallback* callback) { m_callback = callback; }
	void Walk(IFunctionMapWalker* );

	void Swap(FunctionMap& other) 
	{
		m_data.swap(other.m_data);
	}
};

#pragma pack(pop)
