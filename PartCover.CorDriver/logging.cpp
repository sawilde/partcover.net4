#include "StdAfx.h"
#include "logging.h"
#include "helpers.h"
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

void DriverLog::Initialize(const String& file)
{
	log_fp = CreateFile(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
	if (log_fp != 0) {
#ifdef _UNICODE
		DWORD bytesWritten;
		const char ECS2_Signature[] = { (char)0xFF, (char)0xFE };
		WriteFile(log_fp, ECS2_Signature, ARRAYSIZE(ECS2_Signature), &bytesWritten, NULL);
#endif
	}
}

void DriverLog::Deinitialize()
{
    if (!Active()) 
		return;

    CloseHandle(log_fp);
    log_fp = INVALID_HANDLE_VALUE;
}

void DriverLog::WriteBuffer(LPCTSTR message, DWORD length)
{
	if (!Active() || message == 0)
		return;

    DWORD messageWritten;
    WriteFile(log_fp, message, sizeof(TCHAR)*length, &messageWritten, NULL);
}

void DriverLog::WriteLine(LPCTSTR message, ...)
{
	if (!Active() || message == 0)
		return;

    m_cs.Enter();
    
    static TCHAR lineStart[512];
    int lineStartSize = _stprintf_s(lineStart, 512, _T("[%4d][%6d]"), ::GetCurrentThreadId(), ::GetTickCount() - startTick);
    WriteBuffer(lineStart, lineStartSize);

    va_list args;
    va_start( args, message );

    int characters = _vsctprintf(message, args);
    if (characters != -1) {
		DynamicArray<TCHAR> buffer(characters + 5);
        if(-1 != _vstprintf_s(buffer, characters + 5, message, args)) {
            buffer[characters] = _T('\r');
            buffer[characters + 1] = _T('\n');
            WriteBuffer(buffer, characters + 2);
        }
    }
    
    m_cs.Leave();
}


void DriverLog::WriteError(LPCTSTR className, LPCTSTR methodName, LPCTSTR message, ...)
{
	if (!Active() || message == 0)
		return;

    m_cs.Enter();

	static TCHAR lineStart[512];
    int lineStartSize = _stprintf_s(lineStart, 512, _T("[%4d][%6d][ERROR]<%s.%s> "), ::GetCurrentThreadId(), ::GetTickCount() - startTick, className, methodName);
    WriteBuffer(lineStart, lineStartSize);

    va_list args;
    va_start( args, message );

    int characters = _vsctprintf(message, args);
    if (characters != -1) {
		DynamicArray<TCHAR> buffer(characters + 5);
        if(-1 != _vstprintf_s(buffer, characters + 5, message, args)) {
            buffer[characters] = _T('\r');
            buffer[characters + 1] = _T('\n');
            WriteBuffer(buffer, characters + 2);
        }
    }

    m_cs.Leave();
}

bool DriverLog::CanWrite(int infoLevel) const
{
    return (infoLevel <= m_infoLevel || m_infoLevel == -1) && infoLevel != 0 && Active();
}

void DriverLog::WriteInfo(int infoLevel, LPCTSTR message, ...)
{
	if (!CanWrite(infoLevel) || message == 0)
		return;

    m_cs.Enter();
    
    static TCHAR lineStart[32];
    int lineStartSize = _stprintf_s(lineStart, 32, _T("[%4d][%6d] "), ::GetCurrentThreadId(), ::GetTickCount() - startTick);
    if (lineStartSize != -1)
        WriteBuffer(lineStart, lineStartSize);

    va_list args;
    va_start( args, message );

    int characters = _vsctprintf(message, args);
    if (characters != -1) 
	{
		DynamicArray<TCHAR> buffer(characters + 5);
        if(-1 != _vstprintf_s(buffer, characters + 5, message, args)) {
            buffer[characters] = _T('\r');
            buffer[characters + 1] = _T('\n');
            WriteBuffer(buffer, characters + 2);
        }
    }
    
    m_cs.Leave();
}
