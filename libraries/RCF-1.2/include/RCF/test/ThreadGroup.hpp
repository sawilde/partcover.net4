
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TEST_THREADGROUP_HPP
#define INCLUDE_RCF_TEST_THREADGROUP_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

#include <RCF/ThreadLibrary.hpp>

typedef RCF::Thread Thread;

//#ifdef RCF_MULTI_THREADED
//typedef RCF::Thread Thread;
//#else
//#include <RCF/RcfBoostThreads/RcfBoostThreads.hpp>
//#include "../../../src/RCF/RcfBoostThreads/RcfBoostThreads.cpp"
//typedef RCF::RcfBoostThreads::boost::thread Thread;
//#endif

typedef boost::shared_ptr<Thread> ThreadPtr;
typedef std::vector<ThreadPtr> ThreadGroup;

inline void joinThreadGroup(const ThreadGroup &threadGroup)
{
    for (unsigned int i=0; i<threadGroup.size(); ++i)
    {
        threadGroup[i]->join();
    }
}


#endif // ! INCLUDE_RCF_TEST_THREADGROUP_HPP
