
#include <string>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/test/ThreadGroup.hpp>
#include <RCF/util/CommandLine.hpp>

namespace Test_MultiTransportServer {

    class Echo
    {
    public:
        std::string echo(const std::string &s)
        {
            return s;
        }
    };

    RCF_BEGIN(I_Echo, "I_Echo")
        RCF_METHOD_R1(std::string, echo, const std::string &)
    RCF_END(I_Echo)

    RCF::Mutex gIoMutex;

    void clientTask(
        const RCF::I_ClientTransport &clientTransport,
        unsigned int calls,
        std::string s0)
    {
        RcfClient<I_Echo> echo( clientTransport.clone());
        for (unsigned int i=0; i<calls; ++i)
        {
            std::string s = echo.echo(s0);
            RCF_CHECK(s == s0);
            if (s != s0)
            {
                std::cout << "----------------------" << std::endl;
                std::cout << s << std::endl;
                std::cout << s0 << std::endl;
                std::cout << typeid(clientTransport).name() << std::endl;
                std::cout << i << std::endl;
                std::cout << calls << std::endl;
                std::cout << "----------------------" << std::endl;
            }
        }
    }

} // namespace Test_MultiTransportServer

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_MultiTransportServer;

    util::CommandLine::getSingleton().parse(argc, argv);

    std::size_t count = 3;

    // generate a bunch of corresponding server/client transport pairs
    std::vector<RCF::ServerTransportPtr> serverTransports;
    std::vector<RCF::ClientTransportAutoPtrPtr> clientTransports;
    for (unsigned int i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        for (int j=0; j<count; ++j)
        {
            RCF::TransportPair transports = transportFactoryPtr->createTransports();
            serverTransports.push_back(transports.first);
            clientTransports.push_back(transports.second);

            RCF::writeTransportTypes(std::cout, *transports.first, **transports.second);
        }
    }

    std::string s0 = "something to bounce off the server";
    Echo echo;

    {
        RCF::RcfServer server(serverTransports);
        server.bind( (I_Echo *) 0, echo);
        server.start();
        server.stop();
        server.start();
        for(unsigned int i=0; i<clientTransports.size(); ++i)
        {
            std::string s = RcfClient<I_Echo>( (*clientTransports[i])->clone() ).echo(s0);
            RCF_CHECK(s==s0);
        }
        server.stop();
    }

    {
        std::vector<RCF::ServicePtr> services;
        for (unsigned int i=0; i<serverTransports.size(); ++i)
        {
            services.push_back( boost::dynamic_pointer_cast<RCF::I_Service>(serverTransports[i]) );
        }
        RCF::RcfServer server(services);
        server.bind( (I_Echo*) 0, echo);
        server.start();
        server.stop();
        server.start();
        for(unsigned int i=0; i<clientTransports.size(); ++i)
        {
            std::string s = RcfClient<I_Echo>( (*clientTransports[i])->clone() ).echo(s0);
            RCF_CHECK(s==s0);
        }
        server.stop();
    }

    {
        RCF::RcfServer server;
        for (unsigned int i=0; i<serverTransports.size(); ++i)
        {
            server.addService( boost::dynamic_pointer_cast<RCF::I_Service>(serverTransports[i]));
        }
        server.bind( (I_Echo*) 0, echo);
        server.start();
        server.stop();
        server.start();
        for(unsigned int i=0; i<clientTransports.size(); ++i)
        {
            std::string s = RcfClient<I_Echo>( (*clientTransports[i])->clone() ).echo(s0);
            RCF_CHECK(s==s0);
        }
        server.stop();
    }

    {
        RCF::RcfServer server;
        for (unsigned int i=0; i<serverTransports.size(); ++i)
        {
            server.addServerTransport(serverTransports[i]);
        }
        server.bind( (I_Echo*) 0, echo);
        server.start();
        server.stop();
        server.start();
        for(unsigned int i=0; i<clientTransports.size(); ++i)
        {
            std::string s = RcfClient<I_Echo>( (*clientTransports[i])->clone() ).echo(s0);
            RCF_CHECK(s==s0);
        }
        server.stop();
    }

    {
        RCF::RcfServer server(serverTransports);
        server.bind( (I_Echo*) 0, echo);
        server.start();
        server.stop();
        server.start();

        int threadsPerClientTransport = 3;
        int callsPerClientThread = 50;
       
        ThreadGroup threadGroup;
        for (std::size_t i=0; i<clientTransports.size(); ++i)
        {
            for (std::size_t j=0; j<threadsPerClientTransport ; ++j)
            {
                threadGroup.push_back( RCF::ThreadPtr( new RCF::Thread(
                    boost::bind(
                        clientTask,
                        boost::ref(**clientTransports[i]),
                        callsPerClientThread,
                        s0))));
            }
        }
        joinThreadGroup(threadGroup);

        server.stop();
    }

    return boost::exit_success;
}






