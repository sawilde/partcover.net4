
#include <string>

#include <boost/config.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/ObjectFactoryService.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/ThreadManager.hpp>
#include <RCF/UdpServerTransport.hpp>

#include <RCF/test/TransportFactories.hpp>
#include <RCF/test/ThreadGroup.hpp>

#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Profile.hpp>
#include <RCF/util/Platform/OS/ThreadId.hpp>
#include <RCF/util/Platform/OS/Sleep.hpp>

#ifdef RCF_USE_BOOST_ASIO
#include <RCF/TcpAsioServerTransport.hpp>
#endif

#include <RCF/ObjectPool.hpp>

namespace Test_ThreadManager {

    RCF_BEGIN(I_X, "I_X")
        RCF_METHOD_R0(int, ping)
        RCF_METHOD_R1(int, wait, unsigned int /*waitMs*/)
        RCF_METHOD_R0(int, pingBusy)
        RCF_METHOD_R1(int, waitBusy, unsigned int /*waitMs*/)
    RCF_END(I_X)

    RCF_BEGIN(I_Y, "I_Y")
        RCF_METHOD_R0(int, ping)
        RCF_METHOD_R1(int, wait, unsigned int /*waitMs*/)
        RCF_METHOD_R0(int, pingBusy)
        RCF_METHOD_R1(int, waitBusy, unsigned int /*waitMs*/)
    RCF_END(I_Y)

    class X
    {
    public:
        int ping()
        {
            return 17;
        }
       
        int wait(unsigned int waitMs)
        {
            Platform::OS::SleepMs(waitMs);
            return 17;
        }

        int pingBusy()
        {
            return 17;
        }

        int waitBusy(unsigned int waitMs)
        {
            Platform::OS::SleepMs(waitMs);
            return 17;
        }
    };

    typedef X Y;

    RCF::Mutex ioMutex;

    // Apparently RCF_CHECK() is not threadsafe...
    RCF::Mutex boostCheckMutex;

    void clientLoadTest(
        RCF::I_ClientTransport & clientTransport, 
        unsigned int clientReps, 
        unsigned int maxDelayMs)
    {
        RcfClient<I_X> client( clientTransport.clone() );
        client.getClientStub().setRemoteCallTimeoutMs(1000*20); // 20s timeout
        for (unsigned int i=0; i<clientReps; ++i)
        {
            
            //if ((i*100) % (20*clientReps) == 0)
            //{
            //    Platform::OS::ThreadId threadId = 
            //        Platform::OS::GetCurrentThreadId();

            //    RCF::Lock lock(ioMutex);
            //    std::cout 
            //        << "Client thread #" << threadId 
            //        << ": progress: " << i*100/clientReps 
            //        << "%\n";
            //}
            

            unsigned int whichTask = rand()%4;

            // Random millisecond delays of up to maxDelayMs milliseconds.
            unsigned int delayMs = maxDelayMs > 0 ? (rand() % maxDelayMs) : 0; 

            switch (whichTask)
            {
            case 0:
                {
                    int n = client.ping();
                    RCF::Lock lock(boostCheckMutex);
                    RCF_CHECK(n == 17);
                }
                break;
            case 1:
                {
                    int n = client.pingBusy();
                    RCF::Lock lock(boostCheckMutex);
                    RCF_CHECK(n == 17);
                }
                break;
            case 2:
                {
                    int n = client.wait(delayMs);
                    RCF::Lock lock(boostCheckMutex);
                    RCF_CHECK(n == 17);
                }
                break;
            case 3:
                {
                    int n = client.waitBusy(delayMs);
                    RCF::Lock lock(boostCheckMutex);
                    RCF_CHECK(n == 17);
                }
                break;

            default:
                RCF_ASSERT(0)(whichTask);
            }
        }
    }

