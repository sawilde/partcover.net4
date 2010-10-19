
#include <vector>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/PublishingService.hpp>
#include <RCF/SubscriptionService.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Profile.hpp>
#include <RCF/util/Platform/OS/Sleep.hpp>

#include <RCF/test/ThreadGroup.hpp>

#ifndef BOOST_WINDOWS

namespace RCF {

    class Win32NamedPipeEndpoint            {};
    class Win32NamedPipeClientTransport     {};
    class Win32NamedPipeServerTransport     {};

} // namespace RCF

#endif

namespace Test_Notification {

    RCF_BEGIN(I_Events, "I_Events")
        RCF_METHOD_V0(void, onA)
        RCF_METHOD_V1(void, onB, int)
        RCF_METHOD_V2(void, onC, int, const std::string &)
    RCF_END(I_Events)

    class Events
    {
    public:

        Events() : nA(RCF_DEFAULT_INIT), nB(RCF_DEFAULT_INIT), nC(RCF_DEFAULT_INIT)
        {}

        void onA()
        {
            nA++;
        }

        void onB(int n)
        {
            nB++;
        }

        void onC(int n, std::string s)
        {
            nC++;
        }

        int nA;
        int nB;
        int nC;

    };

    class ServerTask
    {
    public:
        ServerTask(
            RCF::PublishingService &publishingService,
            const volatile bool *stopFlag) :
                mPublishingService(publishingService),
                mStopFlag(*stopFlag)
        {}

        void operator()()
        {
            try
            {
                mPublishingService.beginPublish( (I_Events*) 0);
                while (!mStopFlag)
                {
                    mPublishingService.publish( (I_Events*) 0).onA();
                }
                mPublishingService.endPublish( (I_Events*) 0);
            }
            catch(const std::exception &e)
            {
                std::cout << "Server task exception" << std::endl;
                std::cout << RCF::toString(e) << std::endl;
            }
        }

    private:
        RCF::PublishingService &mPublishingService;
        const volatile bool &mStopFlag;
    };

    class ClientTask
    {
    public:
        ClientTask(
            RCF::SubscriptionService &subscriptionService,
            RCF::EndpointPtr endpointPtr) :
                mSubscriptionService(subscriptionService),
                mEndpointPtr(endpointPtr)
        {}

        void operator()()
        {
            Events events;

            // Changed this from 1000 to 100 because it was taking so long on unix platforms.
            for (std::size_t n=0; n<100; ++n)
            {
                try
                {

                    mSubscriptionService.beginSubscribe(
                        (I_Events*) 0,
                        events,
                        *mEndpointPtr);

                    mSubscriptionService.endSubscribe(
                        (I_Events*) 0,
                        events);
                        
                }
                catch(const std::exception &e)
                {
                    std::cout << "Client task exception, n=" << n << std::endl;
                    std::cout << RCF::toString(e) << std::endl;
                }
            }
        }

    private:
        RCF::SubscriptionService &mSubscriptionService;
        RCF::EndpointPtr mEndpointPtr;
    };

    std::size_t gSubscriberConnects = 0;
    std::size_t gSubscriberDisconnects = 0;

    void subscriberConnect(
        RCF::RcfSession & rcfSession, 
        const std::string & publisherName)
    {
        RCF_CHECK(
            publisherName == RCF::getInterfaceName( (I_Events *) NULL ));

        ++gSubscriberConnects;
    }

    void subscriberDisconnect(
        RCF::RcfSession & rcfSession, 
        const std::string & publisherName)
    {
        RCF_CHECK(
            publisherName == RCF::getInterfaceName( (I_Events *) NULL ));

        ++gSubscriberDisconnects;
    }

