
#include <string>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/ObjectFactoryService.hpp>
#include <RCF/PublishingService.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/SubscriptionService.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/ThreadManager.hpp>

#include <RCF/test/TransportFactories.hpp>
#include <RCF/test/ThreadGroup.hpp>
#include <RCF/test/WithStopServer.hpp>

#include <RCF/util/CommandLine.hpp>

#ifdef BOOST_WINDOWS
#include <RCF/SspiFilter.hpp>
#endif

#ifndef BOOST_WINDOWS
#define LONG int
RCF::Mutex mInterlockedMutex;

LONG InterlockedIncrement(LONG volatile *lpn)
{
    RCF::Lock lock(mInterlockedMutex);
    return ++(*lpn);
}

LONG InterlockedExchangeAdd(LONG volatile *lpn, LONG m)
{
    RCF::Lock lock(mInterlockedMutex);
    LONG orig = *lpn;
    (*lpn) += m;
    return orig;
}

#endif

#ifdef RCF_USE_BOOST_SERIALIZATION

namespace RCF
{
    template<typename Archive>
    void serialize(Archive & archive, TcpEndpoint&, const unsigned int)
    {
        RCF_ASSERT(0);
    }

    void serialize(SF::Archive & archive, TcpEndpoint& tcpEndpoint, const unsigned int)
    {
        serialize(archive, tcpEndpoint);
    }
}

#endif

namespace Test_Endurance {

    std::string toString(const RCF::Exception &e)
    {
        std::ostringstream os;
        int err = e.getErrorId();
        std::string errMsg = RCF::getErrorString(err);
        os << "[RCF:" << err << "] " << errMsg << std::endl;
        if (e.getSubSystem() == RCF::RcfSubsystem_Os)
        {
            err = e.getSubSystemError();
            errMsg = Platform::OS::GetErrorString(err);
            os << "[OS:" << err << "] " << errMsg << std::endl;
        }
        os << "[What] " << e.what() << std::endl;
        os << "[Context] " << e.getContext() << std::endl;
        os << "[Exception type] " << typeid(e).name() << std::endl;
        return os.str();
    }

    class Echo : public RCF::WithStopServer
    {
    public:
        Echo(LONG reportingInterval = 5000) :
            totalCalls(RCF_DEFAULT_INIT),
            reportingInterval(reportingInterval),
            totalStringLength(RCF_DEFAULT_INIT),
            lastReportTimeMs( Platform::OS::getCurrentTimeMs())
        {
            //RCF::Thread t( boost::bind(&Echo::setLock, this));
            //t.join();
        }

        std::string echo(const std::string &s)
        {
            InterlockedIncrement(&totalCalls);
            InterlockedExchangeAdd(&totalStringLength, s.length());

            if (totalCalls > reportingInterval)
            {
                RCF::Lock lock(mReportingMutex);
                if (totalCalls > reportingInterval)
                {
                    std::size_t totalTimeMs = Platform::OS::getCurrentTimeMs() - lastReportTimeMs;
                    double totalTimeS = static_cast<double>(totalTimeMs) / 1000.0;

                    std::cout
                        << "Last " << reportingInterval << " echo() calls: "
                        << totalTimeS << " (s), "
                        << totalStringLength << " (total echoed string length)"
                        << std::endl;

                    totalCalls = 0;
                    totalStringLength = 0;
                    lastReportTimeMs = Platform::OS::getCurrentTimeMs();
                }
            }

            return s;
        }

        /*void stopServer()
        {
            mLockPtr->unlock();
        }

        void wait()
        {
            RCF::Lock lock(mMutex);
        }*/

    private:

        /*void setLock()
        {
            mLockPtr.reset( new RCF::Lock(mMutex));
        }

        RCF::Mutex mMutex;
        RCF::LockPtr mLockPtr;*/

        LONG totalCalls;
        LONG totalStringLength;
        std::size_t lastReportTimeMs;
        LONG reportingInterval;
        RCF::Mutex mReportingMutex;

    };

    RCF_BEGIN(I_Echo, "I_Echo")
        RCF_METHOD_R1(std::string, echo, const std::string &)
        RCF_METHOD_V0(void, stopServer)
    RCF_END(I_Echo)

