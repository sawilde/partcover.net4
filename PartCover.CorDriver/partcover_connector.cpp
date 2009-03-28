#include "StdAfx.h"
#include "defines.h"
#include "interface.h"
#include "message.h"
#include "message_pipe.h"
#include "function_map.h"
#include "instrumented_results.h"
#include "rules.h"
#include "partcover_connector.h"
#include "helpers.h"

PartCoverConnector2::PartCoverConnector2(void) : 
    m_driverLogging(0), m_targetExitCodeSet(false)
{
	ITransferrableFactory* list[] = { this };
	m_center.SetMessageMap(list, ARRAYSIZE(list));
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
    VARIANT_BOOL redirectOutput,
	IConnectorActionCallback* callback)
{
    HRESULT hr;
    _bstr_t targetPath(p_targetPath);
    _bstr_t targetWorkingDir(p_targetWorkingDir);
    _bstr_t targetArguments(p_targetArguments);

    if (targetWorkingDir.length() == 0 || targetPath.length() == 0 )
        return E_INVALIDARG;

	if(callback != 0 ) callback->OpenMessagePipe();

    // init message center 
    if(FAILED(hr = m_center.Open()))
        return hr;

	if(callback != 0 ) callback->TargetSetEnvironmentVars();

    targetArguments = _bstr_t("\"") + targetPath + _bstr_t("\" ") + targetArguments;

    // get current working dir and settings
    StringMap env = ParseEnvironment();
    env[_T("Cor_Enable_Profiling")] = _T("1");
    env[_T("Cor_Profiler")] = _T("{") _T(DRIVER_CORPROFILER_GUID) _T("}");
    env[OPTION_MESSOPT] = m_center.getId();

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

	if(callback != 0 ) callback->TargetCreateProcess();

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

	if(callback != 0 ) callback->TargetWaitDriver();

    if (FAILED(hr = m_center.WaitForClient()))
        return hr;

	if(callback != 0 ) callback->DriverConnected();

	struct Starter : ITransferrableVisitor {
		IConnectorActionCallback* m_callback;
	public:
		bool readyToGo;
		Starter(IConnectorActionCallback* callback) : m_callback(callback), readyToGo(false) {}

		void on(MessageType type) { if (Messages::C_RequestStart == type) readyToGo = true; }
		void on(FunctionMap& value) {}
		void on(Rules& value) {}
		void on(InstrumentResults& value) {}
		void on(LogMessage& value) { if (m_callback != 0) m_callback->LogMessage(value.getThreadId(), value.getTicks(), _bstr_t(value.getMessage().c_str())); }
	} messageVisitor(callback);

	ITransferrable* message;
	while(SUCCEEDED(m_center.Wait(message)))
	{
		message->visit(messageVisitor);
		destroy(message);
		if (messageVisitor.readyToGo) break;
	}

	if (!messageVisitor.readyToGo)
	{
		ATLTRACE("PartCoverConnector2::StartTarget - C_RequestStart wait error");
		return E_ABORT;
	}
	

	if(callback != 0 ) callback->DriverSendRules();

	TraceTargetMemoryUsage(callback);

	m_center.Send(m_rules);
	m_center.Send(Messages::Message<Messages::C_EndOfInputs>());

	if(callback != 0 ) callback->DriverWaitEoIConfirm();

	messageVisitor.readyToGo = false;
	while(SUCCEEDED(m_center.Wait(message)))
	{
		message->visit(messageVisitor);
		destroy(message);

		if (messageVisitor.readyToGo) break;
	}

	return true;
}

STDMETHODIMP PartCoverConnector2::EnableOption(ProfilerMode mode)
{
    if (m_center.isOpen()) return E_ACCESSDENIED;
    m_rules.EnableMode(mode);
    return S_OK;
}

STDMETHODIMP PartCoverConnector2::WaitForResults(VARIANT_BOOL delayClose, IConnectorActionCallback* callback)
{
	m_targetExitCodeSet = false;

    if (!m_center.isOpen()) return E_ACCESSDENIED;

	HRESULT hr = S_OK;

	struct Waiter : ITransferrableVisitor {
		IConnectorActionCallback* m_callback;
	public:
		bool readyToDown;
		Waiter(IConnectorActionCallback* callback) : m_callback(callback), readyToDown(false) {}

		void on(MessageType type) { if(Messages::C_EndOfResults) readyToDown = true; }
		void on(FunctionMap& value) {}
		void on(Rules& value) {}
		void on(InstrumentResults& value) {}
		void on(LogMessage& value) { if (m_callback != 0) m_callback->LogMessage(value.getThreadId(), value.getTicks(), _bstr_t(value.getMessage().c_str())); }
	} waiter(callback);

	m_functions.SetCallback(callback);
	m_instrumentResults.SetCallback(callback);

	ITransferrable* message;
	while(SUCCEEDED(m_center.Wait(message)))
	{
		message->visit(waiter);
		destroy(message);
		if (waiter.readyToDown) break;
	}

	m_targetExitCodeSet = false;

	if (delayClose == VARIANT_FALSE) 
	{
		TraceTargetMemoryUsage(callback);

		if (callback != 0) callback->TargetRequestShutdown();
		m_center.Send(Messages::Message<Messages::C_RequestShutdown>());

		if (WAIT_OBJECT_0 == WaitForSingleObject(pi.hProcess, INFINITE)) 
		{
			DWORD exitCode; 
			m_targetExitCodeSet = 0 != GetExitCodeProcess(pi.hProcess, &exitCode);
			m_targetExitCode = exitCode;
		}
	}

	return S_OK;
}

STDMETHODIMP PartCoverConnector2::CloseTarget() {
    if (!m_center.isOpen()) return E_ACCESSDENIED;
	return m_center.Send(Messages::Message<Messages::C_RequestClose>());
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
    if (m_center.isOpen()) 
        return E_ACCESSDENIED;

    if (item == 0 || !Rules::CreateRuleFromItem(item, 0)) 
        return E_INVALIDARG;

    m_rules.IncludeItem(item);
    return S_OK;
}

STDMETHODIMP PartCoverConnector2::ExcludeItem(BSTR item) {
    if (m_center.isOpen()) 
        return E_ACCESSDENIED;

    if (item == 0 || !Rules::CreateRuleFromItem(item, 0)) 
        return E_INVALIDARG;

    m_rules.ExcludeItem(item);
    return S_OK;
}

ITransferrable* PartCoverConnector2::create(MessageType type)
{
	switch(type) {
		case Messages::C_FunctionMap: return &this->m_functions; 
		case Messages::C_Rules: return &this->m_rules;
		case Messages::C_InstrumentResults: return &this->m_instrumentResults;
		case Messages::C_LogMessage: return &this->m_logMessage;
		default: return new Messages::GenericMessage(type);
	}
}

void PartCoverConnector2::destroy(ITransferrable* item)
{
	const ITransferrable* ignore_list[] = { &m_functions, &m_rules, &m_instrumentResults, &m_logMessage };
	const ITransferrable** lbeg = ignore_list;
	const ITransferrable** lend = ignore_list + ARRAYSIZE(ignore_list);
	
	if(item == 0 || lend != std:: find(lbeg, lend, item)) return;
	
	delete item;
}

void PartCoverConnector2::TraceTargetMemoryUsage(IConnectorActionCallback* callback)
{
	if (callback == 0) return;

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
	callback->ShowTargetMemory(mc);
}