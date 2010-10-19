
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/ThreadLocalData.hpp>
#include <RCF/ThreadLocalCache.hpp>

#include <RCF/ByteBuffer.hpp>
#include <RCF/InitDeinit.hpp>

namespace RCF {

    class ThreadLocalData
    {
    public:
        ThreadLocalData()
        {
            clear();
        }

        ObjectCache                     mObjectCache;
        std::vector<ClientStubPtr>      mCurrentClientStubs;
        RcfSessionPtr                   mCurrentRcfSessionPtr;
        ThreadInfoPtr                   mThreadInfoPtr;
        UdpSessionStatePtr              mUdpSessionStatePtr;
        RecursionState<int, int>        mRcfSessionRecursionState;

        void clear()
        {
            mObjectCache.clear();
            mCurrentClientStubs.clear();
            mCurrentRcfSessionPtr.reset();
            mThreadInfoPtr.reset();
            mUdpSessionStatePtr.reset();
            mRcfSessionRecursionState = RecursionState<int, int>();
        }
    };

    typedef ThreadSpecificPtr<ThreadLocalData>::Val ThreadLocalDataPtr;

    ThreadLocalDataPtr *pThreadLocalDataPtr = NULL;

    ThreadLocalData &getThreadLocalData()
    {
        if (NULL == pThreadLocalDataPtr->get())
        {
            pThreadLocalDataPtr->reset( new ThreadLocalData());
        }
        return *(*pThreadLocalDataPtr);
    }

    // Solaris 10 on x86 crashes when trying to delete the thread specific pointer
#if defined(sun) || defined(__sun) || defined(__sun__)

    RCF_ON_INIT_NAMED(if (!pThreadLocalDataPtr) pThreadLocalDataPtr = new ThreadLocalDataPtr; , ThreadLocalDataInit)
    //RCF_ON_DEINIT_NAMED( (*pThreadLocalDataPtr)->clear(); , ThreadLocalDataDeinit)

#else

    RCF_ON_INIT_NAMED(pThreadLocalDataPtr = new ThreadLocalDataPtr;, ThreadLocalDataInit)
    RCF_ON_DEINIT_NAMED( delete pThreadLocalDataPtr; pThreadLocalDataPtr = NULL; , ThreadLocalDataDeinit)

#endif

    // access to the various thread local entities

    ObjectCache &getThreadLocalObjectCache()
    {
        return getThreadLocalData().mObjectCache;
    }

    ClientStubPtr getCurrentClientStubPtr()
    {
        return getThreadLocalData().mCurrentClientStubs.empty() ?
            ClientStubPtr() :
            getThreadLocalData().mCurrentClientStubs.back();
    }

    void pushCurrentClientStubPtr(ClientStubPtr clientStubPtr)
    {
        //getThreadLocalData().mCurrentClientStubPtr = clientStubPtr;
        getThreadLocalData().mCurrentClientStubs.push_back(clientStubPtr);
    }

    void popCurrentClientStubPtr()
    {
        getThreadLocalData().mCurrentClientStubs.pop_back();
    }

    RcfSessionPtr getCurrentRcfSessionPtr()
    {
        return getThreadLocalData().mCurrentRcfSessionPtr;
    }

    void setCurrentRcfSessionPtr(const RcfSessionPtr &rcfSessionPtr)
    {
        getThreadLocalData().mCurrentRcfSessionPtr = rcfSessionPtr;
    }

    ThreadInfoPtr getThreadInfoPtr()
    {
        return getThreadLocalData().mThreadInfoPtr;
    }

    void setThreadInfoPtr(const ThreadInfoPtr &threadInfoPtr)
    {
        getThreadLocalData().mThreadInfoPtr = threadInfoPtr;
    }

    UdpSessionStatePtr getCurrentUdpSessionStatePtr()
    {
        return getThreadLocalData().mUdpSessionStatePtr;
    }

    void setCurrentUdpSessionStatePtr(UdpSessionStatePtr udpSessionStatePtr)
    {
        getThreadLocalData().mUdpSessionStatePtr = udpSessionStatePtr;
    }

    RcfSession & getCurrentRcfSession()
    {
        return *getCurrentRcfSessionPtr();
    }

    RecursionState<int, int> & getCurrentRcfSessionRecursionState()
    {
        return getThreadLocalData().mRcfSessionRecursionState;
    }

} // namespace RCF
