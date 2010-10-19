
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_SUBSCRIPTIONSERVICE_HPP
#define INCLUDE_RCF_SUBSCRIPTIONSERVICE_HPP

#include <map>
#include <memory>
#include <string>
#include <utility>

#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/Marshal.hpp>
#include <RCF/GetInterfaceName.hpp>
#include <RCF/ServerStub.hpp>
#include <RCF/Service.hpp>

namespace RCF {

    class RcfServer;
    class RcfSession;
    class I_ClientTransport;
    class I_ServerTransport;
    class I_Endpoint;
    class I_RcfClient;

    typedef boost::shared_ptr<I_RcfClient>          RcfClientPtr;
    typedef std::auto_ptr<I_ClientTransport>        ClientTransportAutoPtr;
    typedef boost::shared_ptr<RcfSession>           RcfSessionPtr;
    typedef boost::weak_ptr<RcfSession>             RcfSessionWeakPtr;
    typedef boost::shared_ptr<I_ServerTransport>    ServerTransportPtr;

    template<typename T> class RcfClient;
    class I_RequestSubscription;

    class RCF_EXPORT Subscription : boost::noncopyable
    {
    public:

        typedef RcfClient<I_RequestSubscription> AsyncClient;
        typedef boost::shared_ptr<AsyncClient> AsyncClientPtr;

        Subscription(
            ClientTransportAutoPtr  clientTransportAutoPtr,
            RcfSessionWeakPtr       rcfSessionWeakPtr);

        Subscription(
            AsyncClientPtr asyncClientPtr) : 
                mAsyncClientPtr(asyncClientPtr)
        {
        }

        unsigned int getPingTimestamp();
        bool isConnected();
        void close();
        RcfSessionPtr getRcfSessionPtr();
       
    private:
        friend class SubscriptionService;
        ClientTransportAutoPtr          mClientTransportAutoPtr;
        boost::weak_ptr<RcfSession>     mRcfSessionWeakPtr;
        AsyncClientPtr                  mAsyncClientPtr;
    };

    typedef boost::shared_ptr<Subscription> SubscriptionPtr;

    // ObjectId = (typename, address)
    typedef std::pair<std::string, void *> ObjectId;
    typedef std::pair<std::string, ObjectId> SubscriptionId;

    template<typename Interface>
    class BeginSubscribeParms;
    