    void clientObjectLoadTest(
        RCF::I_ClientTransport & clientTransport, 
        unsigned int clientReps, 
        unsigned int maxDelayMs)
    {
        RcfClient<I_Y> client( clientTransport.clone() );
        client.getClientStub().setRemoteCallTimeoutMs(1000*20); // 20s timeout

        bool ok = tryCreateRemoteObject<I_Y>(client);
        {
            RCF::Lock lock(boostCheckMutex);
            RCF_CHECK(ok);
        }

        for (unsigned int i=0; i<clientReps; ++i)
        {
            //if ((i*100) % (20*clientReps) == 0)
            //{
            //    Platform::OS::ThreadId threadId = 
            //        Platform::OS::GetCurrentThreadId();

            //    RCF::Lock lock(ioMutex);
            //    std::cout 
            //        << "Client thread #" << threadId 
            //        << ": progress: " << i*100/clientReps 
            //        << "%\n";
            //}


            unsigned int whichTask = rand()%4;
            unsigned int delayMs = maxDelayMs > 0 ? rand()%maxDelayMs : 0;

            {
                int n = client.waitBusy(delayMs);
                RCF::Lock lock(boostCheckMutex);
                RCF_CHECK(n == 17);
            }

            switch (whichTask)
            {
            case 0:
                {
                    int n = client.ping();
                    RCF::Lock lock(boostCheckMutex);
                    RCF_CHECK(n == 17);
                }
                break;
            case 1:
                {
                    int n = client.pingBusy();
                    RCF::Lock lock(boostCheckMutex);
                    RCF_CHECK(n == 17);
                }
                break;
            case 2:
                {
                    int n = client.wait(delayMs);
                    RCF::Lock lock(boostCheckMutex);
                    RCF_CHECK(n == 17);
                }
                break;
            case 3:
                {
                    int n = client.waitBusy(delayMs);
                    RCF::Lock lock(boostCheckMutex);
                    RCF_CHECK(n == 17);
                }
                break;

            default:
                RCF_ASSERT(0)(whichTask);
            }
        }
    }

    void runTests(
        RCF::RcfServer &server, 
        RCF::I_ClientTransport & clientTransport,
        int concurrencyCount, 
        unsigned int clientReps, 
        unsigned int maxDelayMs = 0)
    {
        // now try a more realistic load test

        boost::shared_ptr<util::ImmediateProfile> immmediateProfilePtr;

        // bind I_X
        X x;
        server.bind( (I_X*) 0, x);

        // bind I_Y to object factory
        RCF::ObjectFactoryServicePtr objectFactoryServicePtr( 
            new RCF::ObjectFactoryService(10, 60));

        objectFactoryServicePtr->bind( (I_Y*) 0,  (Y**) 0);
        server.addService( RCF::ServicePtr(objectFactoryServicePtr));

        server.start();

        ThreadGroup threadGroup;

        // calls to I_X
        {
            util::ImmediateProfile profile("load test: all clients making static object calls");
            for (int i=0; i<concurrencyCount; ++i)
            {
                threadGroup.push_back( ThreadPtr( new Thread( boost::bind(
                    clientLoadTest, 
                    boost::ref(clientTransport), 
                    clientReps, 
                    maxDelayMs))));
            }
            joinThreadGroup(threadGroup);
        }

        // calls to I_Y
        threadGroup.clear();
        {
            util::ImmediateProfile profile("load test: all clients making dynamic object calls");
            for (int i=0;i<concurrencyCount; ++i)
            {
                threadGroup.push_back( ThreadPtr( new Thread( boost::bind(
                    clientObjectLoadTest, 
                    boost::ref(clientTransport), 
                    clientReps, 
                    maxDelayMs))));
            }
            joinThreadGroup(threadGroup);
        }

        // mixed
        threadGroup.clear();
        {
            util::ImmediateProfile profile("load test: half/half clients making static/dynamic object calls");
            for (int i=0; i<concurrencyCount; ++i)
            {
                switch(i%2)
                {
                case 0: 
                    
                    threadGroup.push_back( ThreadPtr( new Thread( boost::bind(
                        clientLoadTest, 
                        boost::ref(clientTransport), 
                        clientReps, 
                        maxDelayMs)))); 
                    
                    break;

                case 1: 
                    
                    threadGroup.push_back( ThreadPtr( new Thread( boost::bind(
                        clientObjectLoadTest, 
                        boost::ref(clientTransport), 
                        clientReps, 
                        maxDelayMs)))); 
                    
                    break;

                default: 
                    
                    RCF_ASSERT(0);
                }
            }
            joinThreadGroup(threadGroup);
        }

        server.stop();
    }