    void test1(
        RCF::PublishingService &publishingService,
        RCF::SubscriptionService &subscriptionService,
        RCF::EndpointPtr publisherClientEndpointPtr,
        std::vector<Events> &events)
    {
        std::cout << "Test 1: Few subscribers" << std::endl;

        publishingService.setOnConnectCallback( 
            boost::bind(subscriberConnect, _1, _2) );

        publishingService.setOnDisconnectCallback( 
            boost::bind( subscriberDisconnect, _1, _2) );

        events.clear();
        events.resize(3);

        for (std::size_t j=0; j<3;++j)
        {
            gSubscriberConnects = 0;
            gSubscriberDisconnects = 0;

            publishingService.beginPublish( (I_Events*) 0);

            for (std::size_t k=0; k<events.size(); ++k)
            {
                events[k] = Events();
                subscriptionService.beginSubscribe(
                    (I_Events *) 0,
                    events[k],
                    *publisherClientEndpointPtr);
            }

            // give the server time to setup the subscriptions
            Platform::OS::Sleep(1);

            for (std::size_t k=0; k<events.size(); ++k)
            {
                subscriptionService.endSubscribe( (I_Events*) 0, events[k]);
                publishingService.publish( (I_Events*) 0).onA();
                publishingService.publish( (I_Events*) 0).onB(1);
                publishingService.publish( (I_Events*) 0).onC(1, "one");

                // give the subscribers time to receive the notifications
                Platform::OS::Sleep(1);
            }

            publishingService.endPublish( (I_Events*) 0);

            for (std::size_t k=0; k<events.size(); ++k)
            {
                RCF_CHECK(events[k].nA == k);
                RCF_CHECK(events[k].nB == k);
                RCF_CHECK(events[k].nC == k);
            }

            RCF_CHECK(gSubscriberConnects = events.size());
            RCF_CHECK(gSubscriberDisconnects == events.size());
        }

        publishingService.setOnConnectCallback(
            RCF::PublishingService::OnConnectCallback());

        publishingService.setOnDisconnectCallback(
            RCF::PublishingService::OnConnectCallback());
    }

    void test2(
        RCF::PublishingService &publishingService,
        RCF::SubscriptionService &subscriptionService,
        RCF::EndpointPtr publisherClientEndpointPtr)
    {
        // intensive subscribing and unsubscribing

        std::cout << "Test 2: Intensive subscribing and unsubscribing" << std::endl;
        
        // now fire off all the threads
        bool serverThreadStopFlag = false;
        Thread serverThread(( ServerTask(
            publishingService,
            &serverThreadStopFlag)));

        Platform::OS::SleepMs(1000);

        ThreadGroup clientThreads;
        for (std::size_t j=0; j<2; ++j)
        {
            clientThreads.push_back( ThreadPtr( new Thread( ClientTask(
                subscriptionService,
                publisherClientEndpointPtr->clone()))));
        }
        joinThreadGroup(clientThreads);

        serverThreadStopFlag = true;
        serverThread.join();
    }

    void onSubscriptionDisconnect(std::vector<bool> &v, std::size_t which)
    {
        RCF_ASSERT(which < v.size());
        RCF_ASSERT(v[which] == false);
        v[which] = true;
    }

