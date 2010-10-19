
#include <iostream>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/FilterService.hpp>
#include <RCF/Idl.hpp>
#include <RCF/IpServerTransport.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/UdpEndpoint.hpp>
#include <RCF/ZlibCompressionFilter.hpp>

#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/Platform/OS/Sleep.hpp>

RCF_BEGIN(I_X, "I_X")
RCF_METHOD_V1(void, ping, const std::string &)
RCF_END(I_X)

RCF::Mutex gMutex;
int gPings = 0;

class X
{
public:
    void ping(const std::string &s)
    {
        RCF::Lock lock(gMutex);
        ++gPings;
    }   
};

RCF_BEGIN(I_Broadcast, "I_Broadcast")
RCF_METHOD_V1(void, serverRunningOnPort, int)
RCF_END(I_Broadcast)

class Broadcast
{
public:
    Broadcast() : mPort(RCF_DEFAULT_INIT)
    {
    }

    void serverRunningOnPort(int port)
    {
        mPort = port;
    }

    int mPort;
};


void broadcastTask(
    int port,
    const std::string &multicastIp,
    int multicastPort,
    const RCF::RcfServer &server)
{
    RcfClient<I_Broadcast> broadcast(RCF::UdpEndpoint(multicastIp, multicastPort));
    broadcast.getClientStub().setDefaultCallingSemantics(RCF::Oneway);

    while (!server.getStopFlag())
    {
        broadcast.serverRunningOnPort(port);
        Platform::OS::SleepMs(500);
    }
}

#define GCC_3_3_1_HACK (const RCF::I_Endpoint &)