    /// Service for implementing the subscribe part of publish/subscribe functionality.
    class RCF_EXPORT SubscriptionService :
        public I_Service,
        boost::noncopyable
    {
    public:

        SubscriptionService();

        typedef boost::function1<void, RcfSession &> OnDisconnect;

#if !defined(_MSC_VER) || _MSC_VER >= 1310

        template<typename Interface, typename Object>
        SubscriptionPtr beginSubscribe(
            Object &object,
            const I_Endpoint &publisherEndpoint,
            const std::string &publisherName = "")
        {
            return beginSubscribe( 
                (Interface*) 0, 
                object, 
                publisherEndpoint, 
                publisherName);
        }

        template<typename Interface, typename Object>
        SubscriptionPtr beginSubscribe(
            Object &object,
            ClientStub & clientStub,
            const std::string &publisherName = "")
        {
            return beginSubscribe( 
                (Interface*) 0, 
                object, 
                clientStub, 
                publisherName);
        }

        template<typename Interface, typename Object>
        SubscriptionPtr beginSubscribe(
            Object &object,
            const I_Endpoint &publisherEndpoint,
            OnDisconnect onDisconnect,
            const std::string &publisherName = "")
        {
            return beginSubscribe( 
                (Interface*) 0, 
                object, 
                publisherEndpoint, 
                onDisconnect, 
                publisherName);
        }

        template<typename Interface, typename Object>
        SubscriptionPtr beginSubscribe(
            Object &object,
            ClientTransportAutoPtr clientTransportAutoPtr,
            const std::string &publisherName = "")
        {
            return beginSubscribe( 
                (Interface*) 0, 
                object, 
                clientTransportAutoPtr, 
                publisherName);
        }

        template<typename Interface, typename Object>
        SubscriptionPtr beginSubscribe(
            Object &object,
            ClientTransportAutoPtr clientTransportAutoPtr,
            OnDisconnect onDisconnect,
            const std::string &publisherName = "")
        {
            return beginSubscribe( 
                (Interface*) 0, 
                object, 
                clientTransportAutoPtr, 
                onDisconnect, 
                publisherName);
        }

        template<typename Interface, typename Object>
        bool endSubscribe(Object &object)
        {
            return endSubscribe( (Interface *) 0, object);
        }

        template<typename Interface, typename Object>
        SubscriptionPtr getSubscriptionPtr(Object &object)
        {
            return getSubscriptionPtr( (Interface*) 0, object);
        }

#endif


        /// Begins a subscription to a remote publisher.
        /// \param Interface RCF interface of the subscription.
        /// \param Object Type of the subscriber object, must be compatible with Interface.
        /// \param object Subscriber object, that will receive published messages.
        /// \param publisherEndpoint Endpoint describing the server where the desired publishing service is located.
        /// \param publisherName Name of the publishing object to subscribe to.
        template<typename Interface, typename Object>
        SubscriptionPtr beginSubscribe(
            Interface *,
            Object &                object,
            const I_Endpoint &      publisherEndpoint,
            const std::string &     publisherName_ = "")
        {
            return beginSubscribe(
                (Interface*) 0,
                object,
                publisherEndpoint,
                OnDisconnect(),
                publisherName_);
        }

        template<typename Interface, typename Object>
        SubscriptionPtr beginSubscribe(
            Interface *,
            Object &                object,
            ClientStub &            clientStub,
            const std::string &     publisherName_ = "")
        {

            return beginSubscribe(
                (Interface*) 0,
                object,
                clientStub,
                OnDisconnect(),
                publisherName_);
        }

        template<typename Interface, typename Object>
        SubscriptionPtr beginSubscribe(
            Interface *,
            Object &                object,
            const I_Endpoint &      publisherEndpoint,
            OnDisconnect            onDisconnect,
            const std::string &     publisherName_ = "")
        {
            ClientStub clientStub("");
            clientStub.setEndpoint(publisherEndpoint);
            clientStub.instantiateTransport();

            return beginSubscribe(
                (Interface*) 0,
                object,
                clientStub,
                onDisconnect,
                publisherName_);

        }

        template<typename Interface, typename Object>
        SubscriptionPtr beginSubscribe(
            Interface *,
            Object &                object,
            ClientStub &            clientStub,
            OnDisconnect            onDisconnect,
            const std::string &     publisherName_ = "")
        {
            const std::string &publisherName = (publisherName_ == "") ?
                getInterfaceName((Interface *) NULL) :
                publisherName_;

            SubscriptionId whichSubscription = getSubscriptionId( (Interface*) 0, object);

            boost::shared_ptr< I_Deref<Object> > derefPtr(
                new DerefObj<Object>(object));

            RcfClientPtr rcfClientPtr(
                createServerStub((Interface *) 0, (Object *) 0, derefPtr));

            return beginSubscribeNamed(
                whichSubscription,
                rcfClientPtr,
                clientStub,
                onDisconnect,
                publisherName);
        }

        /// Begins a subscription to a remote publisher.
        /// \param Interface RCF interface of the subscription.
        /// \param Object Type of the subscriber object, must be compatible with Interface.
        /// \param object Subscriber object, that will receive published messages.
        /// \param clientTransportAutoPtr Client transport to be used to access the desired publishing service.
        /// \param publisherName Name of the publishing object to subscribe to.
        template<typename Interface, typename Object>
        SubscriptionPtr beginSubscribe(
            Interface *,
            Object &                object,
            ClientTransportAutoPtr  clientTransportAutoPtr,
            const std::string &     publisherName_ = "")
        {
           
            return beginSubscribe(
                (Interface*) 0,
                object,
                clientTransportAutoPtr,
                OnDisconnect(),
                publisherName_);
               
        }

        template<typename Interface, typename Object>
        SubscriptionPtr beginSubscribe(
            Interface *,
            Object &                object,
            ClientTransportAutoPtr  clientTransportAutoPtr,
            OnDisconnect            onDisconnect,
            const std::string &     publisherName_ = "")
        {
            ClientStub clientStub("");
            clientStub.setTransport(clientTransportAutoPtr);

            return beginSubscribe(
                (Interface *) 0,
                object,
                clientStub,
                onDisconnect,
                publisherName_);

        }

        /// Ends a subscription.
        /// \param Interface RCF interface of the subscription.
        /// \param Object Type of the subscription object.
        /// \param object Reference to the subscription object, for which to end the subscription.
        template<typename Interface, typename Object>
        bool endSubscribe(Interface *, Object &object)
        {
            SubscriptionId whichSubscription = 
                getSubscriptionId( (Interface*) 0, object);

            return endSubscribeNamed(whichSubscription);
        }

        /// Returns the status of the requested subscription.
        /// \param Interface RCF interface of the subscription.
        /// \param Object Type of the subscription object.
        /// \param object Reference to the subscription object, for which to obtain the status.
        /// \return SubscriptionStatus structure.
        //template<typename Interface, typename Object>
        //SubscriptionStatus getSubscriptionStatus(Object &object)
        //{
        //    SubscriptionId whichSubscription = getSubscriptionId<Interface>(object);
        //    return getSubscriptionStatusNamed(whichSubscription);
        //}

        template<typename Interface, typename Object>
        SubscriptionPtr getSubscriptionPtr(Interface *, Object &object)
        {
            SubscriptionId whichSubscription = 
                getSubscriptionId( (Interface*) 0, object);

            return getSubscriptionPtr(whichSubscription);
        }

        /// Returns the connected status of the given subscription
        /// \param Interface RCF interface of the subscription.
        /// \param Object Type of the subscription object.
        /// \param object Reference to the subscription object, for which to obtain the status.
        /// \return Connected status.
        //template<typename Interface, typename Object>
        //bool isConnected(Object &object)
        //{
        //    SubscriptionId whichSubscription = getSubscriptionId<Interface>(object);
        //    return isConnectedNamed(whichSubscription);
        //}
        
    private:

        void onServiceAdded(RcfServer &server);
        void onServiceRemoved(RcfServer &server);
        void onServerStop(RcfServer &server);
        void onServerClose(RcfServer &server);

        template<typename Object>
        ObjectId getObjectId(Object &object)
        {
            return std::make_pair( typeid(Object).name(), &object );
        }

        

        SubscriptionPtr getSubscriptionPtr(SubscriptionId whichSubscription);

#if defined(_MSC_VER) && _MSC_VER < 1310

        public:

#else

        template<typename Interface>
        friend class BeginSubscribeParms;

#endif

        template<typename Interface, typename Object>
        SubscriptionId getSubscriptionId(Interface *, Object &object)
        {
            return std::make_pair(
                getInterfaceName((Interface *) NULL),
                getObjectId(object));
        }

        SubscriptionPtr beginSubscribeNamed(
            SubscriptionId              whichSubscription,
            RcfClientPtr                rcfClientPtr,
            ClientStub &                clientStub,
            OnDisconnect                onDisconnect,
            const std::string &         publisherName);

        void beginSubscribeNamed(
            SubscriptionId              whichSubscription,
            RcfClientPtr                rcfClientPtr,
            ClientStub &                clientStub,
            OnDisconnect                onDisconnect,
            const std::string &         publisherName,
            boost::function2<void, SubscriptionPtr, ExceptionPtr> onCompletion);

        SubscriptionPtr beginSubscribeNamed(
            SubscriptionId              whichSubscription,
            RcfClientPtr                rcfClientPtr,
            ClientTransportAutoPtr      publisherClientTransportPtr,
            OnDisconnect                onDisconnect,
            const std::string &         publisherName);

        SubscriptionPtr beginSubscribeNamed(
            SubscriptionId              whichSubscription,
            RcfClientPtr                rcfClientPtr,
            const I_Endpoint &          publisherEndpoint,
            OnDisconnect                onDisconnect,
            const std::string &         publisherName);

        bool endSubscribeNamed(
            SubscriptionId              whichSubscription);

    private:

        RcfServer *                     mpServer;
        ServerTransportPtr              mServerTransportPtr;
        ReadWriteMutex                  mSubscriptionsMutex;

        typedef std::map<SubscriptionId, SubscriptionPtr > Subscriptions;
        Subscriptions                   mSubscriptions;

    public:

        template<typename Interface>
        BeginSubscribeParms<Interface> beginSubscribe()
        {
            return BeginSubscribeParms<Interface>(*this);
        }

        template<typename Interface>
        BeginSubscribeParms<Interface> beginSubscribe(Interface *)
        {
            return BeginSubscribeParms<Interface>(*this);
        }

        SubscriptionPtr onRequestSubscriptionCompleted(
            boost::int32_t                          ret,
            SubscriptionId                          whichSubscription,
            RcfClient<I_RequestSubscription> &      client,
            RcfClientPtr                            rcfClientPtr,
            OnDisconnect                            onDisconnect);

        void beginSubscribeNamedCb(
            Subscription::AsyncClientPtr            clientPtr,
            Future<boost::int32_t>                  fRet,
            SubscriptionId                          whichSubscription,
            RcfClientPtr                            rcfClientPtr,
            OnDisconnect                            onDisconnect,
            boost::function2<void, SubscriptionPtr, ExceptionPtr> onCompletion);
    
    };

