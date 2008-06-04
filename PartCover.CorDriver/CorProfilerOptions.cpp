#include "StdAfx.h"
#include "defines.h"
#include "corprofileroptions.h"
#include "driverlog.h"
#include "Environment.h"

CorProfilerOptions::CorProfilerOptions(void)
{
    m_logLevel.set(0);
}

CorProfilerOptions::~CorProfilerOptions(void)
{
    if (m_logFile.is_set()) Environment::FreeStringResource(m_logFile);
}

void CorProfilerOptions::InitializeFromEnvironment() {
	DWORD dwSize;
    LPCTSTR logFile = Environment::GetEnvironmentStringOption(OPTION_LOGFILE, &dwSize);
    if ( logFile != 0 ) {
        LPCTSTR verboseLevel = Environment::GetEnvironmentStringOption(OPTION_VERBOSE, &dwSize);
        if (verboseLevel != 0) {
            int level;
            if(1 == sscanf_s(verboseLevel, "%d", &level) && level > 0) {
                m_logLevel.set(level);
                m_logFile.set(logFile);
                DriverLog::get().SetInfoLevel(level);
                DriverLog::get().Initialize(m_logFile);
            } else 
                Environment::FreeStringResource(logFile);
        } else 
            Environment::FreeStringResource(logFile);
        Environment::FreeStringResource(verboseLevel);
    }
}

void CorProfilerOptions::DumpOptions() {
    DriverLog& log = DriverLog::get();
    log.WriteLine(_T("Options dump:"));
    log.WriteLine("  VerboseLevel: %d", m_logLevel.get());
    if (m_logFile.is_set())
        log.WriteLine(_T("  Log file: %s"), m_logFile.get());
}
