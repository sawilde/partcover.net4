
#include <memory>
#include <string>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/SessionTimeoutService.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Platform/OS/Sleep.hpp>

namespace Test_ClientConnectionDrop {

    RCF_BEGIN(I_X, "I_X")
        RCF_METHOD_R1(std::string, echo, std::string)
        RCF_METHOD_V0(void, disconnectMe)
    RCF_END(I_X)

    struct X
    {
        std::string echo(const std::string &s)
        {
            return s;
        }

        void disconnectMe()
        {
            RCF::getCurrentRcfSession().disconnect();
        }
    };

}


int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_ClientConnectionDrop;

    util::CommandLine::getSingleton().parse(argc, argv);

    for (int i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        {
            RCF::TransportPair transports = transportFactoryPtr->createTransports();
            RCF::ServerTransportPtr serverTransportPtr( transports.first );
            RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

            if (!transportFactoryPtr->isConnectionOriented())
            {
                continue;
            }

            RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

            X x;
            std::string s;
            std::auto_ptr<RCF::RcfServer>  server;

            server.reset( new RCF::RcfServer(serverTransportPtr) );
            server->bind( (I_X*) 0, x);
            server->start();
            
            RcfClient<I_X> client( clientTransportAutoPtr->clone() );
           
            s = client.echo("abc");
            RCF_CHECK( s == "abc" );
            s = client.echo("def");
            RCF_CHECK( s == "def" );

            server->close();
            server->start();

            // client should detect that it's connection is dead at this point, and automatically create a new one
            s = client.echo("abc");
            RCF_CHECK( s == "abc" );
            s = client.echo("def");
            RCF_CHECK( s == "def" );

            // only do the following on Windows (socket linger issue)
    #ifdef BOOST_WINDOWS
            server.reset();
            server.reset( new RCF::RcfServer(serverTransportPtr) );
            //server->bind<I_X>(x);
            server->bind( (I_X*) 0, x);
            server->start();
    #endif
            s = client.echo("abc");
            RCF_CHECK( s == "abc" );
            s = client.echo("def");
            RCF_CHECK( s == "def" );

            RCF::I_ServerTransportEx *pServerTransportEx =
                dynamic_cast<RCF::I_ServerTransportEx *>(serverTransportPtr.get());

            if (pServerTransportEx)
            {
                // test forcible client disconnection
                try
                {
                    client.disconnectMe();
                    RCF_CHECK(1==0);
                }
                catch (RCF::RemoteException &e)
                {
                    RCF_CHECK(1==0);
                }
                catch (RCF::Exception &e)
                {
                    RCF_CHECK(1==1);
                }

                client.echo("");
                client.disconnectMe(RCF::Oneway);
                Platform::OS::Sleep(1);
                RCF_CHECK( !client.getClientStub().isConnected() );
            }
        }

        {
            // Test idle connection timeouts.

            RCF::TransportPair transports = transportFactoryPtr->createTransports();
            RCF::ServerTransportPtr serverTransportPtr( transports.first );
            RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

            std::cout << "Testing session idle timeouts." << std::endl;

            RCF::RcfServer server(serverTransportPtr);

            X x;
            server.bind( (I_X *) 0, x);

            // 5 second session timeout, and run the reaping loop every second.
            const boost::uint32_t SessionTimeoutMs = 5*1000;
            const boost::uint32_t ReapingIntervalMs = 1*1000;

            RCF::SessionTimeoutServicePtr stsPtr( 
                new RCF::SessionTimeoutService(SessionTimeoutMs, ReapingIntervalMs) );

            server.addService(stsPtr);

            server.start();

            std::vector< RcfClient<I_X> > clients;

            for (std::size_t j=0; j<5; ++j)
            {
                clients.push_back( RcfClient<I_X>( clientTransportAutoPtr->clone() ) );
                clients.back().getClientStub().setAutoReconnect(false);
            }

            for (std::size_t j=0; j<clients.size(); ++j)
            {
                clients[j].echo(std::string());
            }

            Platform::OS::Sleep(4);
            
            clients[2].echo(std::string());

            Platform::OS::Sleep(4);

            // Check that all connections except #2 have been disconnected.

            clients[2].echo(std::string());

            for (std::size_t j=0; j<clients.size(); ++j)
            {
                bool connected = clients[j].getClientStub().isConnected();
                if (j == 2)
                {
                    RCF_CHECK(connected);
                }
                else
                {
                    RCF_CHECK(!connected);
                }
            }
        }
    }

    return boost::exit_success;
}
