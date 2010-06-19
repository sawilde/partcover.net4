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
	
    LPCTSTR verboseLevel = Environment::GetEnvironmentStringOption(OPTION_VERBOSE, &dwSize);
    if (verboseLevel != 0) 
	{
        int level;
        if(1 == _stscanf_s(verboseLevel, _T("%d"), &level) && level > 0) 
		{
            m_logLevel.set(level);
            DriverLog::get().SetInfoLevel(level);
        }
	}

    LPCTSTR logFile = Environment::GetEnvironmentStringOption(OPTION_LOGFILE, &dwSize);
	if ( logFile != 0 ) {
        m_logFile.set(logFile);
	    DriverLog::get().Initialize(m_logFile);
	}

	LPCTSTR logPipe =  Environment::GetEnvironmentStringOption(OPTION_LOGPIPE, &dwSize);
	if ( logPipe != 0 ) {
		m_usePipe.set(0 == _tcsicmp(logPipe, _T("1") ));
	}

    Environment::FreeStringResource(logPipe);
    Environment::FreeStringResource(logFile);
    Environment::FreeStringResource(verboseLevel);
}

void CorProfilerOptions::DumpOptions() {
    DriverLog& log = DriverLog::get();
    log.WriteLine(_T("Options dump:"));
    log.WriteLine(_T("  VerboseLevel: %d"), m_logLevel.get());

	if (m_logFile.is_set()) 
	{
		log.WriteLine(_T("  Log file: %s"), m_logFile.get().c_str());
	}

    if (m_usePipe.is_set())
	{
		String dumpMessage = m_usePipe.get() ? _T("yes") : _T("no");
		log.WriteLine(_T("  Log pipe: %s"), dumpMessage.c_str());
	}
}
