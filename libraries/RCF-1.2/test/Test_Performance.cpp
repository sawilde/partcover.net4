
#include <string>
#include <boost/lexical_cast.hpp>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/FilterService.hpp>
#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/UdpEndpoint.hpp>

#include <RCF/test/TransportFactories.hpp>

#include "Test_Performance.hpp"

#include <RCF/util/AutoBuild.hpp>
#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Profile.hpp>

#ifdef RCF_USE_OPENSSL
#include <RCF/OpenSslEncryptionFilter.hpp>
#endif

#ifdef RCF_USE_ZLIB
#include <RCF/ZlibCompressionFilter.hpp>
#endif

#ifdef BOOST_WINDOWS
#include <RCF/SspiFilter.hpp>
#endif

namespace Test_Performance {

    void udpServer(int acceptorSocket, int calls, int buflen)
    {
        int packetsSent = 0;
        int packetsReceived = 0;
        int bytesSent = 0;
        int bytesReceived = 0;

        int ret = 0;

        std::vector<char> buffer(buflen);

        sockaddr_in fromAddr = {0};
        int fromAddrLen = sizeof(fromAddr);

        for (int i=0; i<calls; ++i)
        {
            ret = Platform::OS::BsdSockets::recvfrom(acceptorSocket, &buffer[0], buffer.size(), 0, (sockaddr *)&fromAddr, &fromAddrLen);
            RCF_VERIFY(ret > 0, std::runtime_error("recvfrom()"));
            bytesSent += ret;
            packetsSent += 1;

            ret = Platform::OS::BsdSockets::sendto(acceptorSocket, &buffer[0], ret, 0, (sockaddr *)&fromAddr, fromAddrLen);
            RCF_VERIFY(ret > 0, std::runtime_error("sendto()"));
            bytesReceived += ret;
            packetsReceived += 1;
        }
    }

    void testUdp(const std::string &title, int calls, int buflen)
    {
        std::string ip = "127.0.0.1";
        int port = 0;

        int ret = 0;

        sockaddr_in clientAddr = {0};
        clientAddr.sin_family = AF_INET;
        clientAddr.sin_port = 0;
        clientAddr.sin_addr.s_addr = inet_addr(ip.c_str());

        sockaddr_in serverAddr = {0};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

        int acceptorSocket = socket( AF_INET, SOCK_DGRAM, 0 );

        ret = ::bind(acceptorSocket, (sockaddr *)&serverAddr, sizeof(serverAddr));
        RCF_VERIFY(ret == 0, std::runtime_error("bind()"));

        RCF::Thread serverThread( boost::bind( &udpServer, acceptorSocket, calls, buflen));

        port = RCF::getFdPort(acceptorSocket);
        RCF_VERIFY(port != 0, std::runtime_error("could not retrieve port number"));

        sockaddr_in toAddr = {0};
        toAddr.sin_family = AF_INET;
        toAddr.sin_port = htons(port);
        toAddr.sin_addr.s_addr = inet_addr(ip.c_str());

        sockaddr_in fromAddr = {0};
        int fromAddrLen = sizeof(fromAddr);

        int clientSocket = socket( AF_INET, SOCK_DGRAM, 0 );

        int packetsSent = 0;
        int packetsReceived = 0;
        int bytesSent = 0;
        int bytesReceived = 0;
        
        ret = ::bind(clientSocket, (sockaddr *)&clientAddr, sizeof(clientAddr));
        RCF_VERIFY(ret == 0, std::runtime_error("bind()"));

        std::string msg(buflen, 'A');
        std::vector<char> buffer(buflen);

        {
            util::Profile profile( title + "raw udp round trips, " + boost::lexical_cast<std::string>(calls) + " calls");
            for (int i=0; i<calls; ++i)
            {
                ret = Platform::OS::BsdSockets::sendto(clientSocket, msg.c_str(), msg.size(), 0, (sockaddr *)&toAddr, sizeof(toAddr));
                RCF_VERIFY(ret > 0, std::runtime_error("sendto()"));
                bytesSent += ret;
                packetsSent += 1;

                ret = Platform::OS::BsdSockets::recvfrom(clientSocket, &buffer[0], buffer.size(), 0, (sockaddr *)&fromAddr, &fromAddrLen);
                RCF_VERIFY(ret > 0, std::runtime_error("recvfrom()"));
                bytesReceived += ret;
                packetsReceived += 1;
            }
        }

        std::cout << "Packets sent: "       << packetsSent      << std::endl;
        std::cout << "Bytes sent: "         << bytesSent        << std::endl;
        std::cout << "Packets received: "   << packetsReceived  << std::endl;
        std::cout << "Bytes received: "     << bytesReceived    << std::endl;

        Platform::OS::BsdSockets::closesocket(clientSocket);

        Platform::OS::BsdSockets::closesocket(acceptorSocket);
    }