    void test3(
        RCF::PublishingService &publishingService,
        RCF::SubscriptionService &subscriptionService,
        RCF::EndpointPtr publisherClientEndpointPtr,
        std::vector<Events> &events)
    {
        std::cout << "Test 3: Disconnect notifications" << std::endl;

        events.clear();
        events.resize(10);
        std::vector<bool> disconnectedEvents(10);

        typedef RCF::SubscriptionPtr SubscriptionPtr;
        SubscriptionPtr subscriptionPtr;

        publishingService.beginPublish( (I_Events*) 0);

        for (std::size_t j=0; j<events.size(); ++j)
        {
            // insertion of following line fixed a bizarre ICE with vc6
            std::vector<bool> vb;

            SubscriptionPtr subscriptionPtr =
                subscriptionService.beginSubscribe(
                    (I_Events*) 0,
                    events[j],
                    *publisherClientEndpointPtr,
                    boost::bind(
                        onSubscriptionDisconnect,
                        boost::ref(disconnectedEvents),
                        j));
        }

        // publish to all subscribers
        for (std::size_t j=0; j<events.size(); ++j)
        {
            RCF_CHECK(events[j].nA == 0);
            RCF_CHECK(events[j].nB == 0);
            RCF_CHECK(events[j].nC == 0);
        }

        Platform::OS::Sleep(1);

        publishingService.publish( (I_Events*) 0).onA();
        publishingService.publish( (I_Events*) 0).onB(1);
        publishingService.publish( (I_Events*) 0).onC(1, "one");

        unsigned int t0 = Platform::OS::getCurrentTimeMs();
        publishingService.publish( (I_Events*) 0).getClientStub().ping();

        Platform::OS::Sleep(1);
        unsigned int t1 = Platform::OS::getCurrentTimeMs();

        for (std::size_t j=0; j<events.size(); ++j)
        {
            RCF_CHECK(events[j].nA == 1);
            RCF_CHECK(events[j].nB == 1);
            RCF_CHECK(events[j].nC == 1);
        }

        // check that we get pings on all subscriptions
        for (std::size_t j=0; j<events.size(); ++j)
        {
            subscriptionPtr = subscriptionService.getSubscriptionPtr( 
                (I_Events*) 0, 
                events[j]);

            RCF_CHECK( subscriptionPtr.get() );

            unsigned int pingTimeStamp = subscriptionPtr->getPingTimestamp();
            RCF_CHECK( t0 <= pingTimeStamp && pingTimeStamp <= t1 );
        }


        // check that polling for disconnections works, explicit disconnect

        subscriptionPtr = subscriptionService.getSubscriptionPtr(
            (I_Events*) 0,
            events[0]);

        RCF_CHECK( subscriptionPtr.get() && subscriptionPtr->isConnected());

        subscriptionService.endSubscribe( (I_Events*) 0, events[0]);

        subscriptionPtr = subscriptionService.getSubscriptionPtr(
            (I_Events*) 0,
            events[0]);

        RCF_CHECK(!subscriptionPtr);


        // check that polling for disconnections works, implicit disconnect

        for (std::size_t j=1; j<events.size(); ++j)
        {
            subscriptionPtr = subscriptionService.getSubscriptionPtr( 
                (I_Events*) 0, 
                events[j]);

            RCF_CHECK(subscriptionPtr.get() && subscriptionPtr->isConnected());
        }

        publishingService.endPublish( (I_Events*) 0);
        Platform::OS::Sleep(1);

        for (std::size_t j=1; j<events.size(); ++j)
        {
            subscriptionPtr = subscriptionService.getSubscriptionPtr( 
                (I_Events*) 0, 
                events[j]);

            RCF_CHECK(subscriptionPtr.get() && !subscriptionPtr->isConnected());
        }


        // check that we have disconnect notifications on all but first subscription

        RCF_CHECK(disconnectedEvents[0] == false);
        for (std::size_t j=1; j<disconnectedEvents.size(); ++j)
        {
            RCF_CHECK(disconnectedEvents[j] == true);
        }

        // have to call endSubscribe() before events[] goes out of scope!
        for (std::size_t j=1; j<events.size(); ++j)
        {
            subscriptionService.endSubscribe( (I_Events*) 0, events[j]);
        }
    }

