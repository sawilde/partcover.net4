#include "StdAfx.h"
#include "driverlog.h"
#include <string>

DriverLog::DriverLog(void)
{
    startTick = ::GetTickCount();
    m_infoLevel = 0;
}

DriverLog::~DriverLog(void)
{
    Deinitialize();
}

DriverLog& DriverLog::get() {
    static DriverLog instance;
    return instance;
}

void DriverLog::Initialize(LPCTSTR file) {
    log_fp = CreateFile(file, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
}

void DriverLog::Deinitialize() {
    if (Active()) {
        CloseHandle(log_fp);
        log_fp = INVALID_HANDLE_VALUE;
    }
}

void DriverLog::WriteBuffer(LPCTSTR message, DWORD length) {
    if( Active() && message != 0 ) {
        DWORD messageWritten;
        WriteFile(log_fp, message, sizeof(TCHAR)*length, &messageWritten, NULL);
    }
}

void DriverLog::WriteLine(LPCTSTR message, ...) {
    m_cs.Enter();
    if( Active() && message != 0 ) {
        static TCHAR lineStart[512];
        int lineStartSize = _stprintf(lineStart, "[%4d][%6d]", ::GetCurrentThreadId(), ::GetTickCount() - startTick);
        WriteBuffer(lineStart, lineStartSize + 1);

        va_list args;
        va_start( args, message );

        int characters = _vsctprintf(message, args);
        if (characters != -1) {
            LPTSTR buffer = new TCHAR[characters + 5];
            if(-1 != _vstprintf(buffer, message, args)) {
                buffer[characters] = _T('\r');
                buffer[characters + 1] = _T('\n');
                WriteBuffer(buffer, characters + 2);
            }
            delete[] buffer;
        }
    }
    m_cs.Leave();
}


void DriverLog::WriteError(LPCTSTR className, LPCTSTR methodName, LPCTSTR message, ...) {
    m_cs.Enter();
    if( Active() && message != 0 ) {
        static TCHAR lineStart[512];
        int lineStartSize = _stprintf(lineStart, _T("[%4d][%6d][ERROR]<%s.%s> "), ::GetCurrentThreadId(), ::GetTickCount() - startTick, className, methodName);
        WriteBuffer(lineStart, lineStartSize + 1);

        va_list args;
        va_start( args, message );

        int characters = _vsctprintf(message, args);
        if (characters != -1) {
            LPTSTR buffer = new TCHAR[characters + 5];
            if(-1 != _vstprintf(buffer, message, args)) {
                buffer[characters] = _T('\r');
                buffer[characters + 1] = _T('\n');
                WriteBuffer(buffer, characters + 2);
            }
            delete[] buffer;
        }
    }
    m_cs.Leave();
}

bool DriverLog::CanWrite(int infoLevel) const {
    return (infoLevel <= m_infoLevel || m_infoLevel == -1) && infoLevel != 0 && Active();
}

void DriverLog::WriteInfo(int infoLevel, LPCTSTR message, ...) {
    m_cs.Enter();
    if (CanWrite(infoLevel) && message != 0 ) {
        static TCHAR lineStart[32];
        int lineStartSize = _stprintf(lineStart, _T("[%4d][%6d] "), ::GetCurrentThreadId(), ::GetTickCount() - startTick);
        if (lineStartSize != -1)
            WriteBuffer(lineStart, lineStartSize);

        va_list args;
        va_start( args, message );

        int characters = _vsctprintf(message, args);
        if (characters != -1) {
            LPTSTR buffer = new TCHAR[characters + 5];
            if(-1 != _vstprintf(buffer, message, args)) {
                buffer[characters] = _T('\r');
                buffer[characters + 1] = _T('\n');
                WriteBuffer(buffer, characters + 2);
            }
            delete[] buffer;
        }
    } 
    m_cs.Leave();
}
