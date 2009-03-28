#pragma once

[
    coclass
    ,uuid(DRIVER_CONNECTOR2_GUID)
    ,vi_progid(DRIVER_CONNECTOR_VI_PROGID)
    ,progid(DRIVER_CONNECTOR2_PROGID)
    ,threading(both)
    ,helpstring("CorDriver.PartCoverConnector2 Class")
    ,version(DRIVER_CONNECTOR2_VER)
]
class PartCoverConnector2
	: public IPartCoverConnector2
	, public ITransferrableFactory
{
protected:
    PROCESS_INFORMATION pi;

	int m_targetExitCode;
	bool m_targetExitCodeSet;

	bool m_useFileLogging;
	bool m_usePipeLogging;
    int m_driverLogging;
	String m_logFile;

    MessagePipe m_center;

    Rules m_rules;
    FunctionMap m_functions;
    InstrumentResults m_instrumentResults;
	LogMessage m_logMessage;

private:
	void TraceTargetMemoryUsage(IConnectorActionCallback* callback);

public:
    PartCoverConnector2(void);
    ~PartCoverConnector2(void);

    STDMETHOD(StartTarget)(BSTR targetPath, BSTR targetWorkingDir, BSTR targetArguments, VARIANT_BOOL redirectOutput, IConnectorActionCallback* callback);

	STDMETHOD(put_LoggingLevel)(INT logLevel);
	STDMETHOD(put_FileLoggingEnable)(VARIANT_BOOL enable);
	STDMETHOD(put_PipeLoggingEnable)(VARIANT_BOOL enable);

    STDMETHOD(EnableOption)(ProfilerMode mode);
    STDMETHOD(WaitForResults)(VARIANT_BOOL delayClose, IConnectorActionCallback* callback);
    STDMETHOD(CloseTarget)();

    STDMETHOD(WalkFunctions)(IFunctionMapWalker* walker);
	STDMETHOD(GetReport)(IReportReceiver* walker);

    STDMETHOD(IncludeItem)(BSTR item);
    STDMETHOD(ExcludeItem)(BSTR item);

	STDMETHOD(get_HasTargetExitCode)(VARIANT_BOOL* exitRes);
	STDMETHOD(get_TargetExitCode)(INT* exitCode);
	STDMETHOD(get_ProcessId)(INT* pid);
	STDMETHOD(get_LogFilePath)(BSTR* logFilePath);

	ITransferrable* create(MessageType type);
	void destroy(ITransferrable* item);
};

