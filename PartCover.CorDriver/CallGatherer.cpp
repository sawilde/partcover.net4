#include "StdAfx.h"
#include "interface.h"
#include "callgatherer.h"
#include "MessageCenter.h"

CallGatherer* CallGatherer::current = 0;

CallGatherer::CallGatherer(void) {
    current = this;
}

CallGatherer::~CallGatherer(void) {
    current = 0;
}

void EnterNaked();
void LeaveNaked();
void TailcallNaked();

void CallGatherer::Initialize(ICorProfilerInfo* corProfilerInfo) {
    current = this;
    m_corProfilerInfo = corProfilerInfo;
    HRESULT hr = corProfilerInfo->SetEnterLeaveFunctionHooks(
        (FunctionEnter *)&EnterNaked,
        (FunctionLeave *)&LeaveNaked,
        (FunctionTailcall *)&TailcallNaked);
    ATLASSERT(SUCCEEDED(hr));
}

void CallGatherer::SendResults(MessageCenter& center)
{
    typedef const unsigned char* lpubyte;
    Message message;
    message.code = eResult;
    Message::byte_array& value = message.value;

    value.push_back(eCallGathererResult);

    size_t threadCount = m_threads.size();
    lpubyte threadCountPtr = (lpubyte)&threadCount;
    value.insert(value.end(), threadCountPtr, threadCountPtr + sizeof(size_t));
    for(ThreadCallTreeIterator it = m_threads.begin(); it != m_threads.end(); ++it) {
        ThreadID threadId = it->first;
        lpubyte threadIdPtr = (lpubyte)&threadId;
        value.insert(value.end(), threadIdPtr, threadIdPtr + sizeof(ThreadID));

        ThreadCallTree& tree = it->second;

        size_t callCount = tree.m_calls.size();
        lpubyte callCountPtr = (lpubyte)&callCount;
        value.insert(value.end(), callCountPtr, callCountPtr + sizeof(size_t));

        for(CallTreeAtomIterator trit = tree.m_calls.begin(); trit != tree.m_calls.end(); ++trit) {
            CallTreeAtom& atom = *trit;
            lpubyte functionPtr = (lpubyte)&atom.function;
            lpubyte reasonPtr = (lpubyte)&atom.reason;
            lpubyte ticksPtr = (lpubyte)&atom.ticks;
            value.insert(value.end(), functionPtr, functionPtr + sizeof(FunctionID));
            value.insert(value.end(), reasonPtr, reasonPtr + sizeof(CallReason));
            value.insert(value.end(), ticksPtr, ticksPtr + sizeof(DWORD));
        }
    }

    ATLTRACE("CallGatherer::SendResults - send eResult");
    center.SendOption(message);
}

bool CallGatherer::ReceiveResults(Message& message) {
    if (message.value.size() == 0 || message.value.front() != eCallGathererResult)
        return false;

    typedef unsigned char* lpubyte;

    Message::byte_array& value = message.value;
    Message::byte_array::iterator it = value.begin();

    size_t skipSize = 1;
    while(skipSize-- > 0) ++it;

    size_t  threadCount = 0;
    lpubyte threadCountPtr = (lpubyte) &threadCount;
    for(size_t i = 0; i < sizeof(size_t); ++i)
        *threadCountPtr++ = *it++;

    while(threadCount-- > 0) {
        ThreadCallTree callTree;

        ThreadID threadId;
        lpubyte  threadIdPtr = (lpubyte)&threadId;
        for(size_t i = 0; i < sizeof(size_t); ++i) *threadIdPtr++ = *it++;

        size_t callCount;
        lpubyte  callCountPtr = (lpubyte)&callCount;
        for(size_t i = 0; i < sizeof(size_t); ++i) *callCountPtr++ = *it++;

        while(callCount-- > 0) {
            CallTreeAtom atom;
            lpubyte functionPtr = (lpubyte)&atom.function;
            for(size_t i = 0; i < sizeof(FunctionID); ++i) *functionPtr++ = *it++;
            lpubyte reasonPtr = (lpubyte)&atom.reason;
            for(size_t i = 0; i < sizeof(CallReason); ++i) *reasonPtr++ = *it++;
            lpubyte ticksPtr = (lpubyte)&atom.ticks;
            for(size_t i = 0; i < sizeof(DWORD); ++i) *ticksPtr++ = *it++;
            callTree.m_calls.push_back(atom);
        }

        m_threads[threadId] = callTree;
    }

    return true;
}

