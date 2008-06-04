#include "stdafx.h"
#include "defines.h"
#include "interface.h"
#include "environment.h"
#include "DriverLog.h"
#include "interface.h"
#include "MessageCenter.h"
#include "CorProfilerOptions.h"
#include "CoverageGatherer.h"
#include "callgatherer.h"
#include "FunctionMap.h"
#include "instrumentedilbody.h"
#include "Instrumentator.h"
#include "InstrumentResults.h"
#include "Rules.h"
#include "CorProfiler.h"
#include "helpers.h"
#include "corhelper.h"
#include <conio.h>

CorProfiler* CorProfiler::m_currentInstance = 0;

HRESULT CoCreateInstanceWithoutModel(REFCLSID rclsid, REFIID riid, void **ppv);

void CorProfiler::FinalizeInstance() {
    if (0 != m_currentInstance)
        m_currentInstance->Shutdown();
}

CorProfiler::CorProfiler() : m_instrumentator(m_rules) {}

STDMETHODIMP CorProfiler::Initialize( /* [in] */ IUnknown *pICorProfilerInfoUnk )
{
    HRESULT hr;
    ATLTRACE("CorProfiler::Initialize");

    m_currentInstance = this;
    m_options.InitializeFromEnvironment();

    LPCTSTR messageCenterOption = Environment::GetEnvironmentStringOption(OPTION_MESSOPT);
    if(FAILED(hr = m_center.Connect(messageCenterOption))) {
        Environment::FreeStringResource(messageCenterOption);
        return hr;
    }
    Environment::FreeStringResource(messageCenterOption);

    DriverLog& log = DriverLog::get();

    Message message;
    ATLTRACE("CorProfiler::Initialize - wait for eBeginWork or eEnableMode");
    while(SUCCEEDED(hr = m_center.WaitForOption(&message))) {
        if (message.code == eBeginWork) {
            ATLTRACE("CorProfiler::Initialize - receiving eBeginWork");
            break;
        } else if (message.code == eResult) {
            ATLTRACE("CorProfiler::Initialize - receiving rules");
            m_rules.ReceiveResults(message);
            ATLTRACE("CorProfiler::Initialize - send eOk");
            m_center.SendOption(eOk);
        } else {
            ATLTRACE("CorProfiler::Initialize - unknown startup option %d", message.code);
        }
    }

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

    IResultContainer* results[] = {&m_functions, &m_coverageGatherer, &m_callGatherer, &m_instrumentResults};
    for(int res_count = sizeof(results)/sizeof(results[0]); res_count > 0; --res_count) {
        results[res_count - 1]->SendResults(m_center);
        ATLTRACE("CorProfiler::Shutdown - wait for eOk after result");
        m_center.WaitForOption(eOk);
    }

    ATLTRACE("CorProfiler::Shutdown - send eEndOfResult");
    m_center.SendOption(eEndOfResult);

    ATLTRACE("CorProfiler::Shutdown - wait for eClose");
    if (SUCCEEDED(m_center.WaitForOption(eClose))) {
        m_currentInstance = 0;
        LOGINFO(PROFILER_CALL_METHOD, "CorProfiler was successfully turning off");
        ATLTRACE("CorProfiler was successfully turning off");
        return S_OK;
    }

    ATLTRACE("CorProfiler was successfully turning off (without eClose message)");
    DriverLog::get().WriteLine(_T("CorProfiler was turning off (without eClose message)"));
    m_currentInstance = 0;
    return S_OK; 
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
    OLECHAR guidString[64];
    char szID[64];              // the class ID to register.
    char keyString[1024];
    char dllName[MAX_PATH];

    StringFromGUID2( rclsid, guidString, NumItems( guidString ) );
    WideCharToMultiByte( CP_ACP, 0, guidString, -1, szID, sizeof( szID ), NULL, NULL );

    _stprintf( keyString, _T("CLSID\\%s\\InprocServer32"), szID );

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
    std::wstring asmName = CorHelper::GetAssemblyName(m_profilerInfo, assemblyId);
    LOGINFO2(PROFILER_CALL_METHOD, "Assembly %X loaded (%S)", assemblyId, asmName.length() == 0 ? L"noname" : asmName.c_str());
    return S_OK;
}

STDMETHODIMP CorProfiler::AssemblyUnloadStarted(AssemblyID assemblyId) {
    std::wstring asmName = CorHelper::GetAssemblyName(m_profilerInfo, assemblyId);
    LOGINFO2(PROFILER_CALL_METHOD, "Assembly %X unloaded (%S)", assemblyId, asmName.length() == 0 ? L"noname" : asmName.c_str());
    return S_OK; 
}

STDMETHODIMP CorProfiler::ClassLoadFinished(ClassID classId, HRESULT hrStatus) {
    std::wstring className = CorHelper::GetClassName(m_profilerInfo, classId);
    LOGINFO2(PROFILER_CALL_METHOD, "Class %X loaded (%S)", classId, className.length() == 0 ? L"noname" : className.c_str());
    m_instrumentator.UpdateClassCode(classId, m_profilerInfo, m_binder);
    return S_OK;
}

STDMETHODIMP CorProfiler::ClassUnloadStarted(ClassID classId) {
    std::wstring className = CorHelper::GetClassName(m_profilerInfo, classId);
    LOGINFO2(PROFILER_CALL_METHOD, "Class %X unloaded (%S)", classId, className.length() == 0 ? L"noname" : className.c_str());
    return S_OK;
}

STDMETHODIMP CorProfiler::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)  {
    std::wstring moduleName = CorHelper::GetModuleName(m_profilerInfo, moduleId);
    LOGINFO2(PROFILER_CALL_METHOD, "Module %X loaded (%S)", moduleId, moduleName.length() == 0 ? L"noname" : moduleName.c_str());
    return S_OK;
}

STDMETHODIMP CorProfiler::ModuleUnloadStarted( ModuleID moduleId) {
    std::wstring moduleName = CorHelper::GetModuleName(m_profilerInfo, moduleId);
    LOGINFO2(PROFILER_CALL_METHOD, "Module %X unloaded (%S)", moduleId, moduleName.length() == 0 ? L"noname" : moduleName.c_str());
    m_instrumentator.UnloadModule(moduleId);
    return S_OK;
}

STDMETHODIMP CorProfiler::ModuleAttachedToAssembly(ModuleID module, AssemblyID assembly) {
    std::wstring moduleName = CorHelper::GetModuleName(m_profilerInfo, module);
    std::wstring assemblyName = CorHelper::GetAssemblyName(m_profilerInfo, assembly);
    LOGINFO4(PROFILER_CALL_METHOD, "Module %X (%S) attached to assembly %X (%S)", 
        module, moduleName.length() == 0 ? L"noname" : moduleName.c_str(),
        assembly, assemblyName.length() == 0 ? L"noname" : assemblyName.c_str());
    m_instrumentator.InstrumentModule( module, moduleName.c_str(), m_profilerInfo, m_binder );
    return S_OK;   
}