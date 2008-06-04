#pragma once

//[
//    object
//    ,uuid("1C99BAAF-2D04-4103-9A03-6A4838D200F3")
//    ,helpstring("CorDriver.ICallWalker interface")
//    ,library_block
//]
__interface ICallWalker : IUnknown {
    HRESULT EnterThread(/*[in]*/ ThreadID threadId);
    HRESULT EnterFunction(/*[in]*/ FunctionID funcId, /*[in]*/ DWORD ticks);
    HRESULT LeaveFunction(/*[in]*/ FunctionID funcId, /*[in]*/ DWORD ticks);
    HRESULT TailcallFunction(/*[in]*/ FunctionID funcId, /*[in]*/ DWORD ticks);
};

class MessageCenter;

[
    export
    ,uuid("5B8DF888-1698-4164-84BF-DDE1A15FF5D4")
    ,helpstring("CorDriver.CallReason  enum")
    ,library_block
]
enum CallReason {
    eEnter,
    eLeave,
    eTailcall
};

struct CallTreeAtom {
    FunctionID function;
    CallReason reason;
    DWORD      ticks;
};
typedef std::list<CallTreeAtom>     CallTreeAtomArray;
typedef CallTreeAtomArray::iterator CallTreeAtomIterator;

struct ThreadCallTree {
    CallTreeAtomArray m_calls;
};
typedef std::map<ThreadID, ThreadCallTree> ThreadCallTreeMap;
typedef ThreadCallTreeMap::iterator        ThreadCallTreeIterator;

class CallGatherer : public IResultContainer
{
    CCriticalSection  m_cs;
    ThreadCallTreeMap m_threads;

    ICorProfilerInfo* m_corProfilerInfo;

public:
    CallGatherer(void);
    ~CallGatherer(void);

    static CallGatherer* current;

    void Initialize(ICorProfilerInfo* corProfilerInfo);

    void SendResults(MessageCenter&);
    bool ReceiveResults(Message&);

    void OnEnter(FunctionID funcId);
    void OnLeave(FunctionID funcId);
    void OnTailcall(FunctionID funcId);

    void Walk(ICallWalker* walker);
};
