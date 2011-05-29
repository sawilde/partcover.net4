// PartCover.CorDriver.cpp : Implementation of DLL Exports.
#include "stdafx.h"
#include "resource.h"

#include "CorProfiler.h"

[module(
        type=dll
        ,name="PartCover"
        ,uuid="7D0E6AAB-C5FC-4103-AAD4-8BF3112A56C4"
        ,version="4.0"
        ,helpstring="PartCover module"
        ,resource_name="IDR_PARTCOVERCORDRIVER"
        )]
class CPartCoverCorDriverModule
{
public:
    BOOL WINAPI DllMain(DWORD dwReason, LPVOID lpReserved )
    {    
        // save off the instance handle for later use
        switch ( dwReason )
        {
            case DLL_PROCESS_ATTACH:
                //DisableThreadLibraryCalls( hInstance );
                break;
        
        
            case DLL_PROCESS_DETACH:
                // lpReserved == NULL means that we called FreeLibrary()
                // in that case do nothing
                if ( (lpReserved != NULL) && (CorProfiler::m_currentInstance != NULL) )
                    CorProfiler::m_currentInstance->Shutdown();

                break;  
        
            default:
                break;      
        }
   
        
        return TRUE;

    } // DllMain};
};
