#include "StdAfx.h"
#include "defines.h"
#include "interface.h"
#include "MessageCenter.h"
#include "FunctionMap.h"
#include "CoverageGatherer.h"
#include "CallGatherer.h"
#include "InstrumentResults.h"
#include "rules.h"
#include "partcoverconnector.h"
#include "helpers.h"

#ifdef _UNICODE
typedef std::wstring String;
#else
typedef std::string String;
#endif

struct compare_no_case {
    bool operator()( const String &lhs, const String &rhs ) const {
        return _tcsicmp( lhs.c_str(), rhs.c_str() ) < 0;
    }
};

typedef std::map<String, String, compare_no_case> StringMap;

PartCoverConnector::PartCoverConnector(void) : 
    m_driverLogging(0)
{
}

PartCoverConnector::~PartCoverConnector(void)
{
}

void ParseEnvironmentBlock(LPTSTR data, LPTSTR* op_varEnd, LPTSTR* op_valStart, LPTSTR* op_valEnd) {
    LPTSTR varEnd, valStart, valEnd;
    varEnd = data + 1;
    while(*varEnd != _T('=')) ++varEnd;
    valEnd = valStart = varEnd + 1;
    while(*valEnd != 0) ++valEnd;
    *op_varEnd = varEnd;
    *op_valStart = valStart;
    *op_valEnd = valEnd;
} 

StringMap ParseEnvironment() {
    LPTSTR data = ::GetEnvironmentStrings();
    StringMap result;

    LPTSTR current_token = data;
    do {
        LPTSTR varEnd, valStart, valEnd;    
        ParseEnvironmentBlock(current_token, &varEnd, &valStart, &valEnd);
        String name(current_token, varEnd - current_token);
        String value(valStart, valEnd - valStart);
        result.insert(StringMap::value_type(name, value));
        current_token = valEnd + 1;
    } while( *current_token != 0 );

    ::FreeEnvironmentStrings(data);
    return result;
}

LPTSTR CreateEnvironment(const StringMap& env) {
    size_t buffer_size = 0;
    // get whole buffer size
    for(StringMap::const_iterator it = env.begin(); it != env.end(); ++it)
        buffer_size += it->first.length() + 1 + it->second.length() + 1;
    ++buffer_size;
    // initialize buffer
    LPTSTR buffer = new TCHAR[buffer_size];
    // store for format
    LPTSTR buffer_data = buffer;
    // copy data
    for(StringMap::const_iterator it = env.begin(); it != env.end(); ++it) {
        _stprintf(buffer, _T("%s"), it->first.c_str());
        buffer += it->first.length();
        *buffer++ = _T('=');
        _stprintf(buffer, _T("%s"), it->second.c_str());
        buffer += it->second.length();
        *buffer++ = 0;
    }
    *buffer = 0;
    return buffer_data;
}

void FreeEnvironment( LPTSTR buffer ) {
    delete[] buffer;
}

STDMETHODIMP PartCoverConnector::StartTarget(
    BSTR p_targetPath, 
    BSTR p_targetWorkingDir, 
    BSTR p_targetArguments,
    VARIANT_BOOL redirectOutput)
{
    HRESULT hr;
    _bstr_t targetPath(p_targetPath);
    _bstr_t targetWorkingDir(p_targetWorkingDir);
    _bstr_t targetArguments(p_targetArguments);

    if (targetWorkingDir.length() == 0 || targetPath.length() == 0 )
        return E_INVALIDARG;

    // init message center 
    if(FAILED(hr = m_center.Open()))
        return hr;

    targetArguments = _bstr_t("\"") + targetPath + _bstr_t("\" ") + targetArguments;

    // get current working dir and settings
    StringMap env = ParseEnvironment();
    env[_T("Cor_Enable_Profiling")] = _T("1");
    env[_T("Cor_Profiler")] = _T("{") _T(DRIVER_CORPROFILER_GUID) _T("}");
    env[OPTION_MESSOPT] = m_center.getId();

    if (m_driverLogging > 0) {
        DWORD curLength = ::GetCurrentDirectory(0, NULL);
        DynamicArray<TCHAR> curBuffer(curLength + 25);
        if (curLength = ::GetCurrentDirectory(curLength + 1, curBuffer)) {
            _stprintf(curBuffer + curLength, DRIVER_LOG_FILENAME);
            env[OPTION_LOGFILE] = curBuffer;
            _stprintf(curBuffer, _T("%d"), m_driverLogging);
            env[OPTION_VERBOSE] = curBuffer;
        }
    }

    // copy old and new env settings
    LPTSTR new_env = CreateEnvironment(env);

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
    _tcscpy(args, targetArguments);

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

    if (FAILED(hr = m_center.WaitForClient()))
        return hr;

    m_rules.SendResults(m_center);

    ATLTRACE("PartCoverConnector::StartTarget - wait for eOk");
    if(FAILED(hr = m_center.WaitForOption(eOk)))
        return hr;

    ATLTRACE("PartCoverConnector::BeginWork - sending eBeginWork");
    if(FAILED(hr = m_center.SendOption(eBeginWork)))
        return hr;

    return S_OK;
}

