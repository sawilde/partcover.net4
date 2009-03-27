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

public:
    PartCoverConnector2(void);
    ~PartCoverConnector2(void);

    STDMETHOD(StartTarget)(BSTR targetPath, BSTR targetWorkingDir, BSTR targetArguments, VARIANT_BOOL redirectOutput, IConnectorActionCallback* callback);

	STDMETHOD(put_LoggingLevel)(INT logLevel) { m_driverLogging = logLevel; return S_OK; }
	STDMETHOD(put_FileLoggingEnable)(VARIANT_BOOL enable) { m_useFileLogging = enable == VARIANT_TRUE; return S_OK; }
	STDMETHOD(put_PipeLoggingEnable)(VARIANT_BOOL enable) { m_usePipeLogging = enable == VARIANT_TRUE; return S_OK; }

    STDMETHOD(EnableOption)(ProfilerMode mode);
    STDMETHOD(WaitForResults)(VARIANT_BOOL delayClose, IConnectorActionCallback* callback);
    STDMETHOD(CloseTarget)();

    STDMETHOD(WalkFunctions)(IFunctionMapWalker* walker);
	STDMETHOD(GetReport)(IReportReceiver* walker);

    STDMETHOD(IncludeItem)(BSTR item);
    STDMETHOD(ExcludeItem)(BSTR item);

	STDMETHOD(get_HasTargetExitCode)(VARIANT_BOOL* exitRes) 
	{ 
		if (exitRes  == 0) return E_INVALIDARG; 
		*exitRes = m_targetExitCodeSet ? VARIANT_TRUE : VARIANT_FALSE; 
		return S_OK; 
	}

	STDMETHOD(get_TargetExitCode)(INT* exitCode) 
	{
		if (exitCode == 0) return E_INVALIDARG; 
		*exitCode = m_targetExitCode; 
		return S_OK; 
	}

	STDMETHOD(get_ProcessId)(INT* pid) 
	{ 
		if (pid == 0) return E_INVALIDARG; 
		*pid = pi.dwProcessId; 
		return S_OK; 
	}

	STDMETHOD(get_LogFilePath)(BSTR* logFilePath) 
	{
		if (logFilePath == 0) return E_INVALIDARG;
		*logFilePath = _bstr_t(m_logFile.c_str());
		return S_OK;
	}

	ITransferrable* create(MessageType type);
	void destroy(ITransferrable* item);
};

