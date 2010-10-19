
#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/IpClientTransport.hpp>
#include <RCF/IpServerTransport.hpp>

#include <RCF/test/TransportFactories.hpp>

#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Platform/OS/Sleep.hpp>

namespace Test_IpRestriction {

    RCF_BEGIN(I_X, "I_X")
        RCF_METHOD_V0(void, f);
    RCF_END(I_X);

    class X
    {
    public:
        void f() {}
    };

} // namespace Test_IpRestriction

int test_main(int argc, char **argv)
{
    Platform::OS::BsdSockets::disableBrokenPipeSignals();

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_IpRestriction;

    util::CommandLine::getSingleton().parse(argc, argv);

    std::string localIp = "127.0.0.1";
    int localPort = 0;

    for (unsigned int i=0; i<RCF::getIpTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getIpTransportFactories()[i];
        RCF::TransportPair transports;
        RCF::ServerTransportPtr serverTransportPtr;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr;

        transports = transportFactoryPtr->createTransports();
        serverTransportPtr = transports.first;
        clientTransportAutoPtr = *transports.second;

        X x;

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        {
            // test client restriction

            transports = transportFactoryPtr->createTransports();
            serverTransportPtr = transports.first;
            clientTransportAutoPtr = *transports.second;

            RCF::RcfServer server(serverTransportPtr);
            server.bind( (I_X*) 0, x);

            std::vector<std::string> ips;
            ips.push_back("www.usatoday.com");
            ips.push_back("www.c-span.org");
            ips.push_back("www.google.com");

            RCF::I_IpServerTransport &ipTransport = dynamic_cast<RCF::I_IpServerTransport &>(server.getServerTransport());
            ipTransport.setAllowedClientIps(ips);

            server.start();

            Platform::OS::Sleep(1);

            try
            {
                RcfClient<I_X> client(clientTransportAutoPtr->clone());
                client.f();
                RCF_CHECK(1==0);
            }
            catch(const RCF::Exception &e)
            {
                RCF_TRACE("")(e);
                RCF_CHECK(1==1);
            }

            ips = ipTransport.getAllowedClientIps();
            ips.push_back("127.0.0.1");
            ipTransport.setAllowedClientIps(ips);

            RcfClient<I_X> client(clientTransportAutoPtr->clone());
            client.f();
        }



        {
            // test interface restriction: loopback interface

            transports = transportFactoryPtr->createTransports();
            serverTransportPtr = transports.first;
            clientTransportAutoPtr = *transports.second;

            std::vector<std::string> interfaces;
            interfaces.push_back("127.0.0.1");

            for (unsigned int i=0; i<interfaces.size(); i++)
            {
                RCF::RcfServer server(serverTransportPtr);
                server.bind( (I_X*) 0, x);

                RCF::I_IpServerTransport &ipTransport = dynamic_cast<RCF::I_IpServerTransport &>(server.getServerTransport());
                ipTransport.setNetworkInterface(interfaces[i]);

                server.start();

                RcfClient<I_X>(clientTransportAutoPtr->clone()).f();
                RCF_CHECK(1==1);
            }
        }

        {
            // Test retrieval of local ip and port number of a TCP connection

            transports = transportFactoryPtr->createTransports();
            serverTransportPtr = transports.first;
            clientTransportAutoPtr = *transports.second;

            RCF::RcfServer server(serverTransportPtr);
            server.bind( (I_X*) 0, x);
            server.start();

            RcfClient<I_X> client( clientTransportAutoPtr->clone() );
            client.f();

            RCF::I_IpClientTransport &ipTransport = client.getClientStub().getIpTransport();

            std::string localIp = ipTransport.getLocalIp();

            int port = ipTransport.getLocalPort();
            RCF_CHECK(port > 0);

        }

    }

    return boost::exit_success;
}
