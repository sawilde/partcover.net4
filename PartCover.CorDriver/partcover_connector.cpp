#include "StdAfx.h"
#include "defines.h"
#include "interface.h"
#include "function_map.h"
#include "instrumented_results.h"
#include "rules.h"
#include "partcover_connector.h"
#include "helpers.h"

PartCoverConnector2::PartCoverConnector2(void) 
	: m_driverLogging(0)
	, m_targetExitCodeSet(false)
	, m_aggregate(*this)
	, m_driverStopped(false)
	, m_callback(0)
{
}

PartCoverConnector2::~PartCoverConnector2(void)
{
}

/** from header */
//----------------------------------------------------------

STDMETHODIMP PartCoverConnector2::put_LoggingLevel(INT logLevel) 
{ 
	m_driverLogging = logLevel; 
	return S_OK; 
}

STDMETHODIMP PartCoverConnector2::put_FileLoggingEnable(VARIANT_BOOL enable) 
{ 
	m_useFileLogging = enable == VARIANT_TRUE; 
	return S_OK; 
}

STDMETHODIMP PartCoverConnector2::put_PipeLoggingEnable(VARIANT_BOOL enable) 
{ 
	m_usePipeLogging = enable == VARIANT_TRUE; 
	return S_OK; 
}

STDMETHODIMP PartCoverConnector2::put_StatusCallback(IConnectorActionCallback* callback) 
{
	if (callback == 0) return E_INVALIDARG; 
	m_callback = callback; 
	return S_OK; 
}

STDMETHODIMP PartCoverConnector2::get_HasTargetExitCode(VARIANT_BOOL* exitRes) 
{ 
	if (exitRes  == 0) return E_INVALIDARG; 
	*exitRes = m_targetExitCodeSet ? VARIANT_TRUE : VARIANT_FALSE; 
	return S_OK; 
}

STDMETHODIMP PartCoverConnector2::get_TargetExitCode(INT* exitCode) 
{
	if (exitCode == 0) return E_INVALIDARG; 
	*exitCode = m_targetExitCode; 
	return S_OK; 
}

STDMETHODIMP PartCoverConnector2::get_StatusCallback(IConnectorActionCallback** callback) 
{
	if (callback == 0) return E_INVALIDARG; 
	*callback = m_callback; 
	return S_OK; 
}

STDMETHODIMP PartCoverConnector2::get_ProcessId(INT* pid) 
{ 
	if (pid == 0) return E_INVALIDARG; 
	*pid = pi.dwProcessId; 
	return S_OK; 
}

STDMETHODIMP PartCoverConnector2::get_LogFilePath(BSTR* logFilePath) 
{
	if (logFilePath == 0) return E_INVALIDARG;
	*logFilePath = _bstr_t(m_logFile.c_str());
	return S_OK;
}

//----------------------------------------------------------

