
#include <iostream>
#include <string>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/test/WithStopServer.hpp>
#include <RCF/util/CommandLine.hpp>

#include <SF/wstring.hpp>

class Echo : public RCF::WithStopServer
{
public:

    std::string echo(const std::string &s)
    {
        std::cout << "Echoing: " << s << std::endl;
        return s;
    }

    std::wstring echo(const std::wstring &s)
    {
        return s;
    }
};

RCF_BEGIN(I_Echo, "I_Echo")
    RCF_METHOD_R1(std::string, echo, const std::string &)
    RCF_METHOD_R1(std::wstring, echo, const std::wstring &)
    RCF_METHOD_V0(void, stopServer)
RCF_END(I_Echo)


void myCallback(
    std::size_t bytesTransferred,
    std::size_t bytesTotal,
    RCF::ClientProgress::Trigger trigger,
    RCF::ClientProgress::Activity activity,
    RCF::ClientProgress::Action &action)
{
    action = RCF::ClientProgress::Continue;
}

int test_main(int argc, char **argv)
{
    RCF::RcfInitDeinit rcfInitDeinit;

    util::CommandLineOption<bool>           clServer("server", false, "act as server");
    util::CommandLineOption<bool>           clClient("client", false, "act as client");
    util::CommandLine::getSingleton().parse(argc, argv);

    bool bServer = true;
    bool bClient = true;
    if (clServer && clClient)
    {
        bServer = true;
        bClient = true;
    }
    else if (clServer)
    {
        bServer = true;
        bClient = false;
    }
    else if (clClient)
    {
        bServer = false;
        bClient = true;
    }

    RCF::TransportFactoryPtr transportFactoryPtr( new RCF::TcpIocpTransportFactory );
    RCF::TransportPair transports = transportFactoryPtr->createTransports();
    RCF::ServerTransportPtr serverTransportPtr( transports.first );
    RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

    serverTransportPtr->setMaxMessageLength(-1);
    clientTransportAutoPtr->setMaxMessageLength(-1);

    Echo echo;
    RCF::RcfServer server(serverTransportPtr);
    server.bind( (I_Echo*) 0, echo);
    if (bServer)
    {
        if (!bClient)
        {
            RCF::I_IpServerTransport &ipServerTransport = dynamic_cast<RCF::I_IpServerTransport &>(*serverTransportPtr);
            ipServerTransport.setNetworkInterface("0.0.0.0");
        }
        server.start();
        RCF_CHECK( RcfClient<I_Echo>(clientTransportAutoPtr->clone()).echo("asdf") == "asdf");
    }
    if (bClient)
    {
        RcfClient<I_Echo> client(clientTransportAutoPtr->clone());
        client.getClientStub().setRemoteCallTimeoutMs(1000*60*60);

        std::string line;
        while (line != "quit")
        {
            std::cout << "Input string to echo (\"quit\" to quit): ";
            std::getline(std::cin, line);
            std::cout << "Echo: " << client.echo(line).get() << std::endl;
        }

        RcfClient<I_Echo>(clientTransportAutoPtr->clone()).stopServer();
    }
    if (bServer)
    {
        echo.wait();
        server.stop();
    }

    return boost::exit_success;
}
