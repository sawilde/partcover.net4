
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/ThreadManager.hpp>

#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    // I_ThreadManager

    void I_ThreadManager::setThreadName(const std::string &threadName)
    {
        Lock lock(mInitDeinitMutex);
        mThreadName = threadName;
    }

    std::string I_ThreadManager::getThreadName()
    {
        Lock lock(mInitDeinitMutex);
        return mThreadName;
    }

#if defined(BOOST_WINDOWS) && !defined (__MINGW32__)

    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType; // must be 0x1000
        LPCSTR szName; // pointer to name (in user addr space)
        DWORD dwThreadID; // thread ID (-1=caller thread)
        DWORD dwFlags; // reserved for future use, must be zero
    } THREADNAME_INFO;

    // 32 character limit on szThreadName apparently, or it gets truncated.
    void setWin32ThreadName(DWORD dwThreadID, LPCSTR szThreadName)
    {
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = szThreadName;
        info.dwThreadID = dwThreadID;
        info.dwFlags = 0;

        __try
        {
            RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (ULONG_PTR*)&info );
        }
        __except(EXCEPTION_CONTINUE_EXECUTION)
        {
        }
    }

    void I_ThreadManager::setMyThreadName()
    {
        std::string threadName = getThreadName();
        if (!threadName.empty())
        {
            setWin32ThreadName( DWORD(-1), threadName.c_str());
        }
    }

#elif defined(BOOST_WINDOWS)

    void setWin32ThreadName(DWORD dwThreadID, LPCSTR szThreadName)
    {
    }

    void I_ThreadManager::setMyThreadName()
    {
    }

#else

    void I_ThreadManager::setMyThreadName()
    {
    }

