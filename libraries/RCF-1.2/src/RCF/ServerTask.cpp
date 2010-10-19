
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/ServerTask.hpp>

namespace RCF {

    TaskEntry::TaskEntry(
        Task                    task,
        StopFunctor             stopFunctor,
        const std::string &     threadName,
        ThreadManagerPtr        threadManagerPtr) :
            mTask(task),
            mStopFunctor(stopFunctor)
    {
        if (!threadManagerPtr)
        {
            threadManagerPtr.reset(new FixedThreadPool(1));
        }
        threadManagerPtr->setThreadName(threadName);
        setThreadManagerPtr(threadManagerPtr);
    }

    I_ThreadManager & TaskEntry::getThreadManager()
    {
        return *getThreadManagerPtr();
    }

    ThreadManagerPtr TaskEntry::getThreadManagerPtr()
    {
        return mThreadManagerPtr;
    }

    void TaskEntry::setThreadManagerPtr(ThreadManagerPtr threadManagerPtr)
    {
        mThreadManagerPtr = threadManagerPtr;
        threadManagerPtr->setTask(mTask);
        threadManagerPtr->setStopFunctor(mStopFunctor);
    }

    Task TaskEntry::getTask()
    {
        return mTask;
    }

    void TaskEntry::start(const volatile bool &stopFlag)
    {
        getThreadManager().start(stopFlag);
    }

    void TaskEntry::stop(bool wait)
    {
        getThreadManager().stop(wait);
    }

} // namespace RCF