    volatile bool gStopFlag = false;

    //boost::mutex gIostreamMutex;
    RCF::Mutex gIostreamMutex;

    template<typename Functor>
    void wrapInTryCatch(Functor functor)
    {
        try
        {
            functor();
        }
        catch(const RCF::Exception &e)
        {
            RCF::Lock lock(gIostreamMutex);
            std::cout << toString(e) << std::endl;
            gStopFlag = true;
        }
        catch(const std::exception &e)
        {
            RCF::Lock lock(gIostreamMutex);
            std::cout << e.what() << std::endl;
            gStopFlag = true;
        }
    }

    //**************************************************************************
    // ping task

    namespace PingTask {

        void ping1_(const RCF::I_ClientTransport &clientTransport)
        {
            std::size_t connectionCount = 500;

            std::vector<std::string> strings(10);
            strings[0] = "A";
            for (std::size_t i=1; i<strings.size(); ++i)
            {
                strings[i] = strings[i-1] + strings[i-1];
            }

            typedef RcfClient<I_Echo> Client;
            typedef boost::shared_ptr<Client> ClientPtr;
            std::vector<ClientPtr> clients;
            for (std::size_t i=0; i<connectionCount; ++i)
            {
                clients.push_back( ClientPtr( new Client( clientTransport.clone())));
                std::string s = clients[i]->echo( strings[0] );
            }

            int counter = 0;
            do
            {
                int whichClient = rand() % connectionCount;
                int whichString = rand() % strings.size();
                try
                {
                    std::string s = clients[whichClient]->echo( strings[whichString] );
                    RCF_CHECK( s == strings[whichString] );
                }
                catch(const RCF::Exception &e)
                {
                    RCF::Lock lock(gIostreamMutex);
                    time_t ltime;
                    time(&ltime);
                    std::cout
                        << "Client exception caught: " << __FILE__ << "(" << __LINE__ << ")" << std::endl
                        << "Time: " << ctime(&ltime)
                        << Test_Endurance::toString(e) <<  std::endl << std::endl;
                }

            }
            while (!gStopFlag);
        }

        void ping1(const RCF::I_ClientTransport &clientTransport)
        {
            wrapInTryCatch( boost::bind( &ping1_, boost::ref(clientTransport)));
            RCF::Lock lock(gIostreamMutex);
            std::cout << "Thread exiting: " << __FILE__ << "(" << __LINE__ << ")" << std::endl;
        }

        void ping3_(const RCF::I_ClientTransport &clientTransport)
        {
            std::string s = "the quick brown dog";

            RcfClient<I_Echo> client(clientTransport.clone());
            RCF_CHECK(s == client.echo(s));

            do
            {
                try
                {
                    RCF_CHECK( s == client.echo(s) );
                }
                catch(const RCF::Exception &e)
                {
                    RCF::Lock lock(gIostreamMutex);
                    time_t ltime;
                    time(&ltime);
                    std::cout
                        << "Client exception caught: " << __FILE__ << "(" << __LINE__ << ")" << std::endl
                        << "Time: " << ctime(&ltime)
                        << Test_Endurance::toString(e) <<  std::endl << std::endl;
                }
            }
            while (!gStopFlag);
        }

        void ping3(const RCF::I_ClientTransport &clientTransport)
        {
            wrapInTryCatch( boost::bind( &ping3_, boost::ref(clientTransport)));
            RCF::Lock lock(gIostreamMutex);
            std::cout << "Thread exiting: " << __FILE__ << "(" << __LINE__ << ")" << std::endl;
        }


    } // namespace PingTask

    // basic calls on a fixed number of connections
    void pingTask(int taskId, const RCF::I_ClientTransport &clientTransport, double intensity)
    {
        {
            RCF::Lock lock(gIostreamMutex);
            std::cout << "Client task " << taskId << ": pinging server" << std::endl;
        }
        PingTask::ping1(clientTransport);
    }

    void flatOutPingTask(int taskId, const RCF::I_ClientTransport &clientTransport)
    {
        {
            RCF::Lock lock(gIostreamMutex);
            std::cout << "Client task " << taskId << ": flat out pinging" << std::endl;
        }
        PingTask::ping3(clientTransport);
    }

