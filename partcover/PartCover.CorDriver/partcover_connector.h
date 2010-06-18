#pragma once

#include "protocol.h"

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
{
protected:
    PROCESS_INFORMATION pi;
	IConnectorActionCallback* m_callback;

	int m_targetExitCode;
	bool m_targetExitCodeSet;
	bool m_driverStopped;

	bool m_useFileLogging;
	bool m_usePipeLogging;
    int m_driverLogging;
	String m_logFile;

    PartCoverMessageServer m_intercommunication;

    Rules m_rules;
    FunctionMap m_functions;
    InstrumentResults m_instrumentResults;

private:
	void TraceTargetMemoryUsage();

public:
    PartCoverConnector2(void);
    ~PartCoverConnector2(void);

    STDMETHOD(StartTarget)(BSTR targetPath, BSTR targetWorkingDir, BSTR targetArguments, VARIANT_BOOL redirectOutput);

	STDMETHOD(put_LoggingLevel)(INT logLevel);
	STDMETHOD(put_FileLoggingEnable)(VARIANT_BOOL enable);
	STDMETHOD(put_PipeLoggingEnable)(VARIANT_BOOL enable);
	STDMETHOD(put_StatusCallback)(IConnectorActionCallback* callback);

    STDMETHOD(EnableOption)(ProfilerMode mode);
    STDMETHOD(WaitForResults)(VARIANT_BOOL delayClose);

    STDMETHOD(WalkFunctions)(IFunctionMapWalker* walker);
	STDMETHOD(GetReport)(IReportReceiver* walker);

    STDMETHOD(IncludeItem)(BSTR item);
    STDMETHOD(ExcludeItem)(BSTR item);

	STDMETHOD(get_HasTargetExitCode)(VARIANT_BOOL* exitRes);
	STDMETHOD(get_TargetExitCode)(INT* exitCode);
	STDMETHOD(get_ProcessId)(INT* pid);
	STDMETHOD(get_LogFilePath)(BSTR* logFilePath);
	STDMETHOD(get_StatusCallback)(IConnectorActionCallback** callback);

	void OnDriverRunning();
	void OnDriverStarting();
	void OnDriverStopping();
	void OnDriverStopped();

private:

	struct IntercommunicationFacade : public IIntercommunication 
	{
		IntercommunicationFacade(PartCoverConnector2& connector)
			: m_connector(connector)
		{
		}

		rpclib::RPCRESULT LogMessage(DWORD thread, DWORD tick, String message);
		rpclib::RPCRESULT GetSettings(int type);
		rpclib::RPCRESULT SetDriverState(int type);
		rpclib::RPCRESULT AddFunction(int id, String param1, String param2);
		rpclib::RPCRESULT GetRules(Rules *rules);
		rpclib::RPCRESULT Release(void) { return RPC_S_OK; }
		rpclib::RPCRESULT StoreResultFunctionMap(FunctionMap &map);
		rpclib::RPCRESULT StoreResultInstrumentation(InstrumentResults &map);

	private:
		PartCoverConnector2& m_connector;
	};

	IntercommunicationFacade m_aggregate;
};

