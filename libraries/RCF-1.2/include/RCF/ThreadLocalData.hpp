
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_THREADLOCALDATA_HPP
#define INCLUDE_RCF_THREADLOCALDATA_HPP

#include <boost/shared_ptr.hpp>

#include <RCF/ByteBuffer.hpp>
#include <RCF/Export.hpp>
#include <RCF/RecursionLimiter.hpp>

namespace RCF {

    class ObjectCache;
    class ClientStub;
    class I_Session;
    class RcfSession;
    class ThreadInfo;
    class UdpSessionState;
    class I_Future;

    typedef boost::shared_ptr<ClientStub>       ClientStubPtr;
    typedef boost::shared_ptr<I_Session>        SessionPtr;
    typedef boost::shared_ptr<RcfSession>       RcfSessionPtr;
    typedef boost::shared_ptr<ThreadInfo>       ThreadInfoPtr;
    typedef boost::shared_ptr<UdpSessionState>  UdpSessionStatePtr;
    

    RCF_EXPORT ObjectCache &        getThreadLocalObjectCache();
    RCF_EXPORT ClientStubPtr        getCurrentClientStubPtr();
    
    RCF_EXPORT void                 pushCurrentClientStubPtr(
                                        ClientStubPtr clientStubPtr);

    RCF_EXPORT void                 popCurrentClientStubPtr();

    RCF_EXPORT RcfSessionPtr        getCurrentRcfSessionPtr();

    RCF_EXPORT void                 setCurrentRcfSessionPtr(
                                        const RcfSessionPtr &rcfSessionPtr = RcfSessionPtr());

    RCF_EXPORT ThreadInfoPtr        getThreadInfoPtr();

    RCF_EXPORT void                 setThreadInfoPtr(
                                        const ThreadInfoPtr &threadInfoPtr);

    RCF_EXPORT UdpSessionStatePtr   getCurrentUdpSessionStatePtr();

    RCF_EXPORT void                 setCurrentUdpSessionStatePtr(
                                        UdpSessionStatePtr udpSessionStatePtr);

    RCF_EXPORT RcfSession &         getCurrentRcfSession();

    RecursionState<int, int> &      getCurrentRcfSessionRecursionState();

} // namespace RCF

#endif // ! INCLUDE_RCF_THREADLOCALDATA_HPP