    void test4(
        RCF::PublishingService &publishingService,
        RCF::SubscriptionService &subscriptionService,
        RCF::EndpointPtr publisherClientEndpointPtr,
        RCF::ServerTransportPtr publisherServerTransportPtr,
        std::vector<Events> &events)
    {
        std::cout << "Test 4: Many subscribers" << std::endl;
        events.clear();
        events.resize(100);

        for (std::size_t j=0; j<5; ++j)
        {
            publishingService.beginPublish( (I_Events*) 0);

            for (std::size_t k=0; k<events.size(); ++k)
            {
                events[k] = Events();
                subscriptionService.beginSubscribe(
                    (I_Events*) 0,
                    events[k],
                    *publisherClientEndpointPtr);
            }

            {
                std::string transportName = typeid(*publisherServerTransportPtr).name();
                util::Profile profile(transportName + ": time spent on publishing calls");
                for (unsigned int k=0; k<events.size()/2; ++k)
                {
                    subscriptionService.endSubscribe( (I_Events*) 0, events[k]);
                    publishingService.publish( (I_Events*) 0).onA();
                    publishingService.publish( (I_Events*) 0).onB(1);
                    publishingService.publish( (I_Events*) 0).onC(1, "one");
                }
            }

            for (std::size_t k=events.size()/2; k<events.size(); ++k)
            {
                subscriptionService.endSubscribe( (I_Events*) 0, events[k]);
            }

            publishingService.endPublish( (I_Events*) 0);
        }
    }

    void test5(
        RCF::PublishingService &publishingService,
        RCF::SubscriptionService &subscriptionService,
        RCF::EndpointPtr publisherClientEndpointPtr,
        std::vector<Events> &events)
    {
        std::cout << "Test 5: Topics" << std::endl;

        events.clear();
        events.resize(3);

        std::vector<std::string> topics;
        topics.push_back("Topic1");
        topics.push_back("Topic2");
        topics.push_back("Topic3");

        RCF_ASSERT(events.size() == topics.size());

        for (std::size_t i=0; i<topics.size(); ++i)
        {
            publishingService.beginPublish( (I_Events*) 0, topics[i]);
        }

        for (std::size_t i=0; i<events.size(); ++i)
        {
            events[i] = Events();
            subscriptionService.beginSubscribe(
                (I_Events *) 0,
                events[i],
                *publisherClientEndpointPtr,
                topics[i]);
        }

        // give the server time to setup the subscriptions
        Platform::OS::Sleep(1);

        for (std::size_t i=0; i<topics.size(); ++i)
        {
            publishingService.publish( (I_Events*) 0, topics[i]).onA();
            publishingService.publish( (I_Events*) 0, topics[i]).onB(1);
            publishingService.publish( (I_Events*) 0, topics[i]).onC(1, "one");
        }

        // give the subscribers time to receive the notifications
        Platform::OS::Sleep(1);

        for (std::size_t i=0; i<topics.size(); ++i)
        {
            publishingService.endPublish( (I_Events*) 0, topics[i]);
        }
        
        for (std::size_t i=0; i<events.size(); ++i)
        {
            RCF_CHECK(events[i].nA == 1);
            RCF_CHECK(events[i].nB == 1);
            RCF_CHECK(events[i].nC == 1);
        }    
    }


    void shouldNotBeCalled()
    {
        RCF_CHECK(1==0);
    }

    // Bidirectional transport tests

    RCF_BEGIN(I_X, "I_X")
        RCF_METHOD_V0(void, turnAroundForOneway)
        RCF_METHOD_V0(void, turnAroundForTwoway)
        RCF_METHOD_R1(std::string, echo, const std::string &)
    RCF_END(I_X)

    RCF_BEGIN(I_Y, "I_Y")
        RCF_METHOD_R1(std::string, echoBack, const std::string &)
    RCF_END(I_Y)

    RCF::ClientTransportAutoPtr gClientTransportAutoPtr;

    class X
    {
    public:

        X() : mEchoCount(RCF_DEFAULT_INIT)
        {}

        void turnAroundForOneway()
        {
            RCF::convertRcfSessionToRcfClient( 
                boost::bind(&X::onConversionCompleted, _1),
                RCF::Oneway);
        }

        void turnAroundForTwoway()
        {
            RCF::convertRcfSessionToRcfClient( 
                boost::bind(&X::onConversionCompleted, _1),
                RCF::Twoway);
        }

