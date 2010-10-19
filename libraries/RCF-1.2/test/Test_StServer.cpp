
#define __STDC_CONSTANT_MACROS

#include <iostream>
#include <memory>
#include <string>

//#include <boost/thread/thread.hpp>
#include <RCF/RcfBoostThreads/RcfBoostThreads.hpp>

#include <RCF/test/TestMinimal.hpp>

#ifndef RCF_SINGLE_THREADED
#error This test must be built with RCF in single-threaded mode, i.e. RCF_SINGLE_THREADED must be defined.
#endif

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>

#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Platform/OS/Sleep.hpp>

namespace Test_SingleThreadedServer {

    RCF_BEGIN(I_X, "I_X")
        RCF_METHOD_R1(std::string, echo, std::string)
        RCF_METHOD_V0(void, stopServer)
    RCF_END(I_X)

    volatile bool gServerThreadRunning = false;

    volatile bool gServerStopped = false;

    boost::shared_ptr<RCF::RcfBoostThreads::boost::thread> gServerThreadPtr;

    std::auto_ptr<RCF::RcfServer> gServerAutoPtr;

    void joinServerThread()
    {
        gServerThreadPtr->join();
    }

    void serverThreadFunc()
    {
        gServerAutoPtr->startInThisThread( boost::bind(joinServerThread) );
        gServerThreadRunning = false;
    }

    struct X
    {
        std::string echo(const std::string &s)
        {
            return s;
        }

        void stopServer()
        {
            // Call RcfServer::stop() on a separate thread.
            gServerStopped = false;
            RCF::RcfBoostThreads::boost::thread t(
                boost::bind(&X::stopServerImpl, this));
        }

        void stopServerImpl()
        {
            gServerAutoPtr->stop();
            gServerStopped = true;
        }
    };


    template<typename T, typename U>
    class A
    {
    public:
        void onServerStarted(RCF::RcfServer &server)
        {
            gServerThreadRunning = true;
        }
    };

    void onServerStarted(RCF::RcfServer &server)
    {
        gServerThreadRunning = true;
    }

} // namespace Test_SingleThreadedServer

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_SingleThreadedServer;

    util::CommandLine::getSingleton().parse(argc, argv);

    for (unsigned int i=0; i<RCF::getTransportFactories().size(); ++i)
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

        X x;
        gServerAutoPtr.reset(new RCF::RcfServer(serverTransportPtr));
        gServerAutoPtr->bind( (I_X*) 0, x);

        // callback to a standalone function...
        gServerAutoPtr->setStartCallback(
            boost::bind(Test_SingleThreadedServer::onServerStarted, _1) );
        
        // or to a member function
        A<int,int> a;
        gServerAutoPtr->setStartCallback(
            boost::bind(&A<int,int>::onServerStarted, a, _1));

        RcfClient<I_X> client(clientTransportAutoPtr->clone());
        std::string s;

        gServerThreadRunning = false;
        gServerThreadPtr.reset( new RCF::RcfBoostThreads::boost::thread(&serverThreadFunc) );
        while (!gServerThreadRunning);

        s = client.echo("abc");
        RCF_CHECK(s == "abc");
        RCF_CHECK(gServerThreadRunning == true);

        gServerAutoPtr->stop();

        gServerThreadRunning = false;
        gServerThreadPtr.reset( new RCF::RcfBoostThreads::boost::thread(&serverThreadFunc) );
        while (!gServerThreadRunning);

        Platform::OS::Sleep(1);

        s = client.echo("abc");
        RCF_CHECK(s == "abc");
        //RCF_CHECK(serverStarted == true); // fails on Win32 because the system seems to reuse the old thread

        gServerStopped = false;
        client.stopServer(RCF::Oneway); // remotely stopping the server
        while (!gServerStopped); // wait until server thread exits

        gServerAutoPtr.reset();
    }
    return 0;
}
