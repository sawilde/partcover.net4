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
    export
    ,uuid(METHODBLOCK_GUID)
    ,helpstring("CorDriver.MethodBlockData")
    ,library_block
]
#endif
typedef struct BLOCK_DATA {
	INT position;
	INT blockLen;
	INT visitCount;
	INT fileId;
	INT startLine;
	INT startColumn;
	INT endLine;
	INT endColumn;
} BLOCK_DATA;

#ifndef REMOVE_ATTRIBUTES
[
    export
    ,uuid(MEMORYCOUNTERS_GUID)
    ,helpstring("CorDriver.MemoryCountersData")
    ,library_block
]
#endif
typedef struct MEMORY_COUNTERS
{
	DWORD  PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
} MEMORY_COUNTERS;

#ifndef REMOVE_ATTRIBUTES
[
    object
    ,uuid(IINSTRUMENTEDBLOCKWALKER_GUID)
    ,helpstring("CorDriver.IInstrumentedBlockWalker interface")
    ,library_block
]
#endif
__interface IReportReceiver : IUnknown {
    HRESULT RegisterFile([in] INT fileId, [in] BSTR fileUrl);
    HRESULT RegisterSkippedItem([in] BSTR assemblyName, [in] BSTR typedefName);

	HRESULT EnterAssembly([in] INT domain, [in] BSTR domainName, [in] BSTR assemblyName, [in] BSTR moduleName);
    HRESULT EnterTypedef([in] BSTR typedefName, [in] DWORD flags);
    HRESULT EnterMethod([in] BSTR methodName, [in] BSTR methodSig, [in] INT bodySize, [in] INT bodyLineCount, [in] INT bodySeqCount, [in] DWORD flags, [in] DWORD implFlags, [in] BOOL symbolEntryFound);
    HRESULT AddCoverageBlock([in] BLOCK_DATA blockData);
    HRESULT LeaveMethod();
    HRESULT LeaveTypedef();
	HRESULT LeaveAssembly();
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
	HRESULT ShowTargetMemory(MEMORY_COUNTERS counters);

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

	HRESULT InstrumentDataReceiveSkippedBegin();
	HRESULT InstrumentDataReceiveSkippedCount([in] size_t itemCount);
	HRESULT InstrumentDataReceiveSkippedStat([in] size_t index);
	HRESULT InstrumentDataReceiveSkippedEnd();

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
    HRESULT StartTarget([in] BSTR targetPath, [in] BSTR targetWorkingDir, [in] BSTR targetArguments, [in] VARIANT_BOOL redirectOutput);

    [propput] HRESULT LoggingLevel([in] INT logLevel);
	[propput] HRESULT FileLoggingEnable([in] VARIANT_BOOL exitCode);
	[propput] HRESULT PipeLoggingEnable([in] VARIANT_BOOL exitCode);
	[propput] HRESULT StatusCallback([in] IConnectorActionCallback* callback);

    HRESULT EnableOption([in] ProfilerMode mode);
    HRESULT WaitForResults([in] VARIANT_BOOL delayClose);

    HRESULT GetReport([in] IReportReceiver* receiver);

    HRESULT IncludeItem([in] BSTR item);
    HRESULT ExcludeItem([in] BSTR item);

	[propget] HRESULT HasTargetExitCode([out, retval] VARIANT_BOOL* exitCode);
	[propget] HRESULT TargetExitCode([out, retval] INT* exitCode);

	[propget] HRESULT LogFilePath([out, retval] BSTR* logFilePath);
	[propget] HRESULT ProcessId([out, retval] INT* pid);
	[propget] HRESULT StatusCallback([out, retval] IConnectorActionCallback** callback);
	
};
