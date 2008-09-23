#include "stdafx.h"
#include "defines.h"
#include "interface.h"
#include "environment.h"
#include "logging.h"
#include "interface.h"
#include "message.h"
#include "message_pipe.h"
#include "corprofiler_options.h"
#include "function_map.h"
#include "il_instrumentedbody.h"
#include "il_instrumentator.h"
#include "instrumented_results.h"
#include "rules.h"
#include "corprofiler.h"
#include "helpers.h"
#include "corhelper.h"
#include <conio.h>

CorProfiler* CorProfiler::m_currentInstance = 0;

HRESULT CoCreateInstanceWithoutModel(REFCLSID rclsid, REFIID riid, void **ppv);

void CorProfiler::FinalizeInstance() {
    if (0 != m_currentInstance)
        m_currentInstance->Shutdown();
}

CorProfiler::CorProfiler() : m_instrumentator(m_rules) {
    ATLTRACE("CorProfiler::CorProfiler");

	ITransferrableFactory* list[] = { this };
	m_center.SetMessageMap(list, ARRAYSIZE(list));
}

STDMETHODIMP CorProfiler::Initialize( /* [in] */ IUnknown *pICorProfilerInfoUnk )
{
    HRESULT hr;

    ATLTRACE("CorProfiler::Initialize");

    m_currentInstance = this;
    m_options.InitializeFromEnvironment();

	DWORD dwSize;
    LPCTSTR messageCenterOption = Environment::GetEnvironmentStringOption(OPTION_MESSOPT, &dwSize);
    if(FAILED(hr = m_center.Connect(messageCenterOption))) {
        Environment::FreeStringResource(messageCenterOption);
        return hr;
    }
    Environment::FreeStringResource(messageCenterOption);

    DriverLog& log = DriverLog::get();

	ATLTRACE("CorProfiler::Initialize - send C_RequestStart");
	m_center.Send(Messages::Message<Messages::C_RequestStart>());

    ATLTRACE("CorProfiler::Initialize - wait for C_EndOfInputs");
	ITransferrable* message;

	struct Initializer : public ITransferrableVisitor
	{
	public:
		bool readyToGo;
		Initializer() : readyToGo(false) {}

		void on(MessageType type) { if (type == Messages::C_EndOfInputs) readyToGo = true; }
		void on(FunctionMap&) {}
		void on(Rules&) {}
		void on(InstrumentResults &) {}
	} messageVisitor;

    while(SUCCEEDED(m_center.Wait(message))) 
	{
		message->visit(messageVisitor);
		destroy(message);

		if (messageVisitor.readyToGo) break;
    }

	if (!messageVisitor.readyToGo)
		return E_ABORT;

	ATLTRACE("CorProfiler::Initialize - send C_RequestStart");
	m_center.Send(Messages::Message<Messages::C_RequestStart>());

    m_profilerInfo = pICorProfilerInfoUnk;

    DWORD dwMask = 0;
    //TODO:
    //if (m_rules.IsEnabledMode(COUNT_CALL_DIAGRAM)) {
    //    dwMask |= COR_PRF_MONITOR_JIT_COMPILATION|COR_PRF_MONITOR_ENTERLEAVE;
    //    m_callGatherer.Initialize(m_profilerInfo);
    //}
    if (m_rules.IsEnabledMode(COUNT_COVERAGE)) {
        dwMask |= COR_PRF_MONITOR_CLASS_LOADS|COR_PRF_MONITOR_MODULE_LOADS|COR_PRF_MONITOR_ASSEMBLY_LOADS|COR_PRF_DISABLE_INLINING|COR_PRF_DISABLE_OPTIMIZATIONS;
        if (FAILED(hr = CoCreateInstanceWithoutModel(CLSID_CorSymBinder_SxS, IID_ISymUnmanagedBinder2, (void**) &m_binder))) {
            LOGINFO(PROFILER_CALL_METHOD, "Cannot create symbol binder. Work as usual");
        }
    }
    
    ATLTRACE("CorProfiler::Initialize - set event mask");
    m_profilerInfo->SetEventMask(dwMask);

    LOGINFO(PROFILER_CALL_METHOD, "CorProfiler was successfully turning on with:");
    m_options.DumpOptions();
    m_rules.Dump();

    ATLTRACE("CorProfiler::Initialize - CorProfiler was successfully turning on");
    return S_OK;   
}