    RCF_BEGIN(I_Publish, "I_Publish")
        RCF_METHOD_V0(void, func)
        RCF_METHOD_V2(void, func, int, int)
        RCF_METHOD_V5(void, func, std::string, int, std::string, int, std::string)
    RCF_END(I_Publish)

    //**************************************************************************
    // client subscribing task

    RCF_BEGIN(I_ControlPublishing, "I_ControlPublishing")
        RCF_METHOD_V1(void, setFrequency, int)
        RCF_METHOD_V0(void, resetPublishing)
    RCF_END(I_ControlPublishing)

    class Publish
    {
    public:
        Publish() : mCount(RCF_DEFAULT_INIT)
        {}

        void func()
        {
            ++mCount;
        }

        void func(int n1, int n2)
        {
            ++mCount;
        }

        void func(std::string s1, int n1, std::string s2, int n2, std::string s3)
        {
            ++mCount;
        }

        int mCount;
    };

    class MyPublishingService : public RCF::I_Service
    {
    public:

        MyPublishingService(RCF::PublishingServicePtr publishingServicePtr) :
            mPublishingServicePtr(publishingServicePtr),
            mWaitMs(1000),
            mResetFlag()
        {}

        void onServiceAdded(RCF::RcfServer &server)
        {
            server.bind( (I_ControlPublishing*) 0, *this);
            RCF::WriteLock writeLock(getTaskEntriesMutex());
            getTaskEntries().clear();
            getTaskEntries().push_back( RCF::TaskEntry(
                boost::bind(&MyPublishingService::cyclePublishing, this, _1, _2),
                boost::bind(&MyPublishingService::stopPublishing, this) ));
        }

        void onServiceRemoved(RCF::RcfServer &server)
        {
            server.unbind( (I_ControlPublishing*) 0);
        }

        void onServerOpen(RCF::RcfServer &server)
        {}

        void onServerClose(RCF::RcfServer &server)
        {}

        void onServerStart(RCF::RcfServer &server)
        {
            mPublishingServicePtr->beginPublish( (I_Publish*) 0);
        }

        void onServerStop(RCF::RcfServer &server)
        {
            mPublishingServicePtr->endPublish( (I_Publish*) 0);
        }

        bool cyclePublishing(int timeoutMs, const volatile bool &stopFlag)
        {
            if (mResetFlag)
            {
                mPublishingServicePtr->endPublish( (I_Publish*) 0);
                mPublishingServicePtr->beginPublish( (I_Publish*) 0);
                mResetFlag = false;
            }
            else
            {
                mPublishingServicePtr->publish( (I_Publish*) 0).func(1,2);
                mPublishingServicePtr->publish( (I_Publish*) 0).func("three",4,"five",6,"seven");
                mPublishingServicePtr->publish( (I_Publish*) 0).func(1,2);
                mPublishingServicePtr->publish( (I_Publish*) 0).func("three",4,"five",6,"seven");
                mPublishingServicePtr->publish( (I_Publish*) 0).func(1,2);
                mPublishingServicePtr->publish( (I_Publish*) 0).func("three",4,"five",6,"seven");
            }
            Platform::OS::SleepMs(mWaitMs);
            return stopFlag;
        }

        void stopPublishing()
        {}

        void setFrequency(int waitMs)
        {
            mWaitMs = waitMs;
        }

        void resetPublishing()
        {
            mResetFlag = true;
        }

    private:

        RCF::PublishingServicePtr mPublishingServicePtr;
        int mWaitMs;
        bool mResetFlag;

    };

    typedef boost::shared_ptr<MyPublishingService> MyPublishingServicePtr;



    namespace SubscriptionTask {

