#pragma once

//[
//    object
//    ,uuid("9EAA2FB5-F2D0-4474-A403-5F4CE56FBE9C")
//    ,helpstring("CorDriver.IFunctionMapWalker interface")
//    ,library_block
//]
__interface IFunctionMapWalker : IUnknown {
    HRESULT Function(/*[in]*/ FunctionID funcId, /*[in]*/ BSTR className, /*[in]*/ BSTR functionName);
};

#pragma pack(push)
#pragma pack(1)

class FunctionMap : public IResultContainer
{
    struct FunctionInfo {
        FunctionID  functionId;
        std::wstring functionName;
        std::wstring className;
    };
    typedef stdext::hash_map<FunctionID, FunctionInfo> FunctionInfoArray;
    typedef FunctionInfoArray::iterator FunctionInfoIterator;

    FunctionInfoArray m_data;

public:
    FunctionMap(void);
    ~FunctionMap(void);

    void Register(FunctionID func, ICorProfilerInfo* info);

    void SendResults(MessageCenter&);
    bool ReceiveResults(Message&);

    void Walk(IFunctionMapWalker*);
};

#pragma pack(pop)
