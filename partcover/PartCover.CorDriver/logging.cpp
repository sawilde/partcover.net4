#include "StdAfx.h"
#include "interface.h"
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
	if (file.size() == 0)
		return;
	
	log_fp = CreateFile(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
	if (log_fp != INVALID_HANDLE_VALUE) {
#ifdef _UNICODE
		DWORD bytesWritten;
		const char ECS2_Signature[] = { (char)0xFF, (char)0xFE };
		WriteFile(log_fp, ECS2_Signature, ARRAYSIZE(ECS2_Signature), &bytesWritten, NULL);
#endif
	}
	
}

void DriverLog::SetPipe(IIntercommunication* pipe)
{
	m_pipe = pipe;
}

void DriverLog::Deinitialize()
{
    if (!Active()) 
		return;

    CloseHandle(log_fp);

    log_fp = INVALID_HANDLE_VALUE;
	m_pipe = 0;
}

void DriverLog::WriteFileLog(LPCTSTR message, DWORD length)
{
	if (log_fp == INVALID_HANDLE_VALUE || message == 0)
		return;

	DWORD messageWritten;
    WriteFile(log_fp, message, sizeof(TCHAR)*length, &messageWritten, NULL);
}

void DriverLog::WritePipeLog(LPCTSTR message, DWORD length)
{
	if (m_pipe == 0 || message == 0 || length == 0)
		return;

	m_pipe->LogMessage(::GetCurrentThreadId(), ::GetTickCount() - startTick, String(message, length));
}

void DriverLog::WriteLine(LPCTSTR message, ...)
{
	if (!Active() || message == 0)
		return;

    m_cs.Enter();
    
    static TCHAR lineStart[512];
    int lineStartSize = _stprintf_s(lineStart, 512, _T("[%4d][%6d]"), ::GetCurrentThreadId(), ::GetTickCount() - startTick);
    WriteFileLog(lineStart, lineStartSize);

    va_list args;
    va_start( args, message );

    int characters = _vsctprintf(message, args);
    if (characters != -1) {
		DynamicArray<TCHAR> buffer(characters + 5);
        if(-1 != _vstprintf_s(buffer, characters + 5, message, args)) {
			WritePipeLog(buffer, characters);
            buffer[characters] = _T('\r');
            buffer[characters + 1] = _T('\n');
            WriteFileLog(buffer, characters + 2);
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
    WriteFileLog(lineStart, lineStartSize);

    va_list args;
    va_start( args, message );

    int characters = _vsctprintf(message, args);
    if (characters != -1) {
		DynamicArray<TCHAR> buffer(characters + 5);
        if(-1 != _vstprintf_s(buffer, characters + 5, message, args)) {
			WritePipeLog(buffer, characters);
            buffer[characters] = _T('\r');
            buffer[characters + 1] = _T('\n');
            WriteFileLog(buffer, characters + 2);
        }
    }

    m_cs.Leave();
}

bool DriverLog::CanWrite(int infoLevel) const
{
    return infoLevel != 0 && (infoLevel & m_infoLevel) == infoLevel && Active();
}

void DriverLog::WriteInfo(int infoLevel, LPCTSTR message, ...)
{
	if (!CanWrite(infoLevel) || message == 0)
		return;

    m_cs.Enter();
    
    static TCHAR lineStart[32];
    int lineStartSize = _stprintf_s(lineStart, 32, _T("[%4d][%6d] "), ::GetCurrentThreadId(), ::GetTickCount() - startTick);
    if (lineStartSize != -1)
        WriteFileLog(lineStart, lineStartSize);

    va_list args;
    va_start( args, message );

    int characters = _vsctprintf(message, args);
    if (characters != -1) 
	{
		DynamicArray<TCHAR> buffer(characters + 5);
        if(-1 != _vstprintf_s(buffer, characters + 5, message, args)) {
			WritePipeLog(buffer, characters);
            buffer[characters] = _T('\r');
            buffer[characters + 1] = _T('\n');
            WriteFileLog(buffer, characters + 2);
        }
    }
    
    m_cs.Leave();
}
