#pragma once

class DriverLog
{
    DWORD startTick;
    CCriticalSection m_cs;
    int m_infoLevel;

    DriverLog(void);
    ~DriverLog(void);
public:
    static DriverLog& get();

    void Initialize(const String& fileName);
    bool Active() const { return log_fp != INVALID_HANDLE_VALUE; }
    void Deinitialize();

    void WriteLine(LPCTSTR message, ...);
    void WriteError(LPCTSTR className, LPCTSTR methodName, LPCTSTR message, ...);

    void SetInfoLevel(int infoLevel) {m_infoLevel = infoLevel;}

    void WriteInfo(int infoLevel, LPCTSTR message, ...);

    bool CanWrite(int infoLevel) const;

private:
    void WriteBuffer(LPCTSTR message, DWORD length);
    

    HANDLE log_fp;
};

#define LOGERROR0(instance, method, message)                    DriverLog::get().WriteError(_T(instance), _T(method), _T(message))
#define LOGERROR1(instance, method, message, arg0)              DriverLog::get().WriteError(_T(instance), _T(method), _T(message), arg0)
#define LOGERROR2(instance, method, message, arg0, arg1)        DriverLog::get().WriteError(_T(instance), _T(method), _T(message), arg0, arg1)
#define LOGERROR3(instance, method, message, arg0, arg1, arg2)  DriverLog::get().WriteError(_T(instance), _T(method), _T(message), arg0, arg1, arg2)
#define LOGERROR(instance, method, message)                     LOGERROR0(instance, method, message)

#define LOGINFO(level, message)                            DriverLog::get().WriteInfo(level, _T(message))
#define LOGINFO1(level, message, arg0)                     DriverLog::get().WriteInfo(level, _T(message), arg0)
#define LOGINFO2(level, message, arg0, arg1)               DriverLog::get().WriteInfo(level, _T(message), arg0, arg1)
#define LOGINFO3(level, message, arg0, arg1, arg2)         DriverLog::get().WriteInfo(level, _T(message), arg0, arg1, arg2)
#define LOGINFO4(level, message, arg0, arg1, arg2, arg3)   DriverLog::get().WriteInfo(level, _T(message), arg0, arg1, arg2, arg3)
#define LOGINFO5(level, message, arg0, arg1, arg2, arg3, arg4)   DriverLog::get().WriteInfo(level, _T(message), arg0, arg1, arg2, arg3, arg4)
#define LOGINFO6(level, message, arg0, arg1, arg2, arg3, arg4, arg5)   DriverLog::get().WriteInfo(level, _T(message), arg0, arg1, arg2, arg3, arg4, arg5)
#define LOGINFO7(level, message, arg0, arg1, arg2, arg3, arg4, arg5, arg6)   DriverLog::get().WriteInfo(level, _T(message), arg0, arg1, arg2, arg3, arg4, arg5, arg6)


#define METHOD_INNER         4
#define INSTRUMENT_METHOD    2
#define PROFILER_CALL_METHOD 2
#define DUMP_METHOD          1
#define DUMP_RESULTS         1

#define SKIP_BY_RULES        4
#define SKIP_BY_STATE        4