    void tcpServer(int acceptorSocket, int calls, int buflen)
    {
        int packetsSent = 0;
        int packetsReceived = 0;
        int bytesSent = 0;
        int bytesReceived = 0;

        std::vector<char> buffer(buflen);

        sockaddr_in fromAddr = {0};
        int fromAddrLen = sizeof(fromAddr);

        int serverSocket = Platform::OS::BsdSockets::accept(acceptorSocket, (sockaddr *)&fromAddr, &fromAddrLen);
        RCF_VERIFY(serverSocket > 0, std::runtime_error("accept()"))(serverSocket);

        //while (true)
        for (int i=0; i<calls; ++i)
        {
            int ret = Platform::OS::BsdSockets::recv(serverSocket, &buffer[0], buffer.size(), 0);
            RCF_VERIFY(ret == 0 || ret == buflen, std::runtime_error("recv()"))(ret);

            if (ret == 0)
            {
                Platform::OS::BsdSockets::closesocket(serverSocket);
                break;
            }

            bytesSent += ret;
            packetsSent += 1;

            ret = Platform::OS::BsdSockets::send(serverSocket, &buffer[0], ret, 0);
            RCF_VERIFY(ret == buflen, std::runtime_error("send()"))(ret);

            bytesReceived += ret;
            packetsReceived += 1;
        }
    }

    void testTcp(const std::string &title, int calls, int buflen)
    {
        std::string ip = "127.0.0.1";
        int port = 0;

        int ret = 0;

        int packetsSent = 0;
        int packetsReceived = 0;
        int bytesSent = 0;
        int bytesReceived = 0;

        int acceptorSocket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        sockaddr_in serverAddr = {0};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

        ret = ::bind(acceptorSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
        RCF_VERIFY(ret == 0, std::runtime_error("bind()"));

        ret = listen(acceptorSocket, 10);
        RCF_VERIFY(ret == 0, std::runtime_error("listen()"));

        port = RCF::getFdPort(acceptorSocket);
        RCF_VERIFY(port != 0, std::runtime_error("could not retrieve port number"));

        RCF::Thread serverThread( boost::bind( &tcpServer, acceptorSocket, calls, buflen));

        int clientSocket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        sockaddr_in toAddr = {0};
        toAddr.sin_family = AF_INET;
        toAddr.sin_port = htons(port);
        toAddr.sin_addr.s_addr = inet_addr(ip.c_str());

        ret = Platform::OS::BsdSockets::connect(clientSocket, (sockaddr *)&toAddr, sizeof(toAddr));
        RCF_VERIFY(ret == 0, std::runtime_error("bind()"));

        std::string msg(buflen, 'A');
        std::vector<char> buffer(buflen);

        {
            util::Profile profile( title + "raw tcp round trips, " + boost::lexical_cast<std::string>(calls) + " calls");
            for (int i=0; i<calls; ++i)
            {
                ret = Platform::OS::BsdSockets::send(clientSocket, msg.c_str(), msg.size(), 0);
                RCF_VERIFY(ret == buflen, std::runtime_error("send()"))(ret);
                bytesSent += ret;
                packetsSent += 1;

                ret = Platform::OS::BsdSockets::recv(clientSocket, &buffer[0], buffer.size(), 0);
                RCF_VERIFY(ret == buflen, std::runtime_error("recv()"))(ret);
                bytesReceived += ret;
                packetsReceived += 1;
            }
        }

        std::cout << "Packets sent: "       << packetsSent      << std::endl;
        std::cout << "Bytes sent: "         << bytesSent        << std::endl;
        std::cout << "Packets received: "   << packetsReceived  << std::endl;
        std::cout << "Bytes received: "     << bytesReceived    << std::endl;

        serverThread.join();

        Platform::OS::BsdSockets::closesocket(clientSocket);

        Platform::OS::BsdSockets::closesocket(acceptorSocket);
    }

} // namespace Test_Performance

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_Performance;

    util::CommandLineOption<std::string>    clScert("scert", RCF_TEMP_DIR "ssCert2.pem", "OpenSSL server certificate");
    util::CommandLineOption<std::string>    clSpwd("spwd", "mt2316", "OpenSSL server certificate password");
    util::CommandLineOption<std::string>    clCcert("ccert", RCF_TEMP_DIR "ssCert1.pem", "OpenSSL client certificate");
    util::CommandLineOption<std::string>    clCpwd("cpwd", "mt2316", "OpenSSL client certificate password");
    util::CommandLineOption<int>            clCalls( "calls", 1000, "number of calls");
    util::CommandLineOption<int>            clCallsLarge( "Calls", 1, "number of large message-size calls");
    util::CommandLineOption<int>            clProtocol("protocol", 1, "serialization protocol (1-4)");
    util::CommandLineOption<int>            clTest( "test", 0, "which test to run, 0 to run them all");
    util::CommandLineOption<int>            clLargeMessageSize("msgsize", 25, "large message size, in Mb");
    util::CommandLineOption<int>            clRawMessageSize("rawmsgsize", 50, "raw message size, in bytes, for non-RCF tcp and udp tests");
    util::CommandLine::getSingleton().parse(argc, argv);

