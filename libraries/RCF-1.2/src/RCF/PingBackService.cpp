
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/PingBackService.hpp>

#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>

namespace RCF {

    PingBackService::PingBackService() : 
        mStopFlag(RCF_DEFAULT_INIT)
    {
    }

    void PingBackService::onServiceAdded(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);

        mStopFlag = false;

        WriteLock writeLock(getTaskEntriesMutex());
        getTaskEntries().clear();
        getTaskEntries().push_back( TaskEntry(
            boost::bind(&PingBackService::cycle, this, _1, _2),
            boost::bind(&PingBackService::stop, this),
            "RCF Pingback"));
    }

    void PingBackService::onServiceRemoved(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);
    }
    void PingBackService::stop()
    {
        mStopFlag = true;
    }

    bool PingBackService::cycle(
        int timeoutMs,
        const volatile bool &stopFlag)
    {
        mTimerHeap.rebase();

        PingBackTimerEntry entry;

        while (     !stopFlag 
                &&  !mStopFlag 
                &&  mTimerHeap.getExpiredEntry(entry))
        {
            RcfSessionPtr rcfSessionPtr( entry.second.lock() );
            if (rcfSessionPtr)
            {
                Lock lock(rcfSessionPtr->mIoStateMutex);
                if (mTimerHeap.compareTop(entry))
                {
                    mTimerHeap.remove(entry);
                    PingBackTimerEntry nextEntry;
                    rcfSessionPtr->sendPingBack(nextEntry);
                    mTimerHeap.add(nextEntry);
                }
            }
        } 

        boost::uint32_t queueTimeoutMs = RCF_MIN(
            static_cast<boost::uint32_t>(timeoutMs),
            mTimerHeap.getNextEntryTimeoutMs());

        if (!stopFlag && !mStopFlag)
        {
            Lock lock(mMutex);
            mCondition.timed_wait(lock, queueTimeoutMs);
        }            

        return stopFlag || mStopFlag;
    }

    PingBackService::Entry PingBackService::registerSession(RcfSessionPtr rcfSessionPtr)
    {
        Lock lock(mMutex);

        // First ping back is sent after 1 second, after that the requested ping 
        // back interval is used.
        boost::uint32_t nextFireMs = Platform::OS::getCurrentTimeMs() + 1000;
        
        Entry entry(nextFireMs, rcfSessionPtr);
        mTimerHeap.add(entry);
        mCondition.notify_all();
        return entry;
    }

    void PingBackService::unregisterSession(const Entry & entry)
    {
        mTimerHeap.remove(entry);
    }

}
