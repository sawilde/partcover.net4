
#define __STDC_CONSTANT_MACROS

#include <string>

#include <boost/bind.hpp>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/ObjectFactoryService.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Platform/OS/Sleep.hpp>

#ifdef RCF_MULTI_THREADED
typedef RCF::Thread Thread;
#else
#include <RCF/RcfBoostThreads/RcfBoostThreads.hpp>
typedef RCF::RcfBoostThreads::boost::thread Thread;
#endif

namespace Test_CycleServer {

    class Echo
    {
    public:
        std::string echo(const std::string &s)
        {
            sLog = s;
            return s;
        }
        static std::string sLog;
    };

    std::string Echo::sLog;

    RCF_BEGIN(I_Echo, "I_Echo")
        RCF_METHOD_R1(std::string, echo, const std::string &)
    RCF_END(I_Echo)

    boost::shared_ptr<Thread> serverThread;

    void serverThreadTask(RCF::RcfServer &server)
    {
        while (server.cycle() == false);
    }
    void joinServerThread()
    {
        serverThread->join();
    }
} // namespace Test_CycleServer

int test_main(int argc, char **argv)
{
    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_CycleServer;

    util::CommandLine::getSingleton().parse(argc, argv);

    for (RCF::TransportFactories::size_type i=0; i<RCF::getTransportFactories().size(); ++i)
    {

        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );
        
        if (typeid(*serverTransportPtr) == typeid(RCF::UdpServerTransport))
        {
            // TODO: find out why shared_ptr cycle detection is flagging the UDP server transport
            continue;
        }

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        Echo echo;
        RCF::RcfServer server(serverTransportPtr);

        unsigned int numberOfTokens = 5;
        unsigned int objectTimeoutS = 2;
        RCF::ObjectFactoryServicePtr objectFactoryServicePtr( new RCF::ObjectFactoryService(numberOfTokens, objectTimeoutS) );
        objectFactoryServicePtr->bind( (I_Echo*) 0, (Echo**) 0);
        server.addService(objectFactoryServicePtr);

        server.bind( (I_Echo*) 0, echo);
        server.startSt();

        static const int Immediate = 0;
        static const int Infinite = -1;

        RcfClient<I_Echo> myClient(clientTransportAutoPtr);

        std::string s0;

        s0 = "bingo1";
        myClient.echo(RCF::Oneway, s0);
        Echo::sLog = "";
        while (Echo::sLog == "")
        {
            server.cycle(200); // max wait = 200 ms
        }
        RCF_CHECK(Echo::sLog == s0);

        s0 = "bingo2";
        myClient.echo(RCF::Oneway, s0);
        Echo::sLog = "";
        while (Echo::sLog == "")
        {
            server.cycle(Infinite); // max wait = infinite
        }
        RCF_CHECK(Echo::sLog == s0);

        s0 = "bingo3";
        myClient.echo(RCF::Oneway, s0);
        Echo::sLog = "";
        while (Echo::sLog == "")
        {
            server.cycle(Immediate); // max wait = immediate
        }
        RCF_CHECK(Echo::sLog == s0);

        // check that the object factory service cleanup task is running properly
        {
            serverThread.reset( new Thread( boost::bind(serverThreadTask, boost::ref(server)) ) );
            server.addJoinFunctor( boost::bind(joinServerThread));

            for (unsigned int i=0; i<numberOfTokens; ++i)
            {
                //bool ok = RCF::createRemoteObject<I_Echo>(myClient);
                bool ok = tryCreateRemoteObject<I_Echo>(myClient);
                RCF_CHECK(ok);
            }

            //bool ok = RCF::createRemoteObject<I_Echo>(myClient);
            bool ok = tryCreateRemoteObject<I_Echo>(myClient);
            RCF_CHECK(!ok);

            Platform::OS::Sleep(5);

            for (unsigned int i=0; i<numberOfTokens; ++i)
            {
                bool ok = tryCreateRemoteObject<I_Echo>(myClient);
                RCF_CHECK(ok);
            }

            server.stop();
        }

    }

    return boost::exit_success;
}