    std::string clientCertificateFile           = clCcert;
    std::string clientCertificateFilePassword   = clCpwd;
    std::string serverCertificateFile           = clScert;
    std::string serverCertificateFilePassword   = clSpwd;

    int calls           = clCalls;
    int callsLarge      = clCallsLarge;
    int requestedTest   = clTest;
    int currentTest     = 0;

    if (requestedTest == 0)
    {
        // test all serialization protocols over a TCP transport
        for (int protocol=0; protocol<10; ++protocol)
        {
            if (RCF::isSerializationProtocolSupported(protocol))
            {
                RCF::TcpTransportFactory transportFactory;
                RCF::TransportPair transports = transportFactory.createTransports();
                RCF::ServerTransportPtr serverTransportPtr( transports.first );
                RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );
                
                std::string title = "TcpEndpoint transport: ";

                runPerformanceTest(
                    title,
                    clientTransportAutoPtr,
                    serverTransportPtr,
                    RCF::Twoway,
                    "",
                    protocol,
                    std::vector<RCF::FilterFactoryPtr>(),
                    std::vector<RCF::FilterPtr>(),
                    std::vector<RCF::FilterPtr>(),
                    calls);
            }
        }
    }

    // test tcp network performance, without RCF involvement
    ++currentTest;
    if (requestedTest == 0 || requestedTest == currentTest)
    {
        int buflen = clRawMessageSize;
        std::string title = "Test " + boost::lexical_cast<std::string>(currentTest) + ": ";
        testTcp(title, calls, buflen);
    }

    // test udp network performance, without RCF involvement
    ++currentTest;
    if (requestedTest == 0 || requestedTest == currentTest)
    {
        int buflen = clRawMessageSize;
        std::string title = "Test " + boost::lexical_cast<std::string>(currentTest) + ": ";
        testUdp(title, calls, buflen);
    }

    // test all transports, against the standard serialization protocol
    for (unsigned int i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        std::string s0 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        s0 = s0 + s0;
        s0 = s0 + s0;
        s0 = s0 + s0;

        int serializationProtocol = 1;

        ++currentTest;
        if (requestedTest == 0 || requestedTest == currentTest)
        {
            std::string title = "Test " + boost::lexical_cast<std::string>(currentTest) + ": ";
            runPerformanceTest(
                title,
                clientTransportAutoPtr,
                serverTransportPtr,
                RCF::Twoway,
                s0,
                serializationProtocol,
                std::vector<RCF::FilterFactoryPtr>(),//filterFactories,
                std::vector<RCF::FilterPtr>(),
                std::vector<RCF::FilterPtr>(),
                calls);
        }

    }

    std::string s0 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    s0 = s0 + s0;
    s0 = s0 + s0;
    s0 = s0 + s0;

    int serializationProtocol = clProtocol;

    // test compression and encryption filters, over TCP
    {
        std::vector<std::vector<RCF::FilterPtr> > payloadFilterChains;
        std::vector<RCF::FilterPtr> payloadFilterChain;

#ifdef RCF_USE_ZLIB
        payloadFilterChain.clear();
        payloadFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatefulCompressionFilter()) );
        payloadFilterChains.push_back(payloadFilterChain);

        payloadFilterChain.clear();
        payloadFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatelessCompressionFilter()) );
        payloadFilterChains.push_back(payloadFilterChain);
#endif

        std::vector<std::vector<RCF::FilterPtr> > transportFilterChains;
        std::vector<RCF::FilterPtr> transportFilterChain;

#ifdef RCF_USE_ZLIB
        transportFilterChain.clear();
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatefulCompressionFilter()) );
        transportFilterChains.push_back(transportFilterChain);

        transportFilterChain.clear();
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatelessCompressionFilter()) );
        transportFilterChains.push_back(transportFilterChain);
#endif

#ifdef RCF_USE_OPENSSL
        transportFilterChain.clear();
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)) );
        transportFilterChains.push_back(transportFilterChain);
#endif