    typedef boost::shared_ptr<SubscriptionService> SubscriptionServicePtr;

    template<typename Interface>
    class BeginSubscribeParms
    {
    public:

        typedef boost::function1<void, RcfSession&> OnDisconnect;

        BeginSubscribeParms(SubscriptionService & subscriptionService) :
            mSubscriptionService(subscriptionService),
            mPublisherName( getInterfaceName((Interface *) NULL) ),
            mClientStub("")
        {}

        template<typename Object>
        BeginSubscribeParms & subscriber(Object & object)
        {
            boost::shared_ptr< I_Deref<Object> > derefPtr(
                new DerefObj<Object>(object));

            RcfClientPtr rcfClientPtr(
                createServerStub((Interface *) 0, (Object *) 0, derefPtr));

            mRcfClientPtr = rcfClientPtr;

            SubscriptionId whichSubscription = 
                mSubscriptionService.getSubscriptionId( 
                    (Interface*) 0, 
                    object);

            mSubscriptionId = whichSubscription;

            return *this;
        }

        BeginSubscribeParms & publisherEndpoint(const RCF::I_Endpoint & endpoint)
        {
            mClientStub.setEndpoint(endpoint);
            return *this;
        }

        template<typename RcfClientT>
        BeginSubscribeParms & publisherEndpoint(
            RcfClientT & client)
        {
            mClientStub = client.getClientStub();
            mClientStub.setTransport(client.getClientStub().releaseTransport());
            return *this;
        }

        BeginSubscribeParms & onDisconnect(const OnDisconnect & func)
        {
            mOnDisconnect = func;
            return *this;
        }

        BeginSubscribeParms & topic(const std::string & topic)
        {
            mPublisherName = topic;
            return *this;
        }

        BeginSubscribeParms & onCompletion(boost::function2<void,SubscriptionPtr, ExceptionPtr> func)
        {
            mOnCompletion = func;
            return *this;
        }

        void operator()()
        {
            mSubscriptionService.beginSubscribeNamed(
                mSubscriptionId,
                mRcfClientPtr,
                mClientStub,
                mOnDisconnect,
                mPublisherName,
                mOnCompletion);
        }

    private:
        
        SubscriptionService &                   mSubscriptionService;
        SubscriptionId                          mSubscriptionId;
        RcfClientPtr                            mRcfClientPtr;
        std::string                             mPublisherName;
        ClientStub                              mClientStub;
        OnDisconnect                            mOnDisconnect;

        boost::function2<void, SubscriptionPtr, ExceptionPtr> mOnCompletion;
    };


} // namespace RCF

#endif // ! INCLUDE_RCF_SUBSCRIPTIONSERVICE_HPP