        void subscriber_(
            RCF::TransportFactoryPtr transportFactoryPtr,
            const RCF::I_ClientTransport &clientTransport,
            std::size_t subscriptionCount)
        {
            // subscribing requires a server
            RCF::TransportPair transportPair = transportFactoryPtr->createNonListeningTransports();
            RCF::RcfServer server(transportPair.first);

            // thread pool
            //if (useDynamicThreadPool)
            {
                RCF::DynamicThreadPoolPtr dynamicThreadPoolPtr( new RCF::DynamicThreadPool(2, 4));
                server.getServerTransportService().getTaskEntries()[0].setThreadManagerPtr(dynamicThreadPoolPtr);
            }
            //else
            //{
            //    RCF::FixedThreadPoolPtr fixedThreadPoolPtr( new RCF::FixedThreadPool(threadCount));
            //    server.getServerTransportService().getTaskEntries()[0].setThreadManagerPtr(fixedThreadPoolPtr);
           // }

            // subscription service
            RCF::SubscriptionServicePtr subscriptionServicePtr( new RCF::SubscriptionService());
            server.addService(subscriptionServicePtr);

            // start her up
            server.start();

            std::vector< std::pair<Publish, bool> > publishVec(subscriptionCount);
            for (std::size_t i=0; i<publishVec.size(); ++i)
            {
                try
                {

                    subscriptionServicePtr->beginSubscribe(
                        (I_Publish*) 0,
                        publishVec[i].first,
                        clientTransport.clone());

                    //myBeginSubscribe(clientTransport.clone());

                    publishVec[i].second = true;
                }
                catch (const RCF::Exception &e)
                {
                    RCF::Lock lock(gIostreamMutex);
                    std::cout << "Initial subscription request failed" << std::endl;
                }
            }

            do
            {
                Platform::OS::SleepMs(500);
                for (std::size_t i=0; i<publishVec.size(); ++i)
                {
                    bool flip = ((rand() % 10) == 0);
                    if (flip)
                    {
                        if (publishVec[i].second == true)
                        {
                            subscriptionServicePtr->endSubscribe(
                                (I_Publish*) 0,
                                publishVec[i].first);
                            publishVec[i].second = false;
                        }
                        else
                        {
                            try
                            {

                                subscriptionServicePtr->beginSubscribe(
                                    (I_Publish*) 0,
                                    publishVec[i].first,
                                    clientTransport.clone());

                                //myBeginSubscribe(clientTransport.clone());

                                publishVec[i].second = true;
                            }
                            catch (const RCF::Exception &e)
                            {
                                RCF::Lock lock(gIostreamMutex);
                                std::cout << "Subscription request failed" << std::endl;
                            }
                        }
                    }
                }
            }
            while (!gStopFlag);

        }

        void subscriber(
            RCF::TransportFactoryPtr transportFactoryPtr,
            const RCF::I_ClientTransport &clientTransport)
        {
            wrapInTryCatch( boost::bind( &subscriber_,
                transportFactoryPtr,
                boost::ref(clientTransport),
                50));

            RCF::Lock lock(gIostreamMutex);
            std::cout << "Thread exiting: " << __FILE__ << "(" << __LINE__ << ")" << std::endl;
        }

        void controller_(const RCF::I_ClientTransport &clientTransport)
        {
            do
            {
                Platform::OS::SleepMs(10*1000);

                try
                {
                    int nAction = rand() % 7;
                    switch (nAction)
                    {
                    case 0:
                        RcfClient< I_ControlPublishing >(clientTransport.clone()).setFrequency(500);
                        break;

                    case 1:
                        RcfClient< I_ControlPublishing >(clientTransport.clone()).setFrequency(100);
                        break;

                    case 2:
                        RcfClient< I_ControlPublishing >(clientTransport.clone()).setFrequency(25);
                        break;

                    case 3:
                    case 4:
                    case 5:
                        RcfClient< I_ControlPublishing >(clientTransport.clone()).setFrequency(5);
                        break;

                    case 6:
                        if (rand() % 3 == 0)
                        {
                            RcfClient< I_ControlPublishing >(clientTransport.clone()).resetPublishing();
                        }
                        break;

                    default:
                        break;
                    }
                }
                catch (const RCF::Exception &e)
                {
                    RCF::Lock lock(gIostreamMutex);
                    std::cout
                        << "I_ControlPublishing call failed: " << __FILE__ << "(" << __LINE__ << ")" << std::endl
                        << Test_Endurance::toString(e) <<  std::endl << std::endl;
                }
            }
            while (!gStopFlag);
        }

        void controller(const RCF::I_ClientTransport &clientTransport)
        {
            wrapInTryCatch( boost::bind( &controller_, boost::ref(clientTransport)));
            RCF::Lock lock(gIostreamMutex);
            std::cout << "Thread exiting: " << __FILE__ << "(" << __LINE__ << ")" << std::endl;
        }

    } // namespace SubscriptionTask

