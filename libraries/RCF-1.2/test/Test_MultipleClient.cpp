
#include <string>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/test/ThreadGroup.hpp>
#include <RCF/util/CommandLine.hpp>

namespace Test_MultipleClient {

    class Echo
    {
    public:
        std::string echo(const std::string &s) { return s; }
    };

    RCF_BEGIN(I_Echo, "I_Echo")
        RCF_METHOD_R1(std::string, echo, const std::string &)
    RCF_END(I_Echo)

    void clientThread(const RCF::I_ClientTransport &clientTransport)
    {
        try
        {
            RcfClient<I_Echo> echo(clientTransport.clone());
            std::string s0 = "something special";
            for (int i=0;i<10; i++)
            {
                try
                {
                    std::string s = echo.echo(s0);
                    RCF_CHECK(s0 == s);
                }
                catch(const RCF::Exception &e)
                {
                    RCF_TRACE("")(e); // since we're overloading the server there will probably be some exceptions
                }
            }
        }
        catch(const std::exception & e)
        {
            RCF_CHECK(1==0);
        }
    }

} // namespace Test_MultipleClient

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_MultipleClient;

    util::CommandLine::getSingleton().parse(argc, argv);

    for (unsigned int i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        Echo echo;
        RCF::RcfServer server(serverTransportPtr);
        server.bind( (I_Echo*) 0, echo);
        server.start();

        ThreadGroup clients;
        for (int i=0; i<100; ++i)
        {
            clients.push_back( ThreadPtr(new Thread(
                boost::bind(&clientThread, boost::ref(*clientTransportAutoPtr)))));
        }
        joinThreadGroup(clients);
    }
   
    return boost::exit_success;
}





