
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/SubscriptionService.hpp>

#include <boost/bind.hpp>

#include <typeinfo>

#include <RCF/ClientTransport.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ServerInterfaces.hpp>

namespace RCF {

    bool Subscription::isConnected()
    {
        return
            mClientTransportAutoPtr.get() &&
            mClientTransportAutoPtr->isConnected();
    }

    unsigned int Subscription::getPingTimestamp()
    {
        RcfSessionPtr rcfSessionPtr( mRcfSessionWeakPtr.lock() );
        if (rcfSessionPtr)
        {
            return rcfSessionPtr->getPingTimestamp();
        }
        return 0;
    }

    void Subscription::close()
    {
        {
            RcfSessionPtr rcfSessionPtr(mRcfSessionWeakPtr.lock());
            if (rcfSessionPtr)
            {
                rcfSessionPtr->setOnDestroyCallback(
                    RcfSession::OnDestroyCallback());
            }
        }
        mRcfSessionWeakPtr.reset();
        mClientTransportAutoPtr.reset();
    }

    RcfSessionPtr Subscription::getRcfSessionPtr()
    {
        return mRcfSessionWeakPtr.lock();
    }

    SubscriptionService::SubscriptionService() :
        mpServer(RCF_DEFAULT_INIT),
        mSubscriptionsMutex(WriterPriority)
    {}

    void vc6_boost_bind_helper_1(
        SubscriptionService::OnDisconnect onDisconnect, 
        RcfSession & rcfSession)
    {
        onDisconnect(rcfSession);
    }

    SubscriptionPtr SubscriptionService::onRequestSubscriptionCompleted(
        boost::int32_t                      ret,
        SubscriptionId                      whichSubscription,
        RcfClient<I_RequestSubscription> &  client,
        RcfClientPtr                        rcfClientPtr,
        OnDisconnect                        onDisconnect)
    {
        bool ok = (ret == RcfError_Ok);
        if (ok)
        {
            I_ServerTransportEx &serverTransportEx =
                dynamic_cast<I_ServerTransportEx &>(*mServerTransportPtr);

            SessionPtr sessionPtr = serverTransportEx.createServerSession(
                client.getClientStub().releaseTransport(),
                StubEntryPtr(new StubEntry(rcfClientPtr)));

            RCF_ASSERT( sessionPtr );

            RcfSessionPtr rcfSessionPtr = 
                boost::static_pointer_cast<RcfSession>(sessionPtr);

            rcfSessionPtr->setUserData(client.getClientStub().getUserData());

            if (onDisconnect)
            {

#if defined(_MSC_VER) && _MSC_VER < 1310

                rcfSessionPtr->setOnDestroyCallback(
                    boost::bind(vc6_boost_bind_helper_1, onDisconnect, _1));

#else

                rcfSessionPtr->setOnDestroyCallback(
                    onDisconnect);

#endif

            }          
            ClientTransportAutoPtr apClientTransport(
                serverTransportEx.createClientTransport(sessionPtr) );

            SubscriptionPtr subscriptionPtr(
                new Subscription(apClientTransport, rcfSessionPtr));

            return subscriptionPtr;                
        }
        return SubscriptionPtr();
    }

    SubscriptionPtr SubscriptionService::beginSubscribeNamed(
        SubscriptionId      whichSubscription,
        RcfClientPtr        rcfClientPtr,
        ClientStub &        clientStub,
        OnDisconnect        onDisconnect,
        const std::string & publisherName)
    {
        RcfClient<I_RequestSubscription> client(clientStub);
        client.getClientStub().setTransport(clientStub.releaseTransport());        
        boost::int32_t ret = client.requestSubscription(Twoway, publisherName);

        SubscriptionPtr subscriptionPtr = onRequestSubscriptionCompleted(
            ret,
            whichSubscription,
            client,
            rcfClientPtr,
            onDisconnect);

        if (subscriptionPtr)
        {
            WriteLock lock(mSubscriptionsMutex);
            mSubscriptions[ whichSubscription ] = subscriptionPtr;                
        }
        return subscriptionPtr;
    }

    void SubscriptionService::beginSubscribeNamedCb(
        Subscription::AsyncClientPtr    clientPtr,
        Future<boost::int32_t>          fRet,
        SubscriptionId                  whichSubscription,
        RcfClientPtr                    rcfClientPtr,
        OnDisconnect                    onDisconnect,
        boost::function2<void, SubscriptionPtr, ExceptionPtr> onCompletion)
    {
        SubscriptionPtr subscriptionPtr;

        ExceptionPtr exceptionPtr(
            clientPtr->getClientStub().getAsyncException().release());

        if (!exceptionPtr)
        {
            boost::int32_t ret = fRet;

            subscriptionPtr = onRequestSubscriptionCompleted(
                ret,
                whichSubscription,
                *clientPtr,
                rcfClientPtr,
                onDisconnect);

            if (subscriptionPtr)
            {
                WriteLock lock(mSubscriptionsMutex);
                mSubscriptions[ whichSubscription ] = subscriptionPtr;
            }
        }

        onCompletion(subscriptionPtr, exceptionPtr);
    }