    void clientSubscriptionTask(int taskId, RCF::TransportFactoryPtr transportFactoryPtr, const RCF::I_ClientTransport &clientTransport)
    {

        {
            RCF::Lock lock(gIostreamMutex);
            std::cout << "Client task " << taskId << ": subscribing to server" << std::endl;
        }

        ThreadGroup myThreads;
        myThreads.push_back( ThreadPtr( new Thread(
            boost::bind(SubscriptionTask::subscriber, transportFactoryPtr, boost::ref(clientTransport)))));
        myThreads.push_back( ThreadPtr( new Thread(
            boost::bind(SubscriptionTask::controller, boost::ref(clientTransport)))));
        joinThreadGroup(myThreads);
    }

    //**************************************************************************
    // client publishing task

    RCF_BEGIN(I_ControlSubscribing, "I_ControlSubscribing")
        RCF_METHOD_V1(void, addPublisher, const RCF::TcpEndpoint &)
    RCF_END(I_ControlSubscribing)

    class MySubscribingService : public RCF::I_Service
    {
    public:

        MySubscribingService(RCF::SubscriptionServicePtr subscriptionServicePtr) :
          mSubscriptionServicePtr(subscriptionServicePtr)
          {}

          void onServiceAdded(RCF::RcfServer &server)
          {
              server.bind( (I_ControlSubscribing*) 0, *this);
              RCF::WriteLock writeLock(getTaskEntriesMutex());
              getTaskEntries().clear();
              getTaskEntries().push_back( RCF::TaskEntry(
                  boost::bind(&MySubscribingService::cycleSubscribing, this, _1, _2),
                  boost::bind(&MySubscribingService::stopSubscribing, this) ));
          }

          void onServiceRemoved(RCF::RcfServer &server)
          {
              server.unbind( (I_ControlSubscribing*) 0);
          }

          void onServerOpen(RCF::RcfServer &server)
          {}

          void onServerClose(RCF::RcfServer &server)
          {}

          void onServerStart(RCF::RcfServer &server)
          {}

          void onServerStop(RCF::RcfServer &server)
          {}

          bool cycleSubscribing(int timeoutMs, const volatile bool &stopFlag)
          {
              Platform::OS::SleepMs(10*1000);

              std::vector<RCF::TcpEndpoint> endpointsToAdd;
              (RCF::Lock(mMutex), endpointsToAdd.swap(mEndpointsToAdd));

              std::vector<RCF::TcpEndpoint> endpointsToRemove;

              // add new subscribers
              for (std::size_t i=0; i<endpointsToAdd.size(); ++i)
              {
                  RCF::TcpEndpoint endpoint = endpointsToAdd[i];
                  mPublishers[endpoint].clear();
                  for (int i=0; i<5; ++i)
                  {
                      mPublishers[endpoint].push_back( boost::shared_ptr<Publish>( new Publish()));

                      bool ok = false;

                      try
                      {

                          ok = mSubscriptionServicePtr->beginSubscribe(
                              (I_Publish*) 0,
                              *mPublishers[endpoint].back(),
                              endpoint);

                          //ok = myBeginSubscribe(endpoint);

                      }
                      catch(const RCF::Exception &)
                      {}

                      if (!ok)
                      {
                          endpointsToRemove.push_back(endpoint);
                          break;
                      }
                  }
              }

              // periodic random re-subscribing
              for (
                  Publishers::const_iterator iter = mPublishers.begin();
                  iter != mPublishers.end();
                  ++iter)
              {

                    RCF_ASSERT((*iter).second.size() == 5);

                    int i = rand() % ((*iter).second.size()-1);
                    Publish &publish = *(*iter).second[i+1];
                    const RCF::TcpEndpoint &endpoint = (*iter).first;

                    mSubscriptionServicePtr->endSubscribe(
                        (I_Publish*) 0,
                        publish);

                    bool ok = false;
                    try
                    {

                        ok = mSubscriptionServicePtr->beginSubscribe(
                            (I_Publish*) 0,
                            publish,
                            endpoint);

                        //ok = myBeginSubscribe(endpoint);
                    }
                    catch(const RCF::Exception &)
                    {}

                    if (!ok)
                    {
                        endpointsToRemove.push_back(endpoint);
                    }
              }

              // remove expired endpoints
              for (std::size_t i=0; i<endpointsToRemove.size(); ++i)
              {
                  RCF::TcpEndpoint endpoint = endpointsToRemove[i];
                  mPublishers.erase(endpoint);
                  RCF::Lock lock(gIostreamMutex);
                  std::cout << "Ceasing subscriptions from " << endpoint.getIp() << ":" << endpoint.getPort() << std::endl;
              }

              {
                  RCF::Lock lock(gIostreamMutex);
                  std::cout << "Currently subscribing to " << mPublishers.size() << " endpoints" << std::endl;
              }

              return stopFlag;

          }

