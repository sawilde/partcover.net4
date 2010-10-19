
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TEST_WITHSTOPSERVER_HPP
#define INCLUDE_RCF_TEST_WITHSTOPSERVER_HPP

#include <boost/bind.hpp>

#include <RCF/ThreadLibrary.hpp>

namespace RCF {

    class WithStopServer
    {
    public:
        WithStopServer() : mStop()
        {
        }

        void stopServer()
        {
            RCF::Lock lock(mMutex);
            mStop = true;
            mCondition.notify_one();
        }

        void wait()
        {
            RCF::Lock lock(mMutex);
            if (!mStop)
            {
                mCondition.wait(lock);
            }
        }

    private:

        RCF::Mutex mMutex;
        RCF::Condition mCondition;
        bool mStop;

    };

} // namespace RCF

#endif // ! INCLUDE_RCF_TEST_WITHSTOPSERVER_HPP