    void SubscriptionService::beginSubscribeNamed(
        SubscriptionId                  whichSubscription,
        RcfClientPtr                    rcfClientPtr,
        ClientStub &                    clientStub,
        OnDisconnect                    onDisconnect,
        const std::string &             publisherName,
        boost::function2<void, SubscriptionPtr, ExceptionPtr> onCompletion)
    {

        Subscription::AsyncClientPtr asyncClientPtr( 
            new Subscription::AsyncClient(clientStub));

        asyncClientPtr->getClientStub().setTransport(
            clientStub.releaseTransport());

        asyncClientPtr->getClientStub().setAsyncDispatcher(*mpServer);
      
        Future<boost::int32_t> ret;
        ret = asyncClientPtr->requestSubscription(

            AsyncTwoway( boost::bind( 
                &SubscriptionService::beginSubscribeNamedCb, 
                this,
                asyncClientPtr,
                ret,
                whichSubscription, 
                rcfClientPtr,
                onDisconnect,
                onCompletion)),

            publisherName);
    }

    SubscriptionPtr SubscriptionService::beginSubscribeNamed(
        SubscriptionId          whichSubscription,
        RcfClientPtr            rcfClientPtr,
        const I_Endpoint &      publisherEndpoint,
        OnDisconnect            onDisconnect,
        const std::string &     publisherName)
    {
        ClientStub clientStub("");
        clientStub.setEndpoint(publisherEndpoint);
        clientStub.instantiateTransport();

        return
            beginSubscribeNamed(
                whichSubscription,
                rcfClientPtr,
                clientStub,
                onDisconnect,
                publisherName);
    }

    SubscriptionPtr SubscriptionService::beginSubscribeNamed(
        SubscriptionId          whichSubscription,
        RcfClientPtr            rcfClientPtr,
        ClientTransportAutoPtr  clientTransportAutoPtr,
        OnDisconnect            onDisconnect,
        const std::string &     publisherName)
    {
        ClientStub clientStub("");
        clientStub.setTransport(clientTransportAutoPtr);

        return
            beginSubscribeNamed(
                whichSubscription,
                rcfClientPtr,
                clientStub,
                onDisconnect,
                publisherName);

    }

    bool SubscriptionService::endSubscribeNamed(
        SubscriptionId whichSubscription)
    {
        SubscriptionPtr subscriptionPtr;
        {
            WriteLock lock(mSubscriptionsMutex);
            subscriptionPtr = mSubscriptions[ whichSubscription ];
            mSubscriptions.erase(whichSubscription);
        }
        if (subscriptionPtr)
        {
            RcfSessionPtr rcfSessionPtr =
                subscriptionPtr->mRcfSessionWeakPtr.lock();

            if (rcfSessionPtr)
            {
                // When this function returns, the caller is allowed to delete
                // the object that this subscription refers to. Hence, at this
                // point, we have to block any current published call that may 
                // be in execution, or else wait for it to complete.

                Lock lock(rcfSessionPtr->mStopCallInProgressMutex);
                rcfSessionPtr->mStopCallInProgress = true;

                // Remove subscription binding.
                rcfSessionPtr->setDefaultStubEntryPtr(StubEntryPtr());

                // Clear the destroy callback.
                // TODO: how do we know that we're not clearing someone else's callback?
                rcfSessionPtr->setOnDestroyCallback(
                    RcfSession::OnDestroyCallback());
            }
        }
        return true;
    }

    SubscriptionPtr SubscriptionService::getSubscriptionPtr(
        SubscriptionId whichSubscription)
    {
        ReadLock lock(mSubscriptionsMutex);
        Subscriptions::iterator iter = mSubscriptions.find(whichSubscription);
        return iter != mSubscriptions.end() ? 
            iter->second : 
            SubscriptionPtr();
    }

    void SubscriptionService::onServiceAdded(RcfServer &server)
    {
        mpServer = &server;
        mServerTransportPtr = server.getServerTransportPtr();
    }

    void SubscriptionService::onServiceRemoved(RcfServer &)
    {}

    void SubscriptionService::onServerStop(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);
        WriteLock writeLock(mSubscriptionsMutex);

        for (Subscriptions::iterator iter = mSubscriptions.begin();
            iter != mSubscriptions.end();
            ++iter)
        {
            SubscriptionPtr subscriptionPtr = iter->second;
            subscriptionPtr->close();
        }

        mSubscriptions.clear();
    }

    void SubscriptionService::onServerClose(RcfServer &)
    {}

    Subscription::Subscription(
        ClientTransportAutoPtr clientTransportAutoPtr,
        RcfSessionWeakPtr rcfSessionWeakPtr) :
            mClientTransportAutoPtr(clientTransportAutoPtr),
            mRcfSessionWeakPtr(rcfSessionWeakPtr)
    {}
   
} // namespace RCF