          void stopSubscribing()
          {}

          void addPublisher(const RCF::TcpEndpoint &endpoint)
          {
              RCF::Lock lock(mMutex);
              mEndpointsToAdd.push_back(endpoint);
          }

    private:


        RCF::SubscriptionServicePtr mSubscriptionServicePtr;

        typedef std::map<RCF::TcpEndpoint, std::vector<boost::shared_ptr<Publish> > > Publishers;
        Publishers mPublishers;

        RCF::Mutex mMutex;
        std::vector<RCF::TcpEndpoint> mEndpointsToAdd;
    };

    typedef boost::shared_ptr<MySubscribingService> MySubscribingServicePtr;

    namespace PublishingTask {

        void publisher_(
            RCF::TransportFactoryPtr transportFactoryPtr,
            const RCF::I_ClientTransport &clientTransport)
        {
            RCF::RcfServer server( RCF::TcpEndpoint("0.0.0.0", 0));

            RCF::PublishingServicePtr publishingServicePtr(new RCF::PublishingService());
            server.addService(publishingServicePtr);

            server.start();

            publishingServicePtr->beginPublish( (I_Publish*) 0);

#ifdef BOOST_WINDOWS
            std::string ip = RCF::getMyMachineName();
#else
            RCF_CHECK(1==0 && "need to obtain machine name");
            std::string ip = "127.0.0.1";
#endif
            int port = boost::dynamic_pointer_cast<RCF::I_IpServerTransport>(
                server.getServerTransportPtr())->getPort();

            RcfClient<I_ControlSubscribing>(clientTransport.clone()).addPublisher(RCF::TcpEndpoint(ip, port));
            do
            {
                Platform::OS::SleepMs(50);
                for (int i=0; i<10; ++i)
                {
                    publishingServicePtr->publish( (I_Publish*) 0).func(3,4);
                }

            }
            while (!gStopFlag);
        }

        void publisher(
            RCF::TransportFactoryPtr transportFactoryPtr,
            const RCF::I_ClientTransport &clientTransport)
        {
            wrapInTryCatch( boost::bind( &publisher_,
                transportFactoryPtr,
                boost::ref(clientTransport)));
            RCF::Lock lock(gIostreamMutex);
            std::cout << "Thread exiting: " << __FILE__ << "(" << __LINE__ << ")" << std::endl;
        }

    } // namespace PublishingTask

    void clientPublishingTask(
        int taskId,
        RCF::TransportFactoryPtr transportFactoryPtr,
        const RCF::I_ClientTransport &clientTransport)
    {
        {
            RCF::Lock lock(gIostreamMutex);
            std::cout << "Client task " << taskId << ": publishing to server" << std::endl;
        }

        PublishingTask::publisher(transportFactoryPtr, clientTransport);
    }

    //**************************************************************************
    // object creation task

    RCF_BEGIN(I_MyObject, "I_MyObject")
        RCF_METHOD_R0(int, getVal)
        RCF_METHOD_V1(void, setVal, int)
    RCF_END(I_MyObject)

    class MyObject
    {
    public:
        int getVal()
        {
            return mVal;
        }

        void setVal(int val)
        {
            mVal = val;
        }

    private:
        int mVal;
    };

    namespace ObjectCreationTask {