int test_main(int argc, char **argv)
{
    RCF::RcfInitDeinit rcfinitDeinit;
    
    {
        X x;
        RCF::RcfServer server( RCF::UdpEndpoint("0.0.0.0", 0));
        server.bind( (I_X *) NULL, x);
        server.start();
        
        int udpPort = server.getIpServerTransport().getPort();

        gPings = 0;
        RcfClient<I_X>( GCC_3_3_1_HACK RCF::UdpEndpoint("127.0.0.1", udpPort)).ping(RCF::Oneway, "asdf");
        RcfClient<I_X>( GCC_3_3_1_HACK RCF::UdpEndpoint("232.5.5.5", udpPort)).ping(RCF::Oneway, "asdf");
        RcfClient<I_X>( GCC_3_3_1_HACK RCF::UdpEndpoint("232.5.5.5", udpPort+1)).ping(RCF::Oneway, "asdf");
        RcfClient<I_X>( GCC_3_3_1_HACK RCF::UdpEndpoint("232.5.5.6", udpPort)).ping(RCF::Oneway, "asdf");
        Platform::OS::SleepMs(1000);
        RCF_CHECK(gPings == 1);

        gPings = 0;
        RcfClient<I_X>( GCC_3_3_1_HACK RCF::UdpEndpoint("255.255.255.255", udpPort)).ping(RCF::Oneway, "asdf");
        Platform::OS::SleepMs(1000);
        RCF_CHECK(gPings >= 1);
    }

    {
        X x;
        
        RCF::RcfServer server( 
            RCF::UdpEndpoint("0.0.0.0", 0).listenOnMulticast("232.5.5.5"));

        server.bind( (I_X *) NULL, x);
        server.start();
        
        int udpPort = server.getIpServerTransport().getPort();

        gPings = 0;
        RcfClient<I_X>( GCC_3_3_1_HACK RCF::UdpEndpoint("127.0.0.1", udpPort)).ping(RCF::Oneway, "asdf");
        RcfClient<I_X>( GCC_3_3_1_HACK RCF::UdpEndpoint("232.5.5.5", udpPort)).ping(RCF::Oneway, "asdf");
        RcfClient<I_X>( GCC_3_3_1_HACK RCF::UdpEndpoint("232.5.5.5", udpPort+1)).ping(RCF::Oneway, "asdf");
        RcfClient<I_X>( GCC_3_3_1_HACK RCF::UdpEndpoint("232.5.5.6", udpPort)).ping(RCF::Oneway, "asdf");
        Platform::OS::SleepMs(1000);
        // Depending on network topology, we may receive the same message several times.
        std::cout << "Expect at least two pings, received: " << gPings << std::endl;
        RCF_CHECK(gPings >= 2);

        gPings = 0;
        RcfClient<I_X>( GCC_3_3_1_HACK RCF::UdpEndpoint("255.255.255.255", udpPort)).ping(RCF::Oneway, "asdf");
        Platform::OS::SleepMs(1000);
        RCF_CHECK(gPings >= 1);
    }

    {
        // server discovery, using an agreed multicast address and port

        X x;
        RCF::RcfServer server( RCF::TcpEndpoint(0));
        server.bind( (I_X *) NULL, x);
        server.start();

        int tcpPort = server.getIpServerTransport().getPort();        

        Broadcast broadcast;
        
        RCF::RcfServer broadcastListener( 
            RCF::UdpEndpoint("0.0.0.0", 0).listenOnMulticast("232.5.5.5"));

        broadcastListener.bind( (I_Broadcast *) NULL, broadcast);
        broadcastListener.start();
        
        int udpPort = broadcastListener.getIpServerTransport().getPort();
        
        RCF::ThreadPtr broadcastThreadPtr( new RCF::Thread( boost::bind(broadcastTask, tcpPort, "232.5.5.5", udpPort, boost::cref(server))));
        server.addJoinFunctor( boost::bind( &RCF::Thread::join, broadcastThreadPtr));
        
        // Wait at most 3 seconds for the broadcast to arrive.
        boost::uint32_t t0 = RCF::getCurrentTimeMs();
        while (!broadcast.mPort && (RCF::getCurrentTimeMs() - t0) < 3*1000)
        {
            Platform::OS::SleepMs(200);
        }

        if (broadcast.mPort)
        {
            RcfClient<I_X> client( RCF::TcpEndpoint(broadcast.mPort));
            client.ping("asdf");
        }
        else
        {
            RCF_CHECK(1 == 0);
        }
        
    }

    {
        // multiple multicast receivers, sharing a port (pub/sub)
        
        int udpPort = 0;

        std::vector< boost::shared_ptr<RCF::RcfServer> > subscribers(10);
        for (std::size_t i=0; i<subscribers.size(); ++i)
        {
            boost::shared_ptr<RCF::RcfServer> subscriber(
                new RCF::RcfServer( 
                    RCF::UdpEndpoint("0.0.0.0", udpPort)
                        .listenOnMulticast("232.5.5.5")));

            std::auto_ptr<X> px(new X());
            subscriber->bind( (I_X *) NULL, px);
            subscriber->start();

            if (udpPort == 0)
            {
                udpPort = subscriber->getIpServerTransport().getPort();
            }
            
            subscribers[i] = subscriber;
        }


        RcfClient<I_X> publisher( RCF::UdpEndpoint("232.5.5.5", udpPort));
        publisher.getClientStub().setDefaultCallingSemantics(RCF::Oneway);

        gPings = 0;
        publisher.ping("asdf");
        publisher.ping("asdf");
        publisher.ping("asdf");
        Platform::OS::SleepMs(1000);
        RCF_CHECK(gPings = subscribers.size()*3);
    }

    {
        // multiple broadcast receivers, sharing a port (pub/sub)

        X x1;
        
        RCF::RcfServer server1( 
            RCF::UdpEndpoint("0.0.0.0", 0).enableSharedAddressBinding() );

        server1.bind( (I_X *) NULL, x1);
        server1.start();

        int udpPort = server1.getIpServerTransport().getPort();

        X x2;

        RCF::RcfServer server2( 
            RCF::UdpEndpoint("0.0.0.0", udpPort).enableSharedAddressBinding());

        server2.bind( (I_X *) NULL, x2);
        server2.start();

        gPings = 0;

        RcfClient<I_X>( GCC_3_3_1_HACK RCF::UdpEndpoint("255.255.255.255", udpPort))
            .ping(RCF::Oneway, "asdf");

        Platform::OS::SleepMs(1000);
        RCF_CHECK(gPings == 2);
    }
    

    {
        // Test batched oneway calls.

        std::string s = "asdf";

        X x;
        RCF::RcfServer server( RCF::TcpEndpoint(0) );
        server.bind( (I_X * ) NULL, x);
        server.start();

        int port = server.getIpServerTransport().getPort();

        RCF::TcpEndpoint ep(port);
        RcfClient<I_X> client(ep);

        gPings = 0;

        // Separate calls.
        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);
        Platform::OS::SleepMs(1000);

        RCF_CHECK(gPings == 3);
        gPings = 0;

        // Batched calls.
        client.getClientStub().enableBatching();
        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);

        RCF_CHECK( client.getClientStub().getBatchesSent() == 0 );
        RCF_CHECK( client.getClientStub().getMessagesInCurrentBatch() == 3 );

        RCF_CHECK(gPings == 0);
        gPings = 0;

        client.getClientStub().flushBatch();
        Platform::OS::SleepMs(1000);

        RCF_CHECK(gPings == 3);
        gPings = 0;
        
        RCF_CHECK( client.getClientStub().getBatchesSent() == 1 );
        RCF_CHECK( client.getClientStub().getMessagesInCurrentBatch() == 0 );

        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);
        client.getClientStub().disableBatching();
        Platform::OS::SleepMs(1000);

        RCF_CHECK(gPings == 3);
        gPings = 0;

        RCF_CHECK( client.getClientStub().getBatchesSent() == 2 );
        RCF_CHECK( client.getClientStub().getMessagesInCurrentBatch() == 0 );

        // Automatic flushing of messages, based on message size.
        client.getClientStub().enableBatching();
        client.getClientStub().setMaxBatchMessageLength(1024);

        for (std::size_t i=0; i<1024; ++i)
        {
            client.ping(RCF::Oneway, s);
        }

        client.getClientStub().disableBatching();
        Platform::OS::SleepMs(1000);

        RCF_CHECK( client.getClientStub().getBatchesSent() > 15 );

        RCF_CHECK(gPings == 1024);
        gPings = 0;

        // TODO: 
        // * User Guide doco
        // On any form of disconnection , the batch buffer is cleared.
        // The server will process messages in the batch , in the order they are sent.
        // Message and transport filters can be used as usual.
        // Default limit on batch buffer is 1 Mb, at that point buffer is automatically flushed.
        // Batching can be used on e.g. UDP, but be aware that fragmentation can occur, and set buffer limit appropriately.


        // Check behavior on connection errors.
        client.getClientStub().enableBatching();

        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);

        client.getClientStub().disconnect();

        Platform::OS::SleepMs(1000);

        RCF_CHECK(gPings == 0);

        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);
        client.getClientStub().flushBatch();

        Platform::OS::SleepMs(1000);

        RCF_CHECK(gPings == 2);
        gPings = 0;

        client.getClientStub().disableBatching();

