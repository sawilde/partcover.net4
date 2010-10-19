
#include <RCF/test/TestMinimal.hpp>

#include <string>
#include <utility>

#include <SF/utility.hpp> // serialization for std::pair

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/CurrentSession.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/CommandLine.hpp>

namespace Test_ClientInfo {

    class GetMyAddress
    {
    public:
        std::pair<std::string, int> getMyAddress()
        {
            const RCF::I_RemoteAddress &address = RCF::getCurrentRcfSession().getRemoteAddress();
            const RCF::IpAddress &ipAddress = dynamic_cast<const RCF::IpAddress &>(address);
            return std::make_pair(ipAddress.getIp(), ipAddress.getPort());
        }

        void checkServerHandle()
        {
            RCF::I_Proactor & proactor = RCF::getCurrentRcfSession().getProactor();

#ifdef BOOST_WINDOWS

            // TcpIocpServerTransport
            RCF::TcpIocpProactor *tcpProactor = 
                dynamic_cast<RCF::TcpIocpProactor *>(&proactor);

            if (tcpProactor)
            {
                int serverHandle = tcpProactor->getNativeHandle();
                RCF_CHECK(serverHandle != 0 && serverHandle != -1);
                return;
            }

            // Win32NamedPipeServerTransport
            RCF::Win32NamedPipeProactor * win32PipeProactor = 
                dynamic_cast<RCF::Win32NamedPipeProactor *>(&proactor);

            if (win32PipeProactor)
            {
                HANDLE serverHandle = win32PipeProactor->getNativeHandle();
                RCF_CHECK(serverHandle != 0);
                return;
            }

#endif

#ifdef RCF_USE_BOOST_ASIO

            // AsioServerTransport
            RCF::AsioProactor * asioProactor = 
                dynamic_cast<RCF::AsioProactor *>(&proactor);

            if (asioProactor)
            {
                int serverHandle = asioProactor->getNativeHandle();
                RCF_CHECK(serverHandle != 0 && serverHandle != -1);
                return;
            }

#endif

#if defined(RCF_USE_BOOST_ASIO) && defined (BOOST_ASIO_HAS_LOCAL_SOCKETS)

            // UnixLocalServerTransport
            RCF::AsioProactor * unixLocalProactor = 
                dynamic_cast<RCF::AsioProactor *>(&proactor);

            if (unixLocalProactor)
            {
                int serverHandle = unixLocalProactor->getNativeHandle();
                RCF_CHECK(serverHandle != 0 && serverHandle != -1);
                return;
            }

#endif

            // UdpServerTransport
            RCF::UdpProactor * udpProactor = 
                dynamic_cast<RCF::UdpProactor *>(&proactor);

            if (udpProactor)
            {
                int serverHandle = udpProactor->getNativeHandle();
                RCF_CHECK(serverHandle != 0 && serverHandle != -1);
                return;
            }

            RCF_CHECK(1 == 0);
        }
    };

    typedef std::pair<std::string, int> StringIntPair;

} // namespace Test_ClientInfo

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_ClientInfo::StringIntPair)

namespace Test_ClientInfo {

    RCF_BEGIN(I_GetMyAddress, "I_GetMyAddress")
        RCF_METHOD_R0(StringIntPair, getMyAddress)
        RCF_METHOD_V0(void, checkServerHandle)
    RCF_END(I_GetMyAddress)

} // namespace Test_ClientInfo

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_ClientInfo;

    util::CommandLine::getSingleton().parse(argc, argv);

    for (int i=0; i<RCF::getIpTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getIpTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        GetMyAddress getMyAddress;
        RCF::RcfServer server(serverTransportPtr);
        server.bind( (I_GetMyAddress*) 0, getMyAddress);
        server.start();

#if defined(_MSC_VER) && _MSC_VER < 1310
        std::pair<std::string, int> myAddress = RcfClient<I_GetMyAddress>(clientTransportAutoPtr).getMyAddress(RCF::Twoway).get();
#else
        std::pair<std::string, int> myAddress = RcfClient<I_GetMyAddress>(clientTransportAutoPtr).getMyAddress(RCF::Twoway);
#endif

        RCF_CHECK(myAddress.first == "127.0.0.1");
        RCF_CHECK(myAddress.second > 0);
    }

    // Check that we can get native handles for the underlying I/O objects.
    for (std::size_t i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        GetMyAddress getMyAddress;
        RCF::RcfServer server(serverTransportPtr);
        server.bind( (I_GetMyAddress*) 0, getMyAddress);
        server.start();


        RcfClient<I_GetMyAddress> client(clientTransportAutoPtr);

        // Check that we can get server side native handles.
        client.checkServerHandle();


        // Check that we can get client side native handles.
        RCF::TcpClientTransport *pTcpClientTransport = 
            dynamic_cast<RCF::TcpClientTransport *>( 
                & client.getClientStub().getTransport() );

        if (pTcpClientTransport)
        {
            int clientHandle = pTcpClientTransport->getNativeHandle();
            RCF_CHECK(clientHandle != 0 && clientHandle != -1);
            continue;
        }

        RCF::UdpClientTransport *pUdpClientTransport = 
            dynamic_cast<RCF::UdpClientTransport *>( 
                & client.getClientStub().getTransport() );

        if (pUdpClientTransport)
        {
            int clientHandle = pUdpClientTransport->getNativeHandle();
            RCF_CHECK(clientHandle != 0 && clientHandle != -1);
            continue;
        }

#ifdef BOOST_WINDOWS

        RCF::Win32NamedPipeClientTransport *pWin32PipeClientTransport = 
            dynamic_cast<RCF::Win32NamedPipeClientTransport *>( 
                & client.getClientStub().getTransport() );

        if (pWin32PipeClientTransport)
        {
            HANDLE clientHandle = pWin32PipeClientTransport->getNativeHandle();
            RCF_CHECK(clientHandle != 0);
            continue;
        }

#endif

#if defined(RCF_USE_BOOST_ASIO) && defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

        RCF::UnixLocalClientTransport *pUnixLocalClientTransport = 
            dynamic_cast<RCF::UnixLocalClientTransport *>( 
                & client.getClientStub().getTransport() );

        if (pUnixLocalClientTransport)
        {
            int clientHandle = pUnixLocalClientTransport->getNativeHandle();
            RCF_CHECK(clientHandle != 0 && clientHandle != -1);
            continue;
        }


#endif

        RCF_CHECK(1 == 0);

    }
   
    return boost::exit_success;
}






