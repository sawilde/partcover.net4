
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2009, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.1
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <Windows.h>
#include <DbgHelp.h>

#include <cstddef>
#include <tchar.h>
#include <strsafe.h>

static const std::size_t MaxLen = MAX_PATH;

char SzMiniDumpFileName[MaxLen]    = {0};
char SzMiniDumpPath[MaxLen]        = {0};
char SzTemp[3*MaxLen]              = {0};

#pragma comment(lib, "dbghelp.lib")

void createMiniDump(EXCEPTION_POINTERS * pep)
{

    SYSTEMTIME st = {0};
    ::GetLocalTime(&st);

    DWORD dwPid = GetCurrentProcessId();

    StringCbPrintfA(
        SzTemp, 
        1000, 
        "%sMiniDump-%i-%i-%i-%i_%i-%i-%i.dmp", 
        SzMiniDumpPath, 
        dwPid, 
        st.wYear, 
        st.wMonth, 
        st.wDay,
        st.wHour,
        st.wMinute,
        st.wSecond);

    // Open the file.

    HANDLE hFile = CreateFileA( SzTemp, GENERIC_READ | GENERIC_WRITE, 
        0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

    if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) 
    {
        // Create the minidump.

        MINIDUMP_EXCEPTION_INFORMATION mdei; 

        mdei.ThreadId           = GetCurrentThreadId(); 
        mdei.ExceptionPointers  = pep; 
        mdei.ClientPointers     = FALSE; 

        MINIDUMP_TYPE mdt       = MiniDumpNormal; 

        BOOL rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), 
            hFile, mdt, (pep != 0) ? &mdei : 0, 0, 0 ); 

        if( !rv ) 
            _tprintf( _T("MiniDumpWriteDump failed. Error: %u \n"), GetLastError() ); 
        else 
            _tprintf( _T("Minidump created.\n") ); 

        // Close the file.

        CloseHandle( hFile ); 

    }
    else 
    {
        _tprintf( _T("CreateFile failed. Error: %u \n"), GetLastError() ); 
    }
}

////////////////////////////////////////////////////////////////////////////////
// WriteMemory function 
// 

bool WriteMemory( BYTE* pTarget, const BYTE* pSource, DWORD Size )
{
    DWORD ErrCode = 0;


    // Check parameters 

    if( pTarget == 0 )
    {
        assert( !"Target address is null." );
        return false;
    }

    if( pSource == 0 )
    {
        assert( !"Source address is null." );
        return false;
    }

    if( Size == 0 )
    {
        assert( !"Source size is null." );
        return false;
    }

    if( IsBadReadPtr( pSource, Size ) )
    {
        assert( !"Source is unreadable." );
        return false;
    }


    // Modify protection attributes of the target memory page 

    DWORD OldProtect = 0;

    if( !VirtualProtect( pTarget, Size, PAGE_EXECUTE_READWRITE, &OldProtect ) )
    {
        ErrCode = GetLastError();
        assert( !"VirtualProtect() failed." );
        return false;
    }


    // Write memory 

    memcpy( pTarget, pSource, Size );


    // Restore memory protection attributes of the target memory page 

    DWORD Temp = 0;

    if( !VirtualProtect( pTarget, Size, OldProtect, &Temp ) )
    {
        ErrCode = GetLastError();
        assert( !"VirtualProtect() failed." );
        return false;
    }


    // Success 

    return true;

}

// Patch for SetUnhandledExceptionFilter 
const BYTE PatchBytes[5] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 };

// Original bytes at the beginning of SetUnhandledExceptionFilter 
BYTE OriginalBytes[5] = {0};

bool EnforceFilter( bool bEnforce )
{
    DWORD ErrCode = 0;


    // Obtain the address of SetUnhandledExceptionFilter 

    HMODULE hLib = GetModuleHandle( _T("kernel32.dll") );

    if( hLib == NULL )
    {
        ErrCode = GetLastError();
        assert( !"GetModuleHandle(kernel32.dll) failed." );
        return false;
    }

    BYTE* pTarget = (BYTE*)GetProcAddress( hLib, "SetUnhandledExceptionFilter" );

    if( pTarget == 0 )
    {
        ErrCode = GetLastError();
        assert( !"GetProcAddress(SetUnhandledExceptionFilter) failed." );
        return false;
    }

    if( IsBadReadPtr( pTarget, sizeof(OriginalBytes) ) )
    {
        assert( !"Target is unreadable." );
        return false;
    }


    if( bEnforce )
    {
        // Save the original contents of SetUnhandledExceptionFilter 

        memcpy( OriginalBytes, pTarget, sizeof(OriginalBytes) );


        // Patch SetUnhandledExceptionFilter 

        if( !WriteMemory( pTarget, PatchBytes, sizeof(PatchBytes) ) )
            return false;

    }
    else
    {
        // Restore the original behavior of SetUnhandledExceptionFilter 

        if( !WriteMemory( pTarget, OriginalBytes, sizeof(OriginalBytes) ) )
            return false;

    }


    // Success 

    return true;

}



LPTOP_LEVEL_EXCEPTION_FILTER gPrevExceptionFilter = 0;

LONG __stdcall onUnhandledException( EXCEPTION_POINTERS* pep ) 
{
    createMiniDump(pep); 

    if (gPrevExceptionFilter)
    {
        return gPrevExceptionFilter(pep);
    }
    else
    {
        return EXCEPTION_EXECUTE_HANDLER; 
    }
}

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )  // warning C4996: '' was declared deprecated
#endif

void setMiniDumpExceptionFilter(bool enableGpfDialog)
{
    static bool called = false;
    if (!called)
    {
        called = true;

        // Find the directory containing the executable.

        char szModuleName[ MAX_PATH+1 ];
        GetModuleFileNameA(NULL, szModuleName, MAX_PATH+1);

        std::string moduleName(szModuleName);
        std::size_t pos = moduleName.find_last_of('\\');
        if (pos != std::string::npos)
        {
            moduleName = moduleName.substr(0, pos+1);
        }
        strncpy(
            SzMiniDumpPath, 
            moduleName.c_str(), 
            sizeof(SzMiniDumpPath)/sizeof(SzMiniDumpPath[0]));

        if (enableGpfDialog)
        {
            _tprintf( _T("Installing exception filter (gpf dialog enabled).\n") ); 

            gPrevExceptionFilter = 
                SetUnhandledExceptionFilter( onUnhandledException );

            EnforceFilter(true);
        }
        else
        {

            _tprintf( _T("Installing exception filter (gpf dialog disabled).\n") ); 

            SetUnhandledExceptionFilter( onUnhandledException );
            EnforceFilter(true);

            // Making doubly sure that the gpf dialog doesn't pop up.

            //DWORD dwFlags = SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS;
            //DWORD dwOldFlags = SetErrorMode(dwFlags);
            //SetErrorMode(dwOldFlags | dwFlags);
        }
    }
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