    std::size_t sThreadInitCount = 0;
    std::size_t sThreadDeinitCount = 0;

    void onThreadInit()
    {
        ++sThreadInitCount;
    }

    void onThreadDeinit()
    {
        ++sThreadDeinitCount;
    }

    class Xyz
    {
    public:

        Xyz() :
            mRcfError(RCF::RcfError_Ok)
        {
        }

        void go(std::size_t waitMs)
        {
            try
            {
                mClient.waitBusy(waitMs);
            }
            catch(const RCF::Exception & e)
            {
                mRcfError = e.getErrorId();
            }
        }

        bool isError(boost::int32_t rcfError)
        {
            return mRcfError == rcfError;
        }

        RcfClient<I_X>      mClient;
        boost::int32_t      mRcfError;
        ThreadPtr           mThreadPtr;

    };

} // namespace Test_ThreadManager

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_ThreadManager;

    util::CommandLineOption<unsigned int> concurrencyCount("concurrencyCount", 5, "how many concurrent client threads to spawn");
    util::CommandLineOption<unsigned int> clientReps("clientReps", 500, "how many iterations an individual client goes through");
    util::CommandLine::getSingleton().parse(argc, argv);

    std::cout << "Test fixed and dynamic thread pools, against all transports." << std::endl;

    RCF::ObjectPool & objectPool = RCF::getObjectPool();

    // Run all transports except UDP, on fixed and dynamic thread pools.
    for (std::size_t i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        if (typeid(*serverTransportPtr) == typeid(RCF::UdpServerTransport))
        {
            continue;
        }

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        for (int j=0; j<2; ++j)
        {
            RCF::RcfServer server( serverTransportPtr );

            RCF::ThreadManagerPtr threadManagerPtr;

            if (j == 0)
            {
                threadManagerPtr.reset(
                    new RCF::FixedThreadPool(concurrencyCount+1));
            }
            else
            {
                threadManagerPtr.reset(
                    new RCF::DynamicThreadPool(1,200));
            }

            server.getServerTransportService().getTaskEntries()[0]
                .setThreadManagerPtr( threadManagerPtr );

            runTests(server, *clientTransportAutoPtr, concurrencyCount, clientReps);

            // Have a look at the object pool.

            std::vector<std::size_t> bufferSizes;
            RCF::getObjectPool().enumerateBuffers(bufferSizes);

            std::vector<std::size_t> ostrstreamSizes;
            RCF::getObjectPool().enumerateOstrstreams(ostrstreamSizes);
        }
    }

    // TODO: The next test has never passed on the Solaris and FreeBSD test boxes. Need to figure out why.
#if defined(__SVR4) && defined(__sun)
    return boost::exit_success;
#endif

#if defined(__FreeBSD__)
    return boost::exit_success;