#endif

    

    void I_ThreadManager::onInit()
    {
        std::vector<ThreadInitFunctor> initFunctors;
        {
            Lock lock(mInitDeinitMutex);
            std::copy(
                mThreadInitFunctors.begin(), 
                mThreadInitFunctors.end(), 
                std::back_inserter(initFunctors));
        }

        std::for_each(
            initFunctors.begin(), 
            initFunctors.end(), 
            boost::bind(&ThreadInitFunctor::operator(), _1));
    }

    void I_ThreadManager::onDeinit()
    {
        std::vector<ThreadDeinitFunctor> deinitFunctors;
        {
            Lock lock(mInitDeinitMutex);
            std::copy(
                mThreadDeinitFunctors.begin(), 
                mThreadDeinitFunctors.end(), 
                std::back_inserter(deinitFunctors));
        }

        std::for_each(
            deinitFunctors.begin(), 
            deinitFunctors.end(), 
            boost::bind(&ThreadDeinitFunctor::operator(), _1));
    }

    void I_ThreadManager::addThreadInitFunctor(ThreadInitFunctor threadInitFunctor)
    {
        Lock lock(mInitDeinitMutex);
        mThreadInitFunctors.push_back(threadInitFunctor);
    }

    void I_ThreadManager::addThreadDeinitFunctor(ThreadDeinitFunctor threadDeinitFunctor)
    {
        Lock lock(mInitDeinitMutex);
        mThreadDeinitFunctors.push_back(threadDeinitFunctor);
    }

    // FixedThreadPool

    FixedThreadPool::FixedThreadPool(std::size_t threadCount) :
        mStarted(RCF_DEFAULT_INIT),
        mThreadCount(threadCount)
        
    {
        RCF1_TRACE("");
    }

    void FixedThreadPool::repeatTask(
        const ThreadInfoPtr &threadInfoPtr,
        const Task &task,
        int timeoutMs,
        const volatile bool &stopFlag)
    {

        RCF1_TRACE("");
        setThreadInfoPtr(threadInfoPtr);

        setMyThreadName();

        onInit();

        bool taskFlag = false;
        while (!stopFlag && !taskFlag && !threadInfoPtr->mStopFlag)
        {
            try
            {
                while (!stopFlag && !taskFlag && !threadInfoPtr->mStopFlag)
                {
                    taskFlag = task(timeoutMs, stopFlag, false);
                }
            }
            catch(const std::exception &e)
            {
                RCF1_TRACE("worker thread: exception")(e);
            }
        }

        onDeinit();

        RCF1_TRACE("")(stopFlag);
    }

    // not synchronized
    void FixedThreadPool::start(const volatile bool &stopFlag)
    {
        RCF1_TRACE("");
        if (!mStarted)
        {
            RCF_ASSERT(mThreads.empty())(mThreads.size());
            mThreads.clear();
            mThreads.resize(mThreadCount);
            for (std::size_t i=0; i<mThreads.size(); ++i)
            {
                ThreadInfoPtr threadInfoPtr( new ThreadInfo( shared_from_this()));

                ThreadPtr threadPtr(new Thread(
                    boost::bind(
                    &FixedThreadPool::repeatTask,
                    this,
                    threadInfoPtr,
                    mTask,
                    1000,
                    boost::ref(stopFlag))));

                mThreads[i] = threadPtr;
            }
            mStarted = true;
        }
    }

    // not synchronized
    void FixedThreadPool::stop(bool wait)
    {
        RCF1_TRACE("")(wait);
        if (mStarted)
        {
            for (std::size_t i=0; i<mThreads.size(); ++i)
            {
                if (mStopFunctor)
                {
                    mStopFunctor();
                }
                if (wait)
                {
                    mThreads[i]->join();
                    mThreads[i].reset();
                }
            }
            mThreads.clear();
            if (wait)
            {
                mStarted = false;
            }
        }
    }

    void FixedThreadPool::setTask(const Task &task)
    {
        RCF_ASSERT(!mStarted);
        mTask = task;
    }

    void FixedThreadPool::setStopFunctor(const StopFunctor &stopFunctor)
    {
        RCF_ASSERT(!mStarted);
        mStopFunctor = stopFunctor;
    }

    // DynamicThreadPool

    DynamicThreadPool::DynamicThreadPool(
        std::size_t threadTargetCount,
        std::size_t threadMaxCount,
        boost::uint32_t threadIdleTimeoutMs,
        bool reserveLastThread) :
            mStarted(RCF_DEFAULT_INIT),
            mThreadTargetCount(threadTargetCount),
            mThreadMaxCount(threadMaxCount),
            mReserveLastThread(reserveLastThread),
            mThreadIdleTimeoutMs(threadIdleTimeoutMs),
            mpUserStopFlag(RCF_DEFAULT_INIT),
            mBusyCount(RCF_DEFAULT_INIT)
            
    {
        RCF1_TRACE("");
        RCF_ASSERT(
            0 < mThreadTargetCount && mThreadTargetCount <= mThreadMaxCount)
            (mThreadTargetCount)(mThreadMaxCount);
    }

    // Not synchronized - caller must hold lock on mThreadsMutex.
    bool DynamicThreadPool::launchThread(const volatile bool &userStopFlag)
    {
        RCF1_TRACE("");

        RCF_ASSERT(
            mThreads.size() <= mThreadMaxCount)
            (mThreads.size())(mThreadMaxCount);

        if (mThreads.size() == mThreadMaxCount)
        {
            // We've hit the max thread limit.
            return false;
        }
        else
        {
            ThreadInfoPtr threadInfoPtr( new ThreadInfo(shared_from_this()));

            ThreadPtr threadPtr( new Thread(
                boost::bind(
                    &DynamicThreadPool::repeatTask,
                    this,
                    threadInfoPtr,
                    mTask,
                    1000,
                    boost::ref(userStopFlag))));

            RCF_ASSERT(mThreads.find(threadInfoPtr) == mThreads.end());

            mThreads[threadInfoPtr] = threadPtr;

            return true;
        }
    }

    void DynamicThreadPool::notifyBusy()
    {
        RCF1_TRACE("");

        if (!getThreadInfoPtr()->mBusy)
        {
            getThreadInfoPtr()->mBusy = true;

            Lock lock(mThreadsMutex);

            ++mBusyCount;
            RCF_ASSERT(0 <= mBusyCount && mBusyCount <= mThreads.size());

            if (! (*mpUserStopFlag))
            {
                if (mBusyCount == mThreads.size())
                {
                    bool launchedOk = launchThread(*mpUserStopFlag);                    
                    if (!launchedOk && mReserveLastThread)
                    {
                        RCF_THROW(Exception(_RcfError_AllThreadsBusy()));
                    }
                }
            }
        }
    }

    void DynamicThreadPool::notifyReady()
    {
        RCF1_TRACE("");

        ThreadInfoPtr threadInfoPtr = getThreadInfoPtr();

        if (threadInfoPtr->mBusy)
        {
            threadInfoPtr->mBusy = false;

            Lock lock(mThreadsMutex);
            
            --mBusyCount;            
            
            RCF_ASSERT(0 <= mBusyCount && mBusyCount <= mThreads.size());
        }

        boost::uint32_t nowMs = Platform::OS::getCurrentTimeMs();
        if (nowMs - threadInfoPtr->mTouchMs > mThreadIdleTimeoutMs)
        {
            // If we have more than our target count of threads running, and
            // if at least two of the threads are not busy, then let this thread
            // exit.

            Lock lock(mThreadsMutex);

            if (    mThreads.size() > mThreadTargetCount 
                &&  mBusyCount < mThreads.size() - 1)
            {                
                threadInfoPtr->mStopFlag = true; 

                RCF_ASSERT( mThreads.find(threadInfoPtr) != mThreads.end() );
                mThreads.erase( mThreads.find(threadInfoPtr) );
            }
        }
    }

    void DynamicThreadPool::repeatTask(
        const RCF::ThreadInfoPtr &threadInfoPtr,
        const Task &task,
        int timeoutMs,
        const volatile bool &stopFlag)
    {
        RCF1_TRACE("");
        setThreadInfoPtr(threadInfoPtr);

        setMyThreadName();

        onInit();

        bool taskFlag = false;
        while (!stopFlag && !taskFlag && !threadInfoPtr->mStopFlag)
        {
            try
            {
                while (!stopFlag && !taskFlag && !threadInfoPtr->mStopFlag)
                {
                    taskFlag = task(timeoutMs, stopFlag, false);
                    notifyReady();
                }
            }
            catch(const std::exception &e)
            {
                RCF1_TRACE("worker thread: exception")(e);
            }
        }

        onDeinit();

        {
            Lock lock(mThreadsMutex);
            ThreadMap::iterator iter = mThreads.find(threadInfoPtr);
            if (iter != mThreads.end())
            {
                mThreads.erase(iter);
            }            
        }

        RCF_TRACE("")(stopFlag);
    }

    // not synchronized
    void DynamicThreadPool::start(const volatile bool &stopFlag)
    {
        RCF1_TRACE("");

        if (!mStarted)
        {
            RCF_ASSERT(mThreads.empty())(mThreads.size());
            mThreads.clear();
            mBusyCount = 0;
            mpUserStopFlag = &stopFlag;

            for (std::size_t i=0; i<mThreadTargetCount; ++i)
            {
                bool ok = launchThread(stopFlag);
                RCF_ASSERT(ok);
            }

            mStarted = true;
        }
    }

    void DynamicThreadPool::stop(bool wait)
    {
        RCF1_TRACE("");

        ThreadMap threads;
        {
            Lock lock(mThreadsMutex);
            threads = mThreads;
            bool stopFlag = *mpUserStopFlag;
            RCF_ASSERT(stopFlag);
        }

        if (mStarted)
        {
            ThreadMap::iterator iter;
            for (
                iter = threads.begin(); 
                iter != threads.end(); 
                ++iter)
            {
                if (mStopFunctor)
                {
                    mStopFunctor();
                }
                if (wait)
                {
                    iter->second->join();
                }
            }

            if (wait)
            {
                RCF_ASSERT( mThreads.empty() );
                mThreads.clear();
                mStarted = false;
            }
        }
    }

    void DynamicThreadPool::setTask(const Task &task)
    {
        RCF_ASSERT(!mStarted);
        mTask = task;
    }

    void DynamicThreadPool::setStopFunctor(const StopFunctor &stopFunctor)
    {
        RCF_ASSERT(!mStarted);
        mStopFunctor = stopFunctor;
    }

    ThreadTouchGuard::ThreadTouchGuard() : 
        mThreadInfoPtr(getThreadInfoPtr())
    {
        if (mThreadInfoPtr)
        {
            mThreadInfoPtr->touch();
        }
    }

    ThreadTouchGuard::~ThreadTouchGuard()
    {
        if (mThreadInfoPtr)
        {
            mThreadInfoPtr->touch();
        }
    }

} // namespace RCF