void CallGatherer::OnEnter(FunctionID funcId) {
    m_cs.Enter();
    ThreadID threadId;
    if (SUCCEEDED(m_corProfilerInfo->GetCurrentThreadID(&threadId))) {
        ThreadCallTree& tree = m_threads[threadId];
        CallTreeAtom atom;
        atom.function = funcId;
        atom.reason = eEnter;
        atom.ticks = ::GetTickCount();
        tree.m_calls.push_back(atom);
    }
    m_cs.Leave();
}

void CallGatherer::OnLeave(FunctionID funcId) {
    m_cs.Enter();
    ThreadID threadId;
    if (SUCCEEDED(m_corProfilerInfo->GetCurrentThreadID(&threadId))) {
        ThreadCallTree& tree = m_threads[threadId];
        CallTreeAtom atom;
        atom.function = funcId;
        atom.reason = eLeave;
        atom.ticks = ::GetTickCount();
        tree.m_calls.push_back(atom);
    }
    m_cs.Leave();
}

void CallGatherer::OnTailcall(FunctionID funcId) {
    m_cs.Enter();
    ThreadID threadId;
    if (SUCCEEDED(m_corProfilerInfo->GetCurrentThreadID(&threadId))) {
        ThreadCallTree& tree = m_threads[threadId];
        CallTreeAtom atom;
        atom.function = funcId;
        atom.reason = eTailcall;
        atom.ticks = ::GetTickCount();
        tree.m_calls.push_back(atom);
    }
    m_cs.Leave();
}

void CallGatherer::Walk(ICallWalker* walker) {
    for(ThreadCallTreeIterator it = m_threads.begin(); it != m_threads.end(); ++it) {
        if (S_OK != walker->EnterThread(it->first))
            return;

        ThreadCallTree& tree = it->second;
        for(CallTreeAtomIterator trit = tree.m_calls.begin(); trit != tree.m_calls.end(); ++trit) {
            CallTreeAtom& atom = *trit;
            if (eEnter == atom.reason) {
                if(S_OK != walker->EnterFunction(atom.function, atom.ticks))
                    return;
            }
            else if (eLeave == atom.reason) {
                if(S_OK != walker->LeaveFunction(atom.function, atom.ticks))
                    return;
            }
            else if (eTailcall == atom.reason) {
                if(S_OK != walker->TailcallFunction(atom.function, atom.ticks))
                    return;
            }
        }
    }
}

void __stdcall EnterStub( FunctionID functionID )
{
    CallGatherer::current->OnEnter(functionID );
}

void __stdcall LeaveStub( FunctionID functionID )
{
    CallGatherer::current->OnLeave( functionID );
}

void __stdcall TailcallStub( FunctionID functionID )
{
    CallGatherer::current->OnTailcall( functionID );
}

void __declspec( naked ) EnterNaked()
{
    __asm
    {
        push eax
            push ecx
            push edx
            push [esp+16]
            call EnterStub
                pop edx
                pop ecx
                pop eax
                ret 4
    }
}


void __declspec( naked ) LeaveNaked()
{
    __asm
    {
        push eax
            push ecx
            push edx
            push [esp+16]
            call LeaveStub
                pop edx
                pop ecx
                pop eax
                ret 4
    }
}


void __declspec(naked) TailcallNaked()
{
    __asm
    {
        push eax
            push ecx
            push edx
            push [esp+16]
            call TailcallStub
                pop edx
                pop ecx
                pop eax
                ret 4
    }
}