STDMETHODIMP CorProfiler::Shutdown( void )
{
    ATLTRACE("CorProfiler::Shutdown");

    m_instrumentator.StoreResults(m_instrumentResults);

	ITransferrable* results[] = {
		&m_functions, 
		&m_instrumentResults
	};

    for(int res_count = ARRAYSIZE(results) - 1; res_count >= 0; --res_count) {
        m_center.Send(*results[res_count]);
    }

    ATLTRACE("CorProfiler::Shutdown - send eEndOfResult");
	m_center.Send(Messages::Message<Messages::C_EndOfResults>());

    ATLTRACE("CorProfiler::Shutdown - wait for eClose");
	struct Shutdowner : ITransferrableVisitor {
	public:
		bool readyToDown;
		Shutdowner() : readyToDown(false) {}
		void on(MessageType type) { if (Messages::C_RequestShutdown == type) readyToDown = true; }
		void on(FunctionMap& value) {}
		void on(Rules& value) {}
		void on(InstrumentResults& value) {}
	} shutdowner;

	ITransferrable* message;
	while(SUCCEEDED(m_center.Wait(message))) 
	{
		message->visit(shutdowner);
		if (shutdowner.readyToDown) break;
    }

    m_currentInstance = 0;

	if (shutdowner.readyToDown) 
	{
		ATLTRACE("CorProfiler is turned off");
	    DriverLog::get().WriteLine(_T("CorProfiler is turned off"));
	}
	else 
	{
	    ATLTRACE("CorProfiler is turned off (without eClose message)");
		DriverLog::get().WriteLine(_T("CorProfiler is turning off (without eClose message)"));
	}
    return S_OK; 
}

ITransferrable* CorProfiler::create(MessageType type)
{
	switch(type) {
		case Messages::C_FunctionMap: return &this->m_functions; 
		case Messages::C_Rules: return &this->m_rules;
		case Messages::C_InstrumentResults: return &this->m_instrumentResults;
		default: return new Messages::GenericMessage(type);
	}
}

void CorProfiler::destroy(ITransferrable* item)
{
	const ITransferrable* ignore_list[] = { &m_functions, &m_rules, &m_instrumentResults };
	const ITransferrable** lbeg = ignore_list;
	const ITransferrable** lend = ignore_list + ARRAYSIZE(ignore_list);
	
	if(item == 0 || lend != std:: find(lbeg, lend, item)) return;
	
	delete item;
}

