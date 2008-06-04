#pragma once

#pragma pack(push)
#pragma pack(4)

#include "cor.h"
#include "corsym.h"
#include "corprof.h"
#include "corhlpr.h"

#pragma pack(pop)

class MessageCenter;
struct Message;

interface IResultContainer {
    virtual void SendResults(MessageCenter&) = 0;
    virtual bool ReceiveResults(Message&) = 0;
};

enum ResultHeader {
    eRules = 0x01,
    eFunctionMapResult,
    eCallGathererResult,
    eCoverageGathererResult,
    eInstrumentatorResult
};

enum MessageCode {
    eOk          = 0x00,
    eEnableMode  = 0x01,
    eBeginWork   = 0x10,
    eResult      = 0x21,
    eEndOfResult = 0x22,
    eClose       = 0xFF
};

#ifndef REMOVE_ATTRIBUTES
[
    export
    ,uuid("9BC23D20-04DE-4ee7-AB24-1E890C741F78")
    ,helpstring("CorDriver.IPartCoverConnector interface")
    ,library_block
]
#endif
enum ProfilerMode {
    COUNT_COVERAGE = 0x01,
//    COUNT_CALL_DIAGRAM = 0x02,
    COVERAGE_USE_CLASS_LEVEL = 0x04,
    COVERAGE_USE_ASSEMBLY_LEVEL = 0x08
};

#ifndef REMOVE_ATTRIBUTES
[
    object
    ,uuid("4BAD004E-1EF9-43d2-8D3A-095963E324EF")
    ,helpstring("CorDriver.IInstrumentedBlockWalker interface")
    ,library_block
]
#endif
__interface IInstrumentedBlockWalker : IUnknown {
    HRESULT BeginReport();

    HRESULT EnterTypedef(BSTR assemblyName, BSTR typedefName, DWORD flags);
    HRESULT EnterMethod(BSTR methodName, BSTR methodSig, DWORD flags, DWORD implFlags);
    HRESULT MethodBlock(ULONG position, ULONG blockLen, DWORD visitCount, ULONG32 fileId, ULONG32 startLine, ULONG32 startColumn, ULONG32 endLine, ULONG32 endColumn);
    HRESULT LeaveMethod();
    HRESULT LeaveTypedef();

    HRESULT RegisterFile(ULONG32 fileId, BSTR fileUrl);
    HRESULT EndReport();
};