STDMETHODIMP PartCoverConnector2::StartTarget(
    BSTR p_targetPath, 
    BSTR p_targetWorkingDir, 
    BSTR p_targetArguments,
    VARIANT_BOOL redirectOutput)
{
	if (m_intercommunication.IsStarted()) {
		m_intercommunication.Stop();
		m_intercommunication.Close();
	}

    _bstr_t targetPath(p_targetPath);
    _bstr_t targetWorkingDir(p_targetWorkingDir);
    _bstr_t targetArguments(p_targetArguments);

    if (targetWorkingDir.length() == 0 || targetPath.length() == 0 )
        return E_INVALIDARG;

	if (m_callback) m_callback->OpenMessagePipe();

    // init message center
	StringStream buffer;
	buffer << L"\\\\.\\pipe\\partcover.";
	buffer << ::GetCurrentProcessId();

	IntercommunicationProxy::instance.aggregate = &m_aggregate;

	if(!m_intercommunication.Create(buffer.str().c_str())) {
        return HRESULT_FROM_WIN32( ::GetLastError() );
	}

	if (m_callback) m_callback->TargetSetEnvironmentVars();

    targetArguments = _bstr_t("\"") + targetPath + _bstr_t("\" ") + targetArguments;

    // get current working dir and settings
    StringMap env = ParseEnvironment();
    env[_T("Cor_Enable_Profiling")] = _T("1");
    env[_T("Cor_Profiler")] = _T("{") _T(DRIVER_CORPROFILER_GUID) _T("}");
    env[OPTION_MESSOPT] = buffer.str();

    if (m_driverLogging > 0)
	{
		DynamicArray<TCHAR> curBuffer(5);
        int written = _stprintf_s(curBuffer, curBuffer.size(), _T("%d"), m_driverLogging);
        env[OPTION_VERBOSE] = String(curBuffer, written);
	}

	if (m_useFileLogging) {
        DWORD curLength = ::GetCurrentDirectory(0, NULL);
        DynamicArray<TCHAR> curBuffer(curLength + 25);
        if (curLength = ::GetCurrentDirectory(curLength + 1, curBuffer)) 
		{
            int written = _stprintf_s(curBuffer + curLength, 25, DRIVER_LOG_FILENAME);
            env[OPTION_LOGFILE] = String(curBuffer, written);

			m_logFile = curBuffer;
        }
    }

	if (m_usePipeLogging) {
        env[OPTION_LOGPIPE] = _T("1");
    }

    // copy old and new env settings
    LPTSTR new_env = CreateEnvironment(env);

	if (m_callback) m_callback->TargetCreateProcess();

    // extract 
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    if (redirectOutput == VARIANT_TRUE) {
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);
        si.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
    }

    ZeroMemory(&pi, sizeof(pi));

    DynamicArray<TCHAR> args(targetArguments.length() + 1);
    _tcscpy_s(args, targetArguments.length() + 1, targetArguments);

    BOOL created = ::CreateProcess(
                        NULL, // Application
                        args, // command line
                        NULL, // lpProcessAttributes,
                        NULL, // lpThreadAttributes,
                        redirectOutput == VARIANT_TRUE ? TRUE : FALSE, // bInheritHandles,
#ifdef _UNICODE
                        CREATE_UNICODE_ENVIRONMENT |
#endif
                        (redirectOutput == VARIANT_TRUE ? 0 : CREATE_NEW_CONSOLE),
                        new_env,
                        targetWorkingDir,
                        &si,
                        &pi );

    // clear environment
    FreeEnvironment(new_env);
    
    if (!created)
        return HRESULT_FROM_WIN32( ::GetLastError() );

	if (m_callback) m_callback->TargetWaitDriver();

	if (!m_intercommunication.Start()) {
        return HRESULT_FROM_WIN32( ::GetLastError() );
	}

	return S_OK;
}

STDMETHODIMP PartCoverConnector2::EnableOption(ProfilerMode mode)
{
    m_rules.EnableMode(mode);
    return S_OK;
}

STDMETHODIMP PartCoverConnector2::WaitForResults(VARIANT_BOOL delayClose)
{
	m_targetExitCodeSet = false;

	HRESULT hr = S_OK;

	m_functions.SetCallback(m_callback);
	m_instrumentResults.SetCallback(m_callback);

	DWORD dword;

	while(!m_driverStopped) {
		dword = WaitForSingleObject(pi.hProcess, 1000);
		m_targetExitCodeSet = WAIT_OBJECT_0 == dword;
		if (dword == WAIT_FAILED || dword == WAIT_OBJECT_0)
			break;
	}

	if (!m_targetExitCodeSet && delayClose == VARIANT_FALSE) 
	{
		dword = WaitForSingleObject(pi.hProcess, INFINITE);
		m_targetExitCodeSet = WAIT_OBJECT_0 == dword;
	}

	if (m_targetExitCodeSet) {
		m_targetExitCodeSet = 0 != GetExitCodeProcess(pi.hProcess, &dword);
		m_targetExitCode = dword;
	}

	m_intercommunication.Stop();
	m_intercommunication.Close();

	return S_OK;
}

STDMETHODIMP PartCoverConnector2::WalkFunctions(IFunctionMapWalker* walker) {
    if (walker == 0)
        return E_INVALIDARG;
    m_functions.Walk(walker);
    return S_OK;
}

STDMETHODIMP PartCoverConnector2::GetReport(IReportReceiver* walker) {
    if (walker == 0)
        return E_INVALIDARG;
	m_instrumentResults.GetReport(*walker);
    return S_OK;
}

