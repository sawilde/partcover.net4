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

class FunctionMap : public ITransferrable
{
    struct FunctionInfo {
        FunctionID  functionId;
        String functionName;
        String className;
    };

    typedef stdext::hash_map<FunctionID, FunctionInfo> FunctionInfoArray;
    typedef FunctionInfoArray::iterator FunctionInfoIterator;

    FunctionInfoArray m_data;

public:

    void Register(FunctionID func, ICorProfilerInfo* info);

    void Walk(IFunctionMapWalker*);

	MessageType getType() const { return Messages::C_FunctionMap; }
	void visit(ITransferrableVisitor& visitor) { visitor.on(*this); }
	
    bool SendData(MessagePipe&);
    bool ReceiveData(MessagePipe&);
    bool ReceiveData(MessagePipe&, IConnectorActionCallback*);
};

#pragma pack(pop)