        std::string echo(const std::string &s)
        {
            ++mEchoCount;
            return s;
        }

        std::size_t mEchoCount;

    private:

        static void onConversionCompleted(
            RCF::ClientTransportAutoPtr clientTransportAutoPtr)
        {
            // The client transport is now ready to be used.
            gClientTransportAutoPtr = clientTransportAutoPtr;
        }
    };

    class Y
    {
    public:

        Y() : mEchoCount(RCF_DEFAULT_INIT)
        {}

        std::string echoBack(const std::string & s)
        {
            ++mEchoCount;
            return s;
        }

        std::size_t mEchoCount;
    };



    void testBidirectionalTransports(
        RCF::TransportFactoryPtr transportFactoryPtr)
    {
        RCF::TransportPair transports;

        transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr1( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr1( *transports.second );

        transports = transportFactoryPtr->createNonListeningTransports();
        RCF::ServerTransportPtr serverTransportPtr2( transports.first );

        RCF::writeTransportTypes(
            std::cout,
            *serverTransportPtr1,
            *clientTransportAutoPtr1);

        X x;
        RCF::RcfServer server1( serverTransportPtr1 );
        server1.bind( (I_X *) 0, x);
        server1.start();

        {
            // Client-local server, for receiving callbacks on the client connection.
            Y y;
            RCF::RcfServer server2( serverTransportPtr2 );
            server2.bind( (I_Y *) 0, y);
            server2.start();

            // Test oneway calls in either direction.
            {
                // Make a remote call to the server.
                RcfClient<I_X> client1( clientTransportAutoPtr1->clone() );
                client1.turnAroundForOneway();

                // Convert the client connection to a server session.
                RCF::convertRcfClientToRcfSession(client1, server2);

                while (!gClientTransportAutoPtr.get())
                {
                    Platform::OS::SleepMs(100);
                }

                // The server can now make calls back to the client, on the same 
                // connection used by the client to make the original call.
                {
                    RcfClient<I_Y> client2( gClientTransportAutoPtr );

                    // Oneway calls in either direction.

                    int calls = 100;
                    for (int i=0; i<calls; ++i)
                    {
                        client1.echo(RCF::Oneway, "asdf");
                        client2.echoBack(RCF::Oneway, "asdf");
                    }

                    Platform::OS::SleepMs(1000);

                    RCF_CHECK( x.mEchoCount == calls );
                    RCF_CHECK( y.mEchoCount == calls );
                }
            }

            gClientTransportAutoPtr.reset();

            // TODO: twoway calls back to the client aren't working with
            // Win32 named pipes, because Win32NamedPipeClientTransport uses
            // overlapped I/O to read data, thus the read operations get routed
            // to the server instead.

            // Test twoway calls in reverse direction.

            if (
                ! dynamic_cast<RCF::Win32NamedPipeServerTransport *>(
                    serverTransportPtr1.get()) )
            {
                // Make a remote call to the server.
                RcfClient<I_X> client1( clientTransportAutoPtr1->clone() );
                client1.turnAroundForTwoway();

                // Convert the client connection to a server session.
                RCF::convertRcfClientToRcfSession(client1, server2);

                while (!gClientTransportAutoPtr.get())
                {
                    Platform::OS::SleepMs(100);
                }

                // The server can now make calls back to the client, on the same 
                // connection used by the client to make the original call.
                {
                    RcfClient<I_Y> client2( gClientTransportAutoPtr );

                    // Twoway calls in reverse direction.

                    y.mEchoCount = 0;

                    int calls = 100;
                    for (int i=0; i<calls; ++i)
                    {
                        client2.echoBack(RCF::Twoway, "asdf");
                    }

                    RCF_CHECK( y.mEchoCount == calls );
                }
            }
        }
    }

} // namespace Test_Notification

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    Platform::OS::BsdSockets::disableBrokenPipeSignals();

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_Notification;

    util::CommandLine::getSingleton().parse(argc, argv);