#ifdef RCF_USE_ZLIB

        // Test transport filters
        RCF::FilterServicePtr fsPtr( new RCF::FilterService() );

        fsPtr->addFilterFactory( RCF::FilterFactoryPtr( 
            new RCF::ZlibStatefulCompressionFilterFactory() ));

        fsPtr->addFilterFactory( RCF::FilterFactoryPtr( 
            new RCF::ZlibStatelessCompressionFilterFactory() ));

        server.addService(fsPtr);

        client.getClientStub().requestTransportFilters( 
            RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter() ) );

        client.ping(s);

        gPings = 0;

        client.getClientStub().enableBatching();

        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);

        client.getClientStub().flushBatch();
        Platform::OS::SleepMs(1000);

        RCF_CHECK(gPings == 4);
        gPings = 0;

        client.getClientStub().disableBatching();

        // Test message filters
        client.getClientStub().requestTransportFilters( RCF::FilterPtr() );

        client.getClientStub().setMessageFilters( 
            RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter() ) );

        client.ping(s);

        gPings = 0;

        client.getClientStub().enableBatching();

        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);
        client.ping(RCF::Oneway, s);

        client.getClientStub().flushBatch();
        Platform::OS::SleepMs(1000);

        RCF_CHECK(gPings == 5);
        gPings = 0;

        client.getClientStub().disableBatching();

        client.getClientStub().setMessageFilters( RCF::FilterPtr() );

#endif

    }

    return 0;
}
