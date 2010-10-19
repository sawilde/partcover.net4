
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_SERVERTASK_HPP
#define INCLUDE_RCF_SERVERTASK_HPP

#include <RCF/Export.hpp>
#include <RCF/ThreadManager.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    class RCF_EXPORT TaskEntry
    {
    public:
        TaskEntry(
            Task                    task,
            StopFunctor             stopFunctor,
            const std::string &     threadName = "",
            ThreadManagerPtr        threadManagerPtr = ThreadManagerPtr());

        I_ThreadManager &   
                getThreadManager();

        ThreadManagerPtr    
                getThreadManagerPtr();

        void    setThreadManagerPtr(ThreadManagerPtr threadManagerPtr);
        Task    getTask();
        void    start(const volatile bool &stopFlag);
        void    stop(bool wait = true);

    private:
        Task                mTask;
        StopFunctor         mStopFunctor;
        ThreadManagerPtr    mThreadManagerPtr;
    };

    typedef std::vector<TaskEntry> TaskEntries;

} // namespace RCF

#endif // ! INCLUDE_RCF_SERVERTASK_HPP
