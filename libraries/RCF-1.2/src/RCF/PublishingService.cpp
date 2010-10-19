
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/PublishingService.hpp>

#include <RCF/CurrentSession.hpp>
#include <RCF/MulticastClientTransport.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ServerInterfaces.hpp>
#include <RCF/ServerTransport.hpp>

#ifdef RCF_USE_PROTOBUF
#include <RCF/protobuf/RcfMessages.pb.h>
#endif

namespace RCF {

    PublishingService::PublishingService() :
        mPublishersMutex(WriterPriority)
    {}

    bool PublishingService::beginPublishNamed(
        const std::string &publisherName,
        RcfClientPtr rcfClientPtr)
    {
        rcfClientPtr->getClientStub().setTransport(ClientTransportAutoPtr(new MulticastClientTransport));
        rcfClientPtr->getClientStub().setDefaultCallingSemantics(Oneway);
        rcfClientPtr->getClientStub().setTargetName("");
        rcfClientPtr->getClientStub().setTargetToken( Token());
        PublisherPtr publisherPtr( new Publisher(publisherName, rcfClientPtr));
        WriteLock lock(mPublishersMutex);
        mPublishers[publisherName] = publisherPtr;
        return true;
    }

    I_RcfClient &PublishingService::publishNamed(const std::string &publisherName)
    {
        ReadLock lock(mPublishersMutex);
        if (mPublishers.find(publisherName) != mPublishers.end())
        {
            return *mPublishers[ publisherName ]->mMulticastRcfClientPtr;
        }
        RCF_THROW(Exception(_RcfError_UnknownPublisher(publisherName)))(publisherName);
    }

    bool PublishingService::endPublishNamed(const std::string &publisherName)
    {
        WriteLock lock(mPublishersMutex);
        Publishers::iterator iter = mPublishers.find(publisherName);
        if (iter != mPublishers.end())
        {
            mPublishers.erase(iter);
        }
        return true;
    }

    void PublishingService::setOnConnectCallback(
        OnConnectCallback onConnectCallback)
    {
        WriteLock lock(mPublishersMutex);
        mOnConnectCallback = onConnectCallback;
    }

    void PublishingService::setOnDisconnectCallback(
        OnDisconnectCallback onDisconnectCallback)
    {
        WriteLock lock(mPublishersMutex);
        mOnDisconnectCallback = onDisconnectCallback;
    }

    void vc6_boost_bind_helper_2(
        PublishingService::OnDisconnectCallback onDisconnect, 
        RcfSession & rcfSession,
        const std::string & subscriptionName )
    {
        onDisconnect(rcfSession, subscriptionName);
    }

    // remotely accessible
    boost::int32_t PublishingService::requestSubscription(
        const std::string &subscriptionName)
    {
        std::string publisherName = subscriptionName;
        bool found = false;
        ReadLock lock(mPublishersMutex);
        if (mPublishers.find(publisherName) != mPublishers.end())
        {
            found = true;
        }
        lock.unlock();
        if (found)
        {
            RcfSession & rcfSession = getCurrentRcfSession();

            if (mOnConnectCallback)
            {
                mOnConnectCallback(rcfSession, subscriptionName);
            }            

            I_ServerTransportEx &serverTransport =
                dynamic_cast<I_ServerTransportEx &>(
                    rcfSession.getProactor().getServerTransport());

            ClientTransportAutoPtr clientTransportAutoPtr(
                serverTransport.createClientTransport(
                    rcfSession.shared_from_this()));

            ClientTransportPtr clientTransportPtr(
                clientTransportAutoPtr.release());

            rcfSession.addOnWriteCompletedCallback(
                boost::bind(
                    &PublishingService::addSubscriberTransport,
                    this,
                    _1,
                    publisherName,
                    clientTransportPtr) );

            if (mOnDisconnectCallback)
            {

#if defined(_MSC_VER) && _MSC_VER < 1310

                rcfSession.setOnDestroyCallback(
                    boost::bind(vc6_boost_bind_helper_2, mOnDisconnectCallback, _1, subscriptionName));

#else

                rcfSession.setOnDestroyCallback(
                    boost::bind(mOnDisconnectCallback, _1, subscriptionName));

#endif

            }            
        }        
        return found ? RcfError_Ok : RcfError_Unspecified;
    }

#ifdef RCF_USE_PROTOBUF

    class PublishingServicePb
    {
    public:

        PublishingServicePb(PublishingService & ps) : mPs(ps)
        {
        }

        void requestSubscription(const RequestSubscription & request)
        {
            int error = mPs.requestSubscription(request.subscriptionname());

            if (error != RCF::RcfError_Ok)
            {
                RCF_THROW( RemoteException( Error(error) ) );
            }
        }

    private:
        PublishingService & mPs;
    };

    void onServiceAddedProto(PublishingService & ps, RcfServer & server)
    {
        boost::shared_ptr<PublishingServicePb> psPbPtr(
            new PublishingServicePb(ps));

        server.bind((I_RequestSubscriptionPb *) NULL, psPbPtr);
    }

    void onServiceRemovedProto(PublishingService & ps, RcfServer & server)
    {
        server.unbind( (I_RequestSubscriptionPb *) NULL);
    }

#else

    void onServiceAddedProto(PublishingService &, RcfServer &)
    {
    }

    void onServiceRemovedProto(PublishingService &, RcfServer &)
    {
    }

#endif // RCF_USE_PROTOBUF

    void PublishingService::onServiceAdded(RcfServer & server)
    {
        server.bind( (I_RequestSubscription *) NULL, *this);

        onServiceAddedProto(*this, server);
    }

    void PublishingService::onServiceRemoved(RcfServer &server)
    {
        server.unbind( (I_RequestSubscription *) NULL);

        onServiceRemovedProto(*this, server);
    }

    void PublishingService::onServerClose(RcfServer &server)
    {
        // need to do this now, rather than implicitly, when RcfServer is destroyed, because
        // the client transport objects have links to the server transport (close functor)
        RCF_UNUSED_VARIABLE(server);
        WriteLock writeLock(mPublishersMutex);
        this->mPublishers.clear();
    }

    void PublishingService::addSubscriberTransport(
        RcfSession &session,
        const std::string &publisherName,
        ClientTransportPtr clientTransportPtr)
    {
        RCF_UNUSED_VARIABLE(session);
        WriteLock lock(mPublishersMutex);
        if (mPublishers.find(publisherName) != mPublishers.end())
        {
            I_ClientTransport &clientTransport =
                mPublishers[ publisherName ]->mMulticastRcfClientPtr->
                    getClientStub().getTransport();

            MulticastClientTransport &multiCastClientTransport =
                dynamic_cast<MulticastClientTransport &>(clientTransport);

            multiCastClientTransport.addTransport(clientTransportPtr);
        }
    }
   
} // namespace RCF