STDMETHODIMP PartCoverConnector2::IncludeItem(BSTR item) {
    if (item == 0 || !Rules::CreateRuleFromItem(item, 0)) 
        return E_INVALIDARG;

    m_rules.IncludeItem(item);
    return S_OK;
}

STDMETHODIMP PartCoverConnector2::ExcludeItem(BSTR item) {
    if (item == 0 || !Rules::CreateRuleFromItem(item, 0)) 
        return E_INVALIDARG;

    m_rules.ExcludeItem(item);
    return S_OK;
}

void PartCoverConnector2::TraceTargetMemoryUsage()
{
	if (m_callback == 0) return;

	PROCESS_MEMORY_COUNTERS pmcs;
	ZeroMemory(&pmcs, sizeof(pmcs));

	// Set size of structure
	pmcs.cb = sizeof(pmcs);

	// Get memory usage
	if(::GetProcessMemoryInfo(pi.hProcess, &pmcs, sizeof(pmcs)) != TRUE)
	{
		return;
	}

	MEMORY_COUNTERS mc;
	mc.PageFaultCount = pmcs.PageFaultCount;
	mc.PeakWorkingSetSize = pmcs.PeakWorkingSetSize;
	mc.WorkingSetSize = pmcs.WorkingSetSize;
	mc.QuotaPeakPagedPoolUsage = pmcs.QuotaPeakPagedPoolUsage;
	mc.QuotaPagedPoolUsage = pmcs.QuotaPagedPoolUsage;
	mc.QuotaPeakNonPagedPoolUsage = pmcs.QuotaPeakNonPagedPoolUsage;
	mc.QuotaNonPagedPoolUsage = pmcs.QuotaNonPagedPoolUsage;
	mc.PagefileUsage = pmcs.PagefileUsage;
	mc.PeakPagefileUsage = pmcs.PeakPagefileUsage;

	m_callback->ShowTargetMemory(mc);
}

void PartCoverConnector2::OnDriverRunning()
{
}

void PartCoverConnector2::OnDriverStarting()
{
}

void PartCoverConnector2::OnDriverStopping()
{
}

void PartCoverConnector2::OnDriverStopped()
{
	m_driverStopped = true;
	TraceTargetMemoryUsage();
}

rpclib::RPCRESULT 
PartCoverConnector2::IntercommunicationFacade::LogMessage(DWORD thread, DWORD tick, String message)
{ 
	if (m_connector.m_callback) 
		m_connector.m_callback->LogMessage(thread, tick, _bstr_t(message.c_str()));
	return RPC_S_OK; 
}

rpclib::RPCRESULT 
PartCoverConnector2::IntercommunicationFacade::GetSettings(int type)
{ 
	return RPC_S_OK; 
}

rpclib::RPCRESULT 
PartCoverConnector2::IntercommunicationFacade::SetDriverState(int type)
{ 
	switch(type) 
	{
	case DriverState::Running:
		m_connector.OnDriverRunning();
		break;
	case DriverState::Starting:
		m_connector.OnDriverStarting();
		break;
	case DriverState::Stopping:
		m_connector.OnDriverStopping();
		break;
	case DriverState::Stopped:
		m_connector.OnDriverStopped();
		break;
	default:
		break;
	}
	return RPC_S_OK; 
}

rpclib::RPCRESULT 
PartCoverConnector2::IntercommunicationFacade::AddFunction(int id, String param1, String param2)
{ 
	return RPC_S_OK; 
}

rpclib::RPCRESULT 
PartCoverConnector2::IntercommunicationFacade::GetRules(Rules *rules)
{ 
	*rules = m_connector.m_rules;
	return RPC_S_OK; 
}

rpclib::RPCRESULT 
PartCoverConnector2::IntercommunicationFacade::StoreResultFunctionMap(FunctionMap &map)
{
	m_connector.m_functions.Swap(map);
	return RPC_S_OK; 
}

rpclib::RPCRESULT 
PartCoverConnector2::IntercommunicationFacade::StoreResultInstrumentation(InstrumentResults &map)
{
	m_connector.m_instrumentResults.Swap(map);
	return RPC_S_OK; 
}

