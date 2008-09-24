#pragma once

#pragma pack(push)
#pragma pack(4)

#include "cor.h"
#include "corsym.h"
#include "corprof.h"
#include "corhlpr.h"

#pragma pack(pop)

#ifndef REMOVE_ATTRIBUTES
#include "defines.h"
#endif 

#ifndef REMOVE_ATTRIBUTES
[
    export
    ,uuid(PROFILERMODE_GUID)
    ,helpstring("CorDriver.IPartCoverConnector interface")
    ,library_block
]
#endif
enum ProfilerMode : char {
    COUNT_COVERAGE = 0x01,
//    COUNT_CALL_DIAGRAM = 0x02,
    COVERAGE_USE_CLASS_LEVEL = 0x04,
    COVERAGE_USE_ASSEMBLY_LEVEL = 0x08
};

#ifndef REMOVE_ATTRIBUTES
[
    object
    ,uuid(IINSTRUMENTEDBLOCKWALKER_GUID)
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

#ifndef REMOVE_ATTRIBUTES
[
    object
    ,uuid(ICONNECTORACTIONCALLBACK_GUID)
    ,helpstring("CorDriver.IConnectorCallback interface")
    ,library_block
]
#endif
__interface IConnectorActionCallback : IUnknown 
{
    HRESULT SetConnected(VARIANT_BOOL connected);

	HRESULT MethodsReceiveBegin();
	HRESULT MethodsReceiveStatus();
	HRESULT MethodsReceiveEnd();

	HRESULT InstrumentDataReceiveBegin();
	HRESULT InstrumentDataReceiveStatus();
	HRESULT InstrumentDataReceiveEnd();

	HRESULT InstrumentDataReceiveFilesBegin();
	HRESULT InstrumentDataReceiveFilesCount([in] size_t fileCount);
	HRESULT InstrumentDataReceiveFilesStat([in] size_t index);
	HRESULT InstrumentDataReceiveFilesEnd();

	HRESULT InstrumentDataReceiveCountersBegin();
	HRESULT InstrumentDataReceiveCountersAsmCount([in] size_t asmCount);
	HRESULT InstrumentDataReceiveCountersAsm(BSTR name, BSTR mod, [in] size_t typeDefCount);
	HRESULT InstrumentDataReceiveCountersEnd();

	HRESULT OpenMessagePipe();

	HRESULT TargetSetEnvironmentVars();
	HRESULT TargetCreateProcess();
	HRESULT TargetWaitDriver();
	HRESULT TargetRequestShutdown();

	HRESULT DriverConnected();
	HRESULT DriverSendRules();
	HRESULT DriverWaitEoIConfirm();

	HRESULT FunctionsReceiveBegin();
	HRESULT FunctionsCount([in] size_t count);
	HRESULT FunctionsReceiveStat([in] size_t index);
	HRESULT FunctionsReceiveEnd();

	HRESULT LogMessage([in] INT threadId, [in] LONG tick, [in] BSTR message);
};

#ifndef REMOVE_ATTRIBUTES
[
    object
    ,uuid(DRIVER_ICONNECTOR2_GUID)
    ,helpstring("CorDriver.IPartCoverConnector2 interface")
    ,library_block
]
#endif
__interface IPartCoverConnector2 
{
    HRESULT StartTarget([in] BSTR targetPath, [in] BSTR targetWorkingDir, [in] BSTR targetArguments, [in] VARIANT_BOOL redirectOutput, [in, optional] IConnectorActionCallback* callback);

    [propput] HRESULT LoggingLevel([in] INT logLevel);
	[propput] HRESULT FileLoggingEnable([in] VARIANT_BOOL exitCode);
	[propput] HRESULT PipeLoggingEnable([in] VARIANT_BOOL exitCode);

    HRESULT EnableOption([in] ProfilerMode mode);
    HRESULT WaitForResults([in] VARIANT_BOOL delayClose, [in, optional] IConnectorActionCallback* callback);
    HRESULT CloseTarget();

    HRESULT WalkInstrumentedResults([in] IInstrumentedBlockWalker* walker);

    HRESULT IncludeItem([in] BSTR item);
    HRESULT ExcludeItem([in] BSTR item);

	[propget] HRESULT HasTargetExitCode([out, retval] VARIANT_BOOL* exitCode);
	[propget] HRESULT TargetExitCode([out, retval] INT* exitCode);

	[propget] HRESULT LogFilePath([out, retval] BSTR* logFilePath);
	[propget] HRESULT ProcessId([out, retval] INT* pid);
	
};