    for (std::size_t i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        if ( RCF::getTransportFactories()[i]->isConnectionOriented() )
        {
            testBidirectionalTransports( RCF::getTransportFactories()[i] );
        }
    }

    for (std::size_t i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::SubscriptionPtr subscriptionPtr1;
        RCF::SubscriptionPtr subscriptionPtr2;
        {
            RCF::TransportFactoryPtr transportFactoryPtr;
            RCF::TransportPair transports;

            transportFactoryPtr = RCF::getTransportFactories()[i];
            transports = transportFactoryPtr->createTransports();
            RCF::ServerTransportPtr publisherServerTransportPtr( transports.first );
            RCF::ClientTransportAutoPtr publisherClientTransportAutoPtr( *transports.second );

            transportFactoryPtr = RCF::getTransportFactories()[i];
            transports = transportFactoryPtr->createNonListeningTransports();
            RCF::ServerTransportPtr subscriberServerTransportPtr( transports.first );

            RCF::RcfServer publisher(publisherServerTransportPtr);
            RCF::RcfServer subscriber(subscriberServerTransportPtr);

            // need both I_ServerTransportEx and I_ServerTransportSessionFull for
            // publish/subscribe functionality
            RCF::I_ServerTransportEx *pServerTransportEx =
                dynamic_cast<RCF::I_ServerTransportEx *>(&publisher.getServerTransport());

            if (NULL == pServerTransportEx)
            {
                continue;
            }

            RCF::writeTransportTypes(
                std::cout,
                *publisherServerTransportPtr,
                *publisherClientTransportAutoPtr);

            RCF::PublishingServicePtr publishingServicePtr(new RCF::PublishingService);
            RCF::PublishingService &publishingService = *publishingServicePtr;
            publisher.addService(publishingServicePtr);

            RCF::SubscriptionServicePtr subscriptionServicePtr(new RCF::SubscriptionService);
            RCF::SubscriptionService &subscriptionService = *subscriptionServicePtr;
            subscriber.addService(subscriptionServicePtr);

            publisher.start();
            subscriber.start();

            // NB: must call endSubscribe<>(object) before object goes out of scope!
            // ie, object must outlive the subscription. Otherwise we get memory
            // corruption from invoking methods on a dead object.

            // TODO: only allow shared_ptr's for subscriptions?

            RCF::EndpointPtr publisherClientEndpointPtr(
                publisherClientTransportAutoPtr->getEndpointPtr());

            // the vec nonsense came about while porting to vc6

            std::vector<Events> vec;

            vec.clear();
            test1(
                publishingService,
                subscriptionService,
                publisherClientEndpointPtr,
                vec);

            vec.clear();
            test2(
                publishingService,
                subscriptionService,
                publisherClientEndpointPtr);

            vec.clear();
            test3(
                publishingService,
                subscriptionService,
                publisherClientEndpointPtr,
                vec);

            vec.clear();
            test4(
                publishingService,
                subscriptionService,
                publisherClientEndpointPtr,
                publisherServerTransportPtr,
                vec);

            vec.clear();
            test5(
                publishingService,
                subscriptionService,
                publisherClientEndpointPtr,
                vec);

            // Test that a SubscriptionPtr can be destroyed after the
            // corresponding RcfServer.
            publishingService.beginPublish( (I_Events*) 0);

            Events events[2];

            subscriptionPtr1 = subscriptionService.beginSubscribe(
                (I_Events *) 0,
                events[0], 
                *publisherClientEndpointPtr);

            subscriptionPtr2 = subscriptionService.beginSubscribe(
                (I_Events *) 0,
                events[1], 
                *publisherClientEndpointPtr,
                boost::bind(shouldNotBeCalled));

            subscriber.stop();
            publisher.stop();
        }

        subscriptionPtr1.reset();
        subscriptionPtr2.reset();
    }

    return boost::exit_success;
}