#if defined(RCF_USE_ZLIB) && defined(RCF_USE_OPENSSL)
        transportFilterChain.clear();
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatelessCompressionFilter()) );
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)) );
        transportFilterChains.push_back(transportFilterChain);

        transportFilterChain.clear();
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatefulCompressionFilter()) );
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)) );
        transportFilterChains.push_back(transportFilterChain);
#endif

#ifdef BOOST_WINDOWS
        transportFilterChain.clear();
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::NtlmFilter()));
        transportFilterChains.push_back(transportFilterChain);
#endif

        std::vector<RCF::FilterFactoryPtr> filterFactories;

#ifdef RCF_USE_ZLIB
        filterFactories.push_back(RCF::FilterFactoryPtr( new RCF::ZlibStatelessCompressionFilterFactory() ));
        filterFactories.push_back(RCF::FilterFactoryPtr( new RCF::ZlibStatefulCompressionFilterFactory() ));
#endif

#ifdef RCF_USE_OPENSSL
        filterFactories.push_back(RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(serverCertificateFile, serverCertificateFilePassword) ));
#endif

#ifdef BOOST_WINDOWS
        filterFactories.push_back(RCF::FilterFactoryPtr( new RCF::NtlmFilterFactory()));
#endif

        // test payload filters
        for (unsigned int i=0; i<payloadFilterChains.size(); ++i)
        {
            RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
            RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
            RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

            ++currentTest;
            if (requestedTest == 0 || requestedTest == currentTest)
            {
                std::string title = "Test " + boost::lexical_cast<std::string>(currentTest) + ": ";
                runPerformanceTest(
                    title,
                    clientTransportAutoPtr,
                    serverTransportPtr,
                    RCF::Twoway,
                    s0,
                    serializationProtocol,
                    filterFactories,
                    payloadFilterChains[i],
                    std::vector<RCF::FilterPtr>(),
                    calls);
            }
        }

        // test transport filters
        for (unsigned int i=0; i<transportFilterChains.size(); ++i)
        {
            RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
            RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
            RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

            ++currentTest;
            if (requestedTest == 0 || requestedTest == currentTest)
            {
                std::string title = "Test " + boost::lexical_cast<std::string>(currentTest) + ": ";
                runPerformanceTest(
                    title,
                    clientTransportAutoPtr,
                    serverTransportPtr,
                    RCF::Twoway,
                    s0,
                    serializationProtocol,
                    filterFactories,
                    std::vector<RCF::FilterPtr>(),
                    transportFilterChains[i],
                    calls);
            }
        }
    }

    // test transmission of large messages
    {
        int len = 1024*1024*clLargeMessageSize;
        std::vector<char> buffer(len);
        memset(&buffer[0], 1, len);
        std::string s0(&buffer[0], len);

        ++currentTest;
        if (requestedTest == 0 || requestedTest == currentTest)
        {
            RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
            RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
            RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

            serverTransportPtr->setMaxMessageLength(-1);
            clientTransportAutoPtr->setMaxMessageLength(-1);

            unsigned int timeoutMs = 1000*3600;
            std::string title = "Test " + boost::lexical_cast<std::string>(currentTest) + ": ";
            runPerformanceTest(
                title,
                clientTransportAutoPtr,
                serverTransportPtr,
                RCF::Twoway,
                s0,
                serializationProtocol,
                std::vector<RCF::FilterFactoryPtr>(),
                std::vector<RCF::FilterPtr>(),
                std::vector<RCF::FilterPtr>(),
                callsLarge,
                timeoutMs,
                false);
        }
    }

    // test oneway invocations - tcp
    {
        RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
        RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

        ++currentTest;
        if (requestedTest == 0 || requestedTest == currentTest)
        {
            std::string title = "Test " + boost::lexical_cast<std::string>(currentTest) + ": ";
            runPerformanceTest(
                title,
                clientTransportAutoPtr,
                serverTransportPtr,
                RCF::Oneway,
                s0,
                serializationProtocol,
                std::vector<RCF::FilterFactoryPtr>(),
                std::vector<RCF::FilterPtr>(),
                std::vector<RCF::FilterPtr>(),
                calls);
        }
    }

    // test oneway invocations - udp
    {
        RCF::TransportPair transportPair = RCF::UdpTransportFactory().createTransports();
        RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

        ++currentTest;
        if (requestedTest == 0 || requestedTest == currentTest)
        {
            std::string title = "Test " + boost::lexical_cast<std::string>(currentTest) + ": ";
            runPerformanceTest(
                title,
                clientTransportAutoPtr,
                serverTransportPtr,
                RCF::Oneway,
                s0,
                serializationProtocol,
                std::vector<RCF::FilterFactoryPtr>(),
                std::vector<RCF::FilterPtr>(),
                std::vector<RCF::FilterPtr>(),
                calls);
        }
    }

    return boost::exit_success;
}



