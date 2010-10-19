
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_THREADMANAGER_HPP
#define INCLUDE_RCF_THREADMANAGER_HPP

#include <vector>

#include <boost/bind.hpp>
#include <boost/cstdint.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    typedef boost::function3<bool, int, const volatile bool &, bool>    Task;
    typedef boost::function0<void>                                      StopFunctor;

    class RCF_EXPORT I_ThreadManager
    {
    public:
        virtual ~I_ThreadManager()
        {}

        virtual void    start(const volatile bool &stopFlag) = 0;
        virtual void    stop(bool wait = true) = 0;
        virtual void    notifyBusy() {}

        typedef boost::function0<void> ThreadInitFunctor;

        void            addThreadInitFunctor(
                            ThreadInitFunctor threadInitFunctor);

        typedef boost::function0<void> ThreadDeinitFunctor;

        void            addThreadDeinitFunctor(
                            ThreadDeinitFunctor threadDeinitFunctor);

        void            setThreadName(const std::string &threadName);
        std::string     getThreadName();

    protected:
        void            onInit();
        void            onDeinit();
        void            setMyThreadName();

    private:
        friend class TaskEntry;

    private:
        virtual void    notifyReady() {}
        virtual void    setTask(const Task &task)  = 0;
        virtual void    setStopFunctor(const StopFunctor &stopFunctor) = 0;

    protected:

        Mutex                               mInitDeinitMutex;
        std::vector<ThreadInitFunctor>      mThreadInitFunctors;
        std::vector<ThreadDeinitFunctor>    mThreadDeinitFunctors;

        std::string                         mThreadName;
    };

    typedef boost::shared_ptr<I_ThreadManager> ThreadManagerPtr;

    typedef unsigned int ThreadId;

    class ThreadInfo
    {
    public:

        ThreadInfo(ThreadManagerPtr threadManagerPtr) :
            mThreadManagerPtr(threadManagerPtr),
            mBusy(RCF_DEFAULT_INIT),
            mStopFlag(RCF_DEFAULT_INIT),
            mTouchMs(RCF_DEFAULT_INIT)
        {}

        virtual ~ThreadInfo()
        {}

        void touch()
        {
            mTouchMs = Platform::OS::getCurrentTimeMs();
        }

        ThreadManagerPtr mThreadManagerPtr;
        bool mBusy;
        bool mStopFlag;
        boost::uint32_t mTouchMs;
    };

    typedef boost::shared_ptr<ThreadInfo> ThreadInfoPtr;

    // Thread manager implementations

    // FixedThreadPool

    class RCF_EXPORT FixedThreadPool :
        public I_ThreadManager,
        public boost::enable_shared_from_this<FixedThreadPool>
    {
    public:

        FixedThreadPool(std::size_t threadCount = 1);

    private:

        // start()/stop() are not synchronized
        void    start(const volatile bool &stopFlag);
        void    stop(bool wait);

        void    setTask(const Task &task);
        void    setStopFunctor(const StopFunctor &stopFunctor);

        void    repeatTask(
                    const ThreadInfoPtr &threadInfoPtr,
                    const Task &task,
                    int timeoutMs,
                    const volatile bool &stopFlag);

        Task                        mTask;
        StopFunctor                 mStopFunctor;
        bool                        mStarted;
        std::size_t                 mThreadCount;
        std::vector<ThreadPtr>      mThreads;
    };

    typedef boost::shared_ptr<FixedThreadPool> FixedThreadPoolPtr;

    // DynamicThreadPool

    class RCF_EXPORT DynamicThreadPool :
        public I_ThreadManager,
        public boost::enable_shared_from_this<DynamicThreadPool>
    {
    public:
        DynamicThreadPool(
            std::size_t threadTargetCount = 1,
            std::size_t threadMaxCount = 10,
            boost::uint32_t threadIdleTimeoutMs = 30*1000,
            bool reserveLastThread = true);

        std::size_t getThreadCount()
        {
            Lock lock(mThreadsMutex);
            return mThreads.size();
        }

    private:

        bool            launchThread(const volatile bool &userStopFlag);
        void            notifyBusy();
        void            notifyReady();
        void            repeatTask(
                            const RCF::ThreadInfoPtr &threadInfoPtr,
                            const Task &task,
                            int timeoutMs,
                            const volatile bool &stopFlag);
       
        // start()/stop() not synchronized
        void            start(const volatile bool &stopFlag);
        void            stop(bool wait);
       
        void            setTask(const Task &task);
        void            setStopFunctor(const StopFunctor &stopFunctor);
       
    private:

        bool                        mStarted;
        std::size_t                 mThreadTargetCount;
        std::size_t                 mThreadMaxCount;
        bool                        mReserveLastThread;
        boost::uint32_t             mThreadIdleTimeoutMs;

        Task                        mTask;
        StopFunctor                 mStopFunctor;

        const volatile bool *       mpUserStopFlag;

        typedef std::map<ThreadInfoPtr, ThreadPtr> ThreadMap;

        Mutex                       mThreadsMutex;
        ThreadMap                   mThreads;
        std::size_t                 mBusyCount;
    };

    typedef boost::shared_ptr<DynamicThreadPool> DynamicThreadPoolPtr;

    class ThreadTouchGuard
    {
    public:
        ThreadTouchGuard();
        ~ThreadTouchGuard();
    private:
        ThreadInfoPtr mThreadInfoPtr;
    };

#if defined(BOOST_WINDOWS)

    void setWin32ThreadName(DWORD dwThreadID, LPCSTR szThreadName);

#endif

} // namespace RCF

#endif // ! INCLUDE_RCF_THREADMANAGER_HPP
