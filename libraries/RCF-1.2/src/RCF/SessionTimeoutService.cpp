
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/SessionTimeoutService.hpp>

#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>

#include <RCF/util/Platform/OS/Sleep.hpp>

namespace RCF {

    SessionTimeoutService::SessionTimeoutService(
        boost::uint32_t sessionTimeoutMs,
        boost::uint32_t reapingIntervalMs) : 
            mSessionTimeoutMs(sessionTimeoutMs),
            mLastRunTimeMs(RCF_DEFAULT_INIT),
            mReapingIntervalMs(reapingIntervalMs),
            mStopFlag(RCF_DEFAULT_INIT),
            mpRcfServer(RCF_DEFAULT_INIT)
    {
    }

    void SessionTimeoutService::onServiceAdded(RcfServer &server)
    {
        mpRcfServer = & server;

        mStopFlag = false;

        WriteLock writeLock(getTaskEntriesMutex());
        getTaskEntries().clear();
        getTaskEntries().push_back( TaskEntry(
            boost::bind(&SessionTimeoutService::cycle, this, _1, _2),
            boost::bind(&SessionTimeoutService::stop, this),
            "RCF Session Timeout"));
    }

    void SessionTimeoutService::onServiceRemoved(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);
        mpRcfServer = NULL;
    }

    void SessionTimeoutService::stop()
    {
        mStopFlag = true;
    }

    bool SessionTimeoutService::cycle(
        int timeoutMs,
        const volatile bool &stopFlag)
    {
        RCF_UNUSED_VARIABLE(timeoutMs);

        boost::uint32_t nowMs = Platform::OS::getCurrentTimeMs();

        if (nowMs - mLastRunTimeMs < mReapingIntervalMs 
            && !stopFlag 
            && !mStopFlag)
        {
            Platform::OS::Sleep(1);
            return false;
        }

        mLastRunTimeMs = nowMs;

        mSessionsTemp.resize(0);

        mpRcfServer->enumerateSessions(std::back_inserter(mSessionsTemp));

        for (std::size_t i=0; i<mSessionsTemp.size(); ++i)
        {
            RcfSessionPtr rcfSessionPtr = mSessionsTemp[i].lock();
            if (rcfSessionPtr)
            {
                boost::uint32_t touchMs = rcfSessionPtr->getTouchTimestamp();
                boost::uint32_t nowMs = Platform::OS::getCurrentTimeMs();
                if (nowMs - touchMs > mSessionTimeoutMs)
                {
                    rcfSessionPtr->disconnect();
                }
            }
        }

        return stopFlag || mStopFlag;
    }

} //namespace RCF
