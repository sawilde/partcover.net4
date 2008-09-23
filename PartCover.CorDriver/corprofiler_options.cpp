#include "StdAfx.h"
#include "defines.h"
#include "corprofiler_options.h"
#include "logging.h"
#include "environment.h"

CorProfilerOptions::CorProfilerOptions(void)
{
}

CorProfilerOptions::~CorProfilerOptions(void)
{
}

void CorProfilerOptions::InitializeFromEnvironment() {
	DWORD dwSize;
    LPCTSTR logFile = Environment::GetEnvironmentStringOption(OPTION_LOGFILE, &dwSize);
    if ( logFile == 0 ) 
		return;
	
    LPCTSTR verboseLevel = Environment::GetEnvironmentStringOption(OPTION_VERBOSE, &dwSize);
    if (verboseLevel != 0) {
        int level;
        if(1 == _stscanf_s(verboseLevel, _T("%d"), &level) && level > 0) 
		{
            m_logLevel.set(level);
            m_logFile.set(logFile);
            DriverLog::get().SetInfoLevel(level);
            DriverLog::get().Initialize(m_logFile);
        }
	}

    Environment::FreeStringResource(logFile);
    Environment::FreeStringResource(verboseLevel);
}

void CorProfilerOptions::DumpOptions() {
    DriverLog& log = DriverLog::get();
    log.WriteLine(_T("Options dump:"));
    log.WriteLine(_T("  VerboseLevel: %d"), m_logLevel.get());
    if (m_logFile.is_set())
		log.WriteLine(_T("  Log file: %s"), m_logFile.get().c_str());
}