        void exerciseObjectFactory_(const RCF::I_ClientTransport &clientTransport)
        {
            typedef RcfClient<I_MyObject>       Client;
            typedef boost::shared_ptr<Client>   ClientPtr;
            std::vector<ClientPtr> clients;

            std::size_t clientSize = 20;
            for (std::size_t i=0; i<clientSize; ++i)
            {
                clients.push_back( ClientPtr(new Client(clientTransport.clone())));
            }

            // setup initial window
            const std::size_t windowSize = 5;
            std::size_t windowMin = 0;
            std::size_t windowMax = windowSize;
            for (std::size_t i=windowMin; i<windowMax; ++i)
            {
                bool ok = tryCreateRemoteObject<I_MyObject>(*clients[i]);
                RCF_CHECK(ok);
            }

            do
            {
                {
                    // advance the window
                    clients[windowMin]->getClientStub().disconnect();
                    while (!tryCreateRemoteObject<I_MyObject>(*clients[windowMax]))
                    {
                        std::cout << "Object creation request failed, retrying after 1s...\n";
                        Platform::OS::SleepMs(1000);
                    }

                    windowMin = (windowMin+1) % clients.size();
                    windowMax = (windowMax+1) % clients.size();
                }

                // exercise clients in the window
                for (std::size_t i = windowMin; i != windowMax; i = (i+1) % clients.size())
                {
                    std::size_t calls = rand() % 10;
                    for (std::size_t j=0; j<calls; ++j)
                    {
                        int val0 = rand();
                        clients[i]->setVal(val0);
                        //try { clients[i]->setVal(val0); } catch(const RCF::Exception &e) {}
                        int val1 = 0;
                        val1 = clients[i]->getVal();
                        //try { val1 = clients[i]->getVal(); } catch(const RCF::Exception &e) {}
                        RCF_CHECK(val0 == val1);
                    }
                }

                Platform::OS::SleepMs(1000);
            }
            while(!gStopFlag);
        }

        void exerciseObjectFactory(const RCF::I_ClientTransport &clientTransport)
        {
            wrapInTryCatch( boost::bind(
                &exerciseObjectFactory_,
                boost::ref(clientTransport)));
            RCF::Lock lock(gIostreamMutex);
            std::cout << "Thread exiting: " << __FILE__ << "(" << __LINE__ << ")" << std::endl;
        }

    } // namespace ObjectCreationTask


    void objectCreationTask(int taskId, const RCF::I_ClientTransport &clientTransport)
    {
        // create server objects and call methods on them

        {
            RCF::Lock lock(gIostreamMutex);
            std::cout << "Client task " << taskId << ": exercising object factory" << std::endl;
        }

        ObjectCreationTask::exerciseObjectFactory(clientTransport);
    }

    // TODO: payload and transport filtered tasks
    // ...

    // vc6 hack
    void VC6Hack(boost::function1<void, int> task, int arg)
    {
        task(arg);
    }

    void optionallyAddTask(ThreadGroup &threadGroup, const std::vector<int> &taskIds, int taskId, boost::function1<void, int> task)
    {
        if (taskIds.empty() || std::find(taskIds.begin(), taskIds.end(), taskId) != taskIds.end())
        {
            // this doesn't work on vc6
            //threadGroup.push_back( ThreadPtr( new Thread(
            //    boost::bind(task, taskId))));

            threadGroup.push_back( ThreadPtr( new Thread(
                boost::bind(VC6Hack, task, taskId))));
        }
    }

} // namespace Test_Minimal


