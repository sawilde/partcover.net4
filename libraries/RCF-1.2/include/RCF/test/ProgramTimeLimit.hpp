
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TEST_PROGRAMTIMELIMIT_HPP
#define INCLUDE_RCF_TEST_PROGRAMTIMELIMIT_HPP

#include <iostream>
#include <boost/bind.hpp>
#include <boost/config.hpp>

#include <RCF/ThreadLibrary.hpp>
#include <RCF/util/Platform/OS/GetCurrentTime.hpp>
#include <RCF/util/CommandLine.hpp>

class ProgramTimeLimit
{
public:
    ProgramTimeLimit(unsigned int timeLimitS)
    {
        mStartTimeMs = Platform::OS::getCurrentTimeMs();
        mTimeLimitMs = timeLimitS*1000;
        mStopFlag = false;
        if (timeLimitS)
        {
            mThreadPtr.reset( new RCF::Thread( boost::bind(&ProgramTimeLimit::poll, this)));
        }
    }

    ~ProgramTimeLimit()
    {
        if (mThreadPtr)
        {
            {
                RCF::Lock lock(mStopMutex);
                mStopFlag = true;
                mStopCondition.notify_all();
            }
            mThreadPtr->join();
        }
    }

private:

    void poll()
    {
        while (true)
        {
            unsigned int pollIntervalMs = 1000;
            RCF::Lock lock(mStopMutex);
            mStopCondition.timed_wait(lock, pollIntervalMs);
            if (mStopFlag)
            {
                break;
            }
            else
            {
                unsigned int currentTimeMs = Platform::OS::getCurrentTimeMs();
                if (currentTimeMs - mStartTimeMs > mTimeLimitMs)
                {
                    std::cout 
                        << "Time limit expired (" << mTimeLimitMs/1000 << " (s) ). Killing this test." 
                        << std::endl;

#if defined(_MSC_VER) && _MSC_VER >= 1310

                    // By simulating an access violation , we will trigger the 
                    // creation  of a minidump, which will aid postmortem analysis.

                    int * pn = 0;
                    *pn = 1;

#elif defined(BOOST_WINDOWS)
                    
                    TerminateProcess(GetCurrentProcess(), 1);

#else

                    abort();

#endif
                }
            }
        }
    }

    unsigned int mStartTimeMs;
    unsigned int mTimeLimitMs;
    RCF::ThreadPtr mThreadPtr;
    bool mStopFlag;

    RCF::Mutex mStopMutex;
    RCF::Condition mStopCondition;
};

ProgramTimeLimit *gpProgramTimeLimit;

class ProgramTimeLimitCommandLineOption;
ProgramTimeLimitCommandLineOption *gpPtlClo;

#ifdef RCF_ENABLE_PROGRAM_TIME_LIMIT
static const int DefaultProgramTimeLimit = 60*5;
#else
static const int DefaultProgramTimeLimit = 0;
#endif

class ProgramTimeLimitCommandLineOption : public util::CommandLineOption<unsigned int>
{
public:

    ProgramTimeLimitCommandLineOption(unsigned int defaultTimeLimitS = DefaultProgramTimeLimit) :
        util::CommandLineOption<unsigned int>("timelimit", defaultTimeLimitS, "Set program time limit in seconds. 0 to disable.")
    {}

    void on_notify_end()
    {
        unsigned int timeLimitS = get();
        gpProgramTimeLimit = new ProgramTimeLimit(timeLimitS);
    }

};

UTIL_ON_INIT_NAMED( gpPtlClo = new ProgramTimeLimitCommandLineOption(); , PtoCloInitialize )
UTIL_ON_DEINIT_NAMED( delete gpProgramTimeLimit; gpProgramTimeLimit = NULL; , PtoCloDeinitialize )

#endif // ! INCLUDE_RCF_TEST_PROGRAMTIMELIMIT_HPP