STDMETHODIMP PartCoverConnector::SetVerbose(INT enable)
{
    if (m_center.isOpen()) return E_ACCESSDENIED;
    m_driverLogging = enable;
    return S_OK;
}

STDMETHODIMP PartCoverConnector::EnableOption(ProfilerMode mode)
{
    if (m_center.isOpen()) return E_ACCESSDENIED;
    m_rules.EnableMode(mode);
    return S_OK;
}

STDMETHODIMP PartCoverConnector::WaitForResults(VARIANT_BOOL delayClose) {
    if (!m_center.isOpen()) return E_ACCESSDENIED;

    HRESULT hr = S_OK;
    ATLTRACE("PartCoverConnector::ReceiveResult - waiting for eResult");
    Message message;
    while(SUCCEEDED(hr = m_center.WaitForOption(&message))) {
        if ( message.code == eResult ) {
            ATLTRACE("PartCoverConnector::ReceiveResult - eResult received");
            if (m_functions.ReceiveResults(message)) {
                ATLTRACE("PartCoverConnector::ReceiveResult - eResult: Function Map");
            } else if (m_callGatherer.ReceiveResults(message)) {
                ATLTRACE("PartCoverConnector::ReceiveResult - eResult: Call Tree");
            } else if (m_coverageGatherer.ReceiveResults(message)) {
                ATLTRACE("PartCoverConnector::ReceiveResult - eResult: Coverage Tree");
            } else if (m_instrumentResults.ReceiveResults(message)) {
                ATLTRACE("PartCoverConnector::ReceiveResult - eResult: Instrument Results");
            }
            ATLTRACE("PartCoverConnector::ReceiveResult - send eOk");
            m_center.SendOption(eOk);
        } else if (message.code == eEndOfResult) {
            if (delayClose == VARIANT_TRUE) {
                return S_OK;
            }
            ATLTRACE("PartCoverConnector::ReceiveResult - eEndOfResult received");
            ATLTRACE("PartCoverConnector::EndTarget - send eClose");
            return m_center.SendOption(eClose);
        } else
            return E_UNEXPECTED;
    }
    return hr;
}

STDMETHODIMP PartCoverConnector::CloseTarget() {
    if (!m_center.isOpen()) return E_ACCESSDENIED;
    return m_center.SendOption(eClose);
}

STDMETHODIMP PartCoverConnector::WalkFunctions(IFunctionMapWalker* walker) {
    if (walker == 0)
        return E_INVALIDARG;
    m_functions.Walk(walker);
    return S_OK;
}

STDMETHODIMP PartCoverConnector::WalkCallTree(ICallWalker* walker) {
    if (walker == 0)
        return E_INVALIDARG;
    m_callGatherer.Walk(walker);
    return S_OK;
}

STDMETHODIMP PartCoverConnector::WalkInstrumentedResults(IInstrumentedBlockWalker* walker) {
    if (walker == 0)
        return E_INVALIDARG;
    m_instrumentResults.WalkResults(*walker);
    return S_OK;
}

STDMETHODIMP PartCoverConnector::IncludeItem(BSTR item) {
    if (m_center.isOpen()) 
        return E_ACCESSDENIED;

    if (item == 0 || !Rules::CreateRuleFromItem(item, 0)) 
        return E_INVALIDARG;

    m_rules.IncludeItem(item);
    return S_OK;
}

STDMETHODIMP PartCoverConnector::ExcludeItem(BSTR item) {
    if (m_center.isOpen()) 
        return E_ACCESSDENIED;

    if (item == 0 || !Rules::CreateRuleFromItem(item, 0)) 
        return E_INVALIDARG;

    m_rules.ExcludeItem(item);
    return S_OK;
}