#define NumItems( s ) (sizeof( s ) / sizeof( s[0] ))
typedef HRESULT __stdcall DLLGETCLASSOBJECT( REFCLSID rclsid, REFIID riid, void **ppv );
IID IID_ICF = {0x00000001, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

HRESULT CoCreateInstanceWithoutModel( REFCLSID rclsid, REFIID riid, void **ppv )
{
    HKEY key;
    DWORD len;
    LONG result;
    HRESULT hr = S_OK;
    OLECHAR guidString[128];

	wchar_t keyString[1024];
    wchar_t dllName[MAX_PATH];

    StringFromGUID2( rclsid, guidString, NumItems( guidString ) );

#ifndef _UNICODE
    wchar_t szID[64];              // the class ID to register.

    WideCharToMultiByte( CP_ACP, 0, guidString, -1, szID, sizeof( szID ), NULL, NULL );
    _stprintf_s( keyString, 1024, _T("CLSID\\%s\\InprocServer32"), szID );
#else
    _stprintf_s( keyString, 1024, _T("CLSID\\%s\\InprocServer32"), guidString );
#endif

    // Lets grab the DLL name now.
    result = RegOpenKeyEx( HKEY_CLASSES_ROOT, keyString, 0, KEY_READ, &key );
    if ( result == ERROR_SUCCESS )
    {
        DWORD type;
        result = RegQueryValueEx( key, NULL, NULL, &type, NULL, &len );
        if ((result == ERROR_SUCCESS) && ((type == REG_SZ) || (type == REG_EXPAND_SZ)))
        {
            result = RegQueryValueEx( key, NULL, NULL, &type, (BYTE*) dllName, &len );
            if (result == ERROR_SUCCESS)
            {
                // We've got the name of the DLL to load, so load it.
                HINSTANCE dll;

                dll = LoadLibraryEx( dllName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
                if ( dll != NULL )
                {
                    // We've loaded the DLL, so find the DllGetClassObject function.
                    FARPROC func;

                    func = GetProcAddress( dll, "DllGetClassObject" );
                    if ( func != NULL )
                    {
                        // Cool, lets call the function to get a class factory for
                        // the rclsid passed in.
                        IClassFactory *classFactory;
                        DLLGETCLASSOBJECT *dllGetClassObject = (DLLGETCLASSOBJECT*)func;

                        hr = dllGetClassObject( rclsid, IID_ICF, (void**)&classFactory );
                        if ( SUCCEEDED( hr ) )
                        {
                            //
                            // Ask the class factory to create an instance of the
                            // necessary object.
                            //
                            hr = classFactory->CreateInstance( NULL, riid, ppv );

                            // Release that class factory!
                            classFactory->Release();
                        }
                    }
                    else
                        hr = HRESULT_FROM_WIN32( GetLastError() );
                }
                else
                    hr = HRESULT_FROM_WIN32( GetLastError() );
            }
            else
                hr = HRESULT_FROM_WIN32( GetLastError() );
        }
        else
            hr = HRESULT_FROM_WIN32( GetLastError() );
        RegCloseKey( key );
    }
    else
        hr = HRESULT_FROM_WIN32( GetLastError() );
    return hr;
}

STDMETHODIMP CorProfiler::AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus) {
    String asmName = CorHelper::GetAssemblyName(m_profilerInfo, assemblyId);
    LOGINFO2(PROFILER_CALL_METHOD, "Assembly %X loaded (%s)", assemblyId, asmName.length() == 0 ? _T("noname") : asmName.c_str());
    return S_OK;
}

STDMETHODIMP CorProfiler::AssemblyUnloadStarted(AssemblyID assemblyId) {
    String asmName = CorHelper::GetAssemblyName(m_profilerInfo, assemblyId);
    LOGINFO2(PROFILER_CALL_METHOD, "Assembly %X unloaded (%s)", assemblyId, asmName.length() == 0 ? _T("noname") : asmName.c_str());
    return S_OK; 
}

STDMETHODIMP CorProfiler::ClassLoadFinished(ClassID classId, HRESULT hrStatus) {
    String className = CorHelper::GetClassName(m_profilerInfo, classId);
    LOGINFO2(PROFILER_CALL_METHOD, "Class %X loaded (%s)", classId, className.length() == 0 ? _T("noname") : className.c_str());
    m_instrumentator.UpdateClassCode(classId, m_profilerInfo, m_binder);
    return S_OK;
}

STDMETHODIMP CorProfiler::ClassUnloadStarted(ClassID classId) {
    String className = CorHelper::GetClassName(m_profilerInfo, classId);
	LOGINFO2(PROFILER_CALL_METHOD, "Class %X unloaded (%s)", classId, className.length() == 0 ? _T("noname") : className.c_str());
    return S_OK;
}

STDMETHODIMP CorProfiler::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)  {
    String moduleName = CorHelper::GetModuleName(m_profilerInfo, moduleId);
    LOGINFO2(PROFILER_CALL_METHOD, "Module %X loaded (%s)", moduleId, moduleName.length() == 0 ? _T("noname") : moduleName.c_str());
    return S_OK;
}

STDMETHODIMP CorProfiler::ModuleUnloadStarted( ModuleID moduleId) {
    String moduleName = CorHelper::GetModuleName(m_profilerInfo, moduleId);
    LOGINFO2(PROFILER_CALL_METHOD, "Module %X unloaded (%s)", moduleId, moduleName.length() == 0 ? _T("noname") : moduleName.c_str());
    m_instrumentator.UnloadModule(moduleId);
    return S_OK;
}

STDMETHODIMP CorProfiler::ModuleAttachedToAssembly(ModuleID module, AssemblyID assembly) {
    String moduleName = CorHelper::GetModuleName(m_profilerInfo, module);
    String assemblyName = CorHelper::GetAssemblyName(m_profilerInfo, assembly);
    LOGINFO4(PROFILER_CALL_METHOD, "Module %X (%s) attached to assembly %X (%s)", 
        module, moduleName.length() == 0 ? _T("noname") : moduleName.c_str(),
        assembly, assemblyName.length() == 0 ? _T("noname") : assemblyName.c_str());
    m_instrumentator.InstrumentModule( module, moduleName.c_str(), m_profilerInfo, m_binder );
    return S_OK;   
}