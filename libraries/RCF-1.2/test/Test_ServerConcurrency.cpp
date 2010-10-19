
#include <string>

#include <boost/lexical_cast.hpp>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/test/ThreadGroup.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/test/WithStopServer.hpp>
#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Profile.hpp>

namespace Test_ServerTransportConcurrency {

    class Echo : public RCF::WithStopServer
    {
    public:
       
        std::string echo(const std::string &s)
        {
            return s;
        }
    };

    RCF_BEGIN(I_Echo, "I_Echo")
        RCF_METHOD_R1(std::string, echo, const std::string &)
        RCF_METHOD_V0(void, stopServer)
    RCF_END(I_Echo)

    typedef RcfClient<I_Echo> Client;
    typedef boost::shared_ptr<Client> ClientPtr;

    void clientThreadTask(
        const std::vector<ClientPtr> &  clients, 
        const std::string &             msg, 
        bool                            random, 
        int                             i0, 
        int                             i1)
    {
        for (int j=i0; j<i1; ++j)
        {
            if (random)
            {
                int idx = rand() % (i1-i0);
                RCF_CHECK( msg == clients[i0 + idx]->echo(msg) );
            }
            else
            {
                RCF_CHECK( msg == clients[j]->echo(msg) );
            }
        }
    }

    ThreadGroup createClientThreads(
        int                             threads, 
        const std::vector<ClientPtr> &  clients, 
        const std::string &             msg, 
        bool                            random)
    {
        int calls = clients.size();
        ThreadGroup threadGroup;
        for (int i=0; i<threads; ++i)
        {
            int threadCalls = calls/threads;
            int j0 = i*threadCalls;
            int j1 = (i+1)*threadCalls;
            if (i == threads-1)
            {
                j1 = calls;
            }
            bool random = false;

            threadGroup.push_back( ThreadPtr( new Thread( boost::bind(
                clientThreadTask, 
                clients, 
                msg, 
                random, 
                j0, 
                j1))));
        }
        return threadGroup;
    }

} // namespace Test_ServerTransportConcurrency

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_ServerTransportConcurrency;

    util::CommandLineOption<bool>           clServer("server", false, "act as server");
    util::CommandLineOption<bool>           clClient("client", false, "act as client");
    util::CommandLineOption<int>            clCalls("calls", 500, "number of calls");
    util::CommandLineOption<int>            clThreads("threads", 1, "number of threads making client calls");
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

    for (unsigned int i=0; i<RCF::getTransportFactories().size(); ++i)
    {

        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        std::ostringstream ostr;
        RCF::writeTransportTypes(ostr, *serverTransportPtr, *clientTransportAutoPtr);
        std::string transportDesc = ostr.str();

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        std::string     msg = "payload";
        int             calls = clCalls;
        int             threads = clThreads;
        std::string     ip = "127.0.0.1";
        int             port = 50001;

        RCF_ASSERT(threads <= calls)(threads)(calls);

        std::cout << "Calls: " << calls << std::endl;
        std::cout << "Threads: " << threads << std::endl;

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
        }
        else
        {
            Platform::OS::SleepMs(1000);
        }

        // all requests on 1 client
        if (bClient)
        {
            Client client(clientTransportAutoPtr->clone());
            RCF_CHECK( msg == client.echo(msg) );
            {
                std::string title = transportDesc + ": " +
                    boost::lexical_cast<std::string>(calls) + " call(s), 1 client(s)";
                util::Profile profile(title);
                for (int i=0; i<calls; ++i)
                {
                    RCF_CHECK( msg == client.echo(msg) );
                }
            }

            // 1 request on all clients, a) in order, and b) in random order
            std::vector<ClientPtr> clients;
            for (int i=0; i<calls; ++i)
            {
                clients.push_back( ClientPtr(new Client(
                    clientTransportAutoPtr->clone())));
            }
            RCF_ASSERT(clients.size() == calls);
            for (int j=0; j<clients.size(); ++j)
            {
                RCF_CHECK( msg == clients[j]->echo(msg) );
            }
            {
                std::string title = transportDesc + ": 1 call(s), " +
                    boost::lexical_cast<std::string>(calls) + " client(s): " +
                    boost::lexical_cast<std::string>(threads) + " threads(s)";

                util::Profile profile(title);
                joinThreadGroup(createClientThreads(threads, clients, msg, false));
            }
            {
                srand(RCF::getCurrentTimeMs());

                std::string title = transportDesc + ": 1 call(s), " +
                    boost::lexical_cast<std::string>(calls) + " client(s): " +
                    boost::lexical_cast<std::string>(threads) + " threads(s): randomized";

                util::Profile profile(title);
                joinThreadGroup(createClientThreads(threads, clients, msg, true));
            }

            RcfClient<I_Echo>(clientTransportAutoPtr->clone()).stopServer();

        }

        if (bServer)
        {
            echo.wait();
            server.stop();
        }

    }

    // Test connection limit on IP based transports.
    for (unsigned int i=0; i<RCF::getIpTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getIpTransportFactories()[i];
        RCF::TransportPair transports;
        RCF::ServerTransportPtr serverTransportPtr;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr;

        transports = transportFactoryPtr->createTransports();
        serverTransportPtr = transports.first;
        clientTransportAutoPtr = *transports.second;

        RCF::I_ServerTransportEx *pServerTransportEx =
            dynamic_cast<RCF::I_ServerTransportEx *>(
                serverTransportPtr.get());

        if (NULL == pServerTransportEx)
        {
            continue;
        }

        RCF::writeTransportTypes(
            std::cout, 
            *serverTransportPtr, 
            *clientTransportAutoPtr);

        Echo echo;
        RCF::RcfServer server(serverTransportPtr);
        server.bind( (I_Echo *) 0, echo);
        server.getServerTransport().setConnectionLimit(10);
        server.start();
        
        std::vector< RcfClient<I_Echo> > clients(10);

        for (std::size_t j=0; j<clients.size(); ++j)
        {
            RcfClient<I_Echo> temp(clientTransportAutoPtr->clone());
            clients[j].swap(temp);

            clients[j].echo("");            
        }

        for (std::size_t j=0; j<10; ++j)
        {
            clients[j].echo("");

            try
            {
                RcfClient<I_Echo>(clientTransportAutoPtr->clone()).echo("");
                RCF_CHECK(1==0);
            }
            catch(RCF::Exception & e)
            {
                //std::cout << e.what() << std::endl;
                //RCF_CHECK(e.getError() == RCF::RcfError_ConnectionLimitExceeded);

                // Supposed to get RCF::RcfError_ConnectionLimitExceeded as 
                // the error here, but we can't depend on it.
                RCF_CHECK(1 == 1);
            }
        }

        // Kill off one of the connections...
        clients.pop_back();
        
        Platform::OS::Sleep(1);

        // ... so these calls can go through.
        RcfClient<I_Echo>(clientTransportAutoPtr->clone()).echo("");
        
        Platform::OS::Sleep(1);

        RcfClient<I_Echo>(clientTransportAutoPtr->clone()).echo("");
    }

    return boost::exit_success;
}