int test_main(int argc, char **argv)
{
    using namespace Test_Endurance;

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    util::CommandLineOption<bool>           clServer(       "server",           false,  "act as server");
    util::CommandLineOption<bool>           clClient(       "client",           false,  "act as client");
    util::CommandLineOption<int>            clInterval(     "interval",         5000,   "reporting interval (calls)");
    util::CommandLineOption<int>            clTask(         "task",             0,      "which client task to run");
    util::CommandLineOption<bool>           clStopFlag(     "stopflag",         true,   "stop test immediately");
    util::CommandLineOption<int>            clTransport(    "transport",        0,      "which transport");
    util::CommandLineOption<int>            clServerThreads("serverthreads",    1,      "server thread pool size");
    util::CommandLineOption<int>            clConnectTimeout("connectS",        10,        "connect timeout (s)");
    util::CommandLineOption<int>            clReceiveTimeout("receiveS",        10,        "receive timeout (s)");
    util::CommandLine::getSingleton().parse(argc, argv);

    RCF::setDefaultConnectTimeoutMs(1000*clConnectTimeout);
    RCF::setDefaultRemoteCallTimeoutMs(1000*clReceiveTimeout);

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

    int whichTransportFactory = clTransport;

    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[whichTransportFactory];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        serverTransportPtr->setMaxMessageLength(1000*10);
        clientTransportAutoPtr->setMaxMessageLength(1000*10);

        std::string s0 = "something special";

        Echo echo( clInterval.get() );
        RCF::RcfServer server( serverTransportPtr );
        if (bServer)
        {
            // TODO: transport variation: fixed/dynamic, rcf/asio
            if (!bClient)
            {
                RCF::I_IpServerTransport &ipServerTransport = dynamic_cast<RCF::I_IpServerTransport &>(*serverTransportPtr);
                ipServerTransport.setNetworkInterface("0.0.0.0");
            }
            RCF::FixedThreadPoolPtr fixedThreadPoolPtr( new RCF::FixedThreadPool(clServerThreads));
            server.getServerTransportService().getTaskEntries()[0].setThreadManagerPtr( fixedThreadPoolPtr);
            std::cout << "Server thread pool size (fixed): " << clServerThreads.get() << std::endl;

            // expose a single object
            server.bind( (I_Echo *) 0, echo);

            // setup an object factory
            unsigned int numberOfTokens = 75*5;
            unsigned int objectTimeoutS = 2*5;
            RCF::ObjectFactoryServicePtr objectFactoryServicePtr(
                new RCF::ObjectFactoryService(numberOfTokens, objectTimeoutS));
            //objectFactoryServicePtr->bind<I_MyObject, MyObject>();
            objectFactoryServicePtr->bind( (I_MyObject*) 0, (MyObject**) 0);
            server.addService(objectFactoryServicePtr);

            // setup a publishing service
            RCF::PublishingServicePtr publishingServicePtr( new RCF::PublishingService());
            server.addService(publishingServicePtr);

            // setup a publishing thread
            //MyPublishingServicePtr myPublishingServicePtr( new MyPublishingService(publishingServicePtr));
            //server.addService(myPublishingServicePtr);

            // setup a subscription service
            RCF::SubscriptionServicePtr subscriptionServicePtr( new RCF::SubscriptionService());
            server.addService(subscriptionServicePtr);

            // setup a subscribing thread
            //MySubscribingServicePtr mySubscribingServicePtr( new MySubscribingService(subscriptionServicePtr));
            //server.addService(mySubscribingServicePtr);

            server.start();
        }
        else
        {
            Platform::OS::SleepMs(1000);
        }

        if (bClient)
        {
            std::vector<int> taskIds    = clTask.getValues();
            gStopFlag                   = clStopFlag;
            int taskId                  = 0;
            ThreadGroup threadGroup;

            optionallyAddTask(
                threadGroup, taskIds, ++taskId,
                boost::bind( &pingTask, _1, boost::ref(*clientTransportAutoPtr), .6));

            //optionallyAddTask(
            //    threadGroup, taskIds, ++taskId,
            //    boost::bind( &clientSubscriptionTask, _1, transportFactoryPtr, boost::ref(*clientTransportAutoPtr)));

            //optionallyAddTask(
            //    threadGroup, taskIds, ++taskId,
            //    boost::bind( &clientPublishingTask, _1, transportFactoryPtr, boost::ref(*clientTransportAutoPtr)));

            optionallyAddTask(
                threadGroup, taskIds, ++taskId,
                boost::bind( &objectCreationTask, _1, boost::ref(*clientTransportAutoPtr)));

            optionallyAddTask(
                threadGroup, taskIds, ++taskId,
                boost::bind( &flatOutPingTask, _1, boost::ref(*clientTransportAutoPtr)));

            joinThreadGroup(threadGroup);

            RcfClient<I_Echo>( clientTransportAutoPtr->clone()).stopServer();
        }

        if (bServer)
        {
            echo.wait();
            server.stop();
        }
    }

    time_t now;
    time(&now);
    std::cout << "Time: " << ctime(&now);

    return boost::exit_success;
}