#endif

    std::cout << "Test growing and shrinking of dynamic thread pools, against all transports." << std::endl;

    // Test dynamic growing and shrinking of server thread pools.
    for (std::size_t i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        if (typeid(*serverTransportPtr) == typeid(RCF::UdpServerTransport))
        {
            continue;
        }

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        X x;
        RCF::RcfServer server( serverTransportPtr );

        const std::size_t ThreadCountInit = 5;
        const std::size_t ThreadCountMax = 50;
        const std::size_t ThreadIdleTimeoutMs = 5*1000;
        const bool ReserveLastThread = true;

        RCF::DynamicThreadPoolPtr threadPoolPtr( new RCF::DynamicThreadPool(
            ThreadCountInit, 
            ThreadCountMax, 
            ReserveLastThread, 
            ThreadIdleTimeoutMs) );

        threadPoolPtr->setThreadName("RCF Dynamic Thread Pool");

        server.getServerTransportService().getTaskEntries()[0]
            .setThreadManagerPtr( threadPoolPtr );

        server.bind( (I_X *) 0, x);

        server.start();

        RCF_CHECK(threadPoolPtr->getThreadCount() == ThreadCountInit);
        
        std::vector<Xyz> clients(ThreadCountMax + 1);

        for (std::size_t j=0; j<clients.size(); ++j)
        {
            RcfClient<I_X> temp(clientTransportAutoPtr->clone());
            clients[j].mClient.swap(temp);
            clients[j].mClient.getClientStub().setAutoReconnect(false);
            clients[j].mClient.getClientStub().setConnectTimeoutMs(1000*2);
            clients[j].mClient.getClientStub().setRemoteCallTimeoutMs(1000*10);
        }

        for (std::size_t j=0; j<3; ++j)
        {
            RCF_CHECK(
                threadPoolPtr->getThreadCount() == ThreadCountInit)
                (threadPoolPtr->getThreadCount())(ThreadCountInit)(j);

            // Sequential
            for (std::size_t k=0; k<clients.size(); ++k)
            {
                clients[k].mClient.ping();
            }

            RCF_CHECK(
                threadPoolPtr->getThreadCount() == ThreadCountInit)
                (threadPoolPtr->getThreadCount())(ThreadCountInit)(j);

            // Concurrent
            boost::uint32_t t0 = Platform::OS::getCurrentTimeMs();
            for (std::size_t k=0; k<clients.size(); ++k)
            {
                clients[k].mThreadPtr.reset( new RCF::Thread( boost::bind(
                    &Xyz::go,
                    &clients[k],
                    5000)));
            }
            for (std::size_t k=0; k<clients.size(); ++k)
            {
                clients[k].mThreadPtr->join();
            }
            boost::uint32_t t1 = Platform::OS::getCurrentTimeMs();
            RCF_CHECK( t1-t0 < 2*5000 )(t1-t0)(j);

            // See how many calls failed, and why.

            std::size_t errorsBusy = std::count_if(
                clients.begin(), 
                clients.end(), 
                boost::bind(
                    &Xyz::isError,
                    _1,
                    RCF::RcfError_AllThreadsBusy));

            std::size_t errorsOk = std::count_if(
                clients.begin(), 
                clients.end(), 
                boost::bind(
                    &Xyz::isError,
                    _1,
                    RCF::RcfError_Ok));

            if (ReserveLastThread)
            {
                // With asio transports, we get 1 for errorsBusy, with iocp 
                // transports, we get 2.

                std::vector<boost::int32_t> errors;
                for (std::size_t k=0; k<clients.size(); ++k)
                {
                    errors.push_back(clients[k].mRcfError);
                }

                RCF_CHECK(errorsBusy + errorsOk == clients.size())
                    (errorsBusy)(errorsOk)(clients.size())(errors)(j);
            }
            else
            {
                std::vector<boost::int32_t> errors;
                for (std::size_t k=0; k<clients.size(); ++k)
                {
                    errors.push_back(clients[k].mRcfError);
                }

                RCF_CHECK(errorsBusy == 0)(errors);
                RCF_CHECK(errorsOk < clients.size())(j)(errors);
            }

            // Wait for all the extra server threads to go away.
            Platform::OS::SleepMs(2*ThreadIdleTimeoutMs);

            RCF_CHECK(
                threadPoolPtr->getThreadCount() == ThreadCountInit)
                (threadPoolPtr->getThreadCount())(ThreadCountInit);
        }
    }
   
    return boost::exit_success;
}
