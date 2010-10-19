
#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfClient.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/ClientProgress.hpp>
#include <RCF/TcpClientTransport.hpp>

#ifdef RCF_USE_BOOST_ASIO
#include <RCF/TcpAsioServerTransport.hpp>
#else
#include <RCF/TcpIocpServerTransport.hpp>
#endif

#include <RCF/test/PrintTestHeader.hpp>
#include <RCF/test/TransportFactories.hpp>

#ifndef BOOST_WINDOWS
#define QS_PAINT 0
#endif

namespace Test_ClientProgress {

    RCF_BEGIN(I_X, "I_X")
        RCF_METHOD_R2(std::string, echo, std::string, unsigned int)
    RCF_END(I_X)

    class X
    {
    public:
        std::string echo(const std::string &s, unsigned int waitMs)
        {
            Platform::OS::SleepMs(waitMs);
            return s;
        }
    };

    unsigned int gEventCallbacks = 0;
    unsigned int gEventCallbacksConnect = 0;
    unsigned int gEventCallbacksSend = 0;
    unsigned int gEventCallbacksReceive = 0;

    unsigned int gTimerCallbacks = 0;
    unsigned int gMessageFilterCallbacks = 0;
    RCF::ClientProgress::Action gAction = RCF::ClientProgress::Continue;

    void myCallback(
        std::size_t bytesTransferred,
        std::size_t bytesTotal,
        RCF::ClientProgress::Trigger trigger,
        RCF::ClientProgress::Activity activity,
        RCF::ClientProgress::Action &action)
    {

        switch (trigger)
        {
            case RCF::ClientProgress::Event:
                ++gEventCallbacks;

                if (activity == RCF::ClientProgress::Connect) 
                {
                    ++gEventCallbacksConnect;
                }

                if (activity == RCF::ClientProgress::Send) 
                {
                    ++gEventCallbacksSend;
                }

                if (activity == RCF::ClientProgress::Receive) 
                {
                    ++gEventCallbacksReceive;
                }

                break;

            case RCF::ClientProgress::Timer:
                ++gTimerCallbacks;
                break;

            case RCF::ClientProgress::UiMessage:

#ifdef BOOST_WINDOWS

                {
                    MSG msg = {0};
                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                    {
                        if (msg.message == WM_QUIT)
                        {

                        }
                        else if (msg.message == WM_PAINT)
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
                }

#endif

                ++gMessageFilterCallbacks;
                break;

            default:
                RCF_ASSERT(0);
        }
        action = gAction;
    }

} // namespace Test_ClientProgress

int test_main(int argc, char **argv)
{
    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_ClientProgress;

    for (int i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        if (!transportFactoryPtr->isConnectionOriented())
        {
            continue;
        }

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        X x;
        RCF::RcfServer server(serverTransportPtr);
        server.bind( (I_X*) 0, x);
        server.start();

        std::string s0 = "asdf";
        std::string s;
        RcfClient<I_X> client(clientTransportAutoPtr->clone());

        s = client.echo(s0, 10);
        RCF_CHECK(s == s0);

        RCF::ClientProgressPtr clientProgressPtr( new RCF::ClientProgress() );
        client.getClientStub().setClientProgressPtr(clientProgressPtr);

        {
            clientProgressPtr->mTriggerMask = RCF::ClientProgress::Event | RCF::ClientProgress::Timer | RCF::ClientProgress::UiMessage;
            clientProgressPtr->mTimerIntervalMs = 500;
            clientProgressPtr->mUiMessageFilter = QS_PAINT;
            // vc6 hack
            //clientProgressPtr->mProgressCallback = myCallback;
            clientProgressPtr->mProgressCallback = RCF::ClientProgress::ProgressCallback(&myCallback);

            gEventCallbacks = 0;
            gEventCallbacksConnect = 0;
            gEventCallbacksSend = 0;
            gEventCallbacksReceive = 0;
            gTimerCallbacks = 0;
            gMessageFilterCallbacks = 0;
            gAction = RCF::ClientProgress::Continue;

            client.getClientStub().disconnect();

            s = client.echo(s0, 1000*5);
            RCF_CHECK(gEventCallbacks > 0);
            RCF_CHECK(gEventCallbacksConnect == 1);
            RCF_CHECK(gEventCallbacksSend > 0);
            RCF_CHECK(gEventCallbacksReceive > 0);
            RCF_CHECK(gTimerCallbacks > 0);
            RCF_CHECK(gMessageFilterCallbacks == 0); // no UI thread running anyway...
        }

        {
            clientProgressPtr->mTriggerMask = RCF::ClientProgress::Event;
            clientProgressPtr->mTimerIntervalMs = 500;
            clientProgressPtr->mUiMessageFilter = QS_PAINT;
            clientProgressPtr->mProgressCallback = RCF::ClientProgress::ProgressCallback(&myCallback);

            gEventCallbacks = 0;
            gEventCallbacksConnect = 0;
            gEventCallbacksSend = 0;
            gEventCallbacksReceive = 0;
            gTimerCallbacks = 0;
            gMessageFilterCallbacks = 0;
            gAction = RCF::ClientProgress::Continue;

            s = client.echo(s0, 1000*5);
            RCF_CHECK(gEventCallbacks > 0);
            RCF_CHECK(gEventCallbacksConnect == 0);
            RCF_CHECK(gEventCallbacksSend > 0);
            RCF_CHECK(gEventCallbacksReceive > 0);
            RCF_CHECK(gTimerCallbacks == 0);
            RCF_CHECK(gMessageFilterCallbacks == 0); // no UI thread running anyway...
        }

        {
            clientProgressPtr->mTriggerMask = RCF::ClientProgress::Timer;
            clientProgressPtr->mTimerIntervalMs = 500;
            clientProgressPtr->mUiMessageFilter = QS_PAINT;
            clientProgressPtr->mProgressCallback = RCF::ClientProgress::ProgressCallback(&myCallback);

            gEventCallbacks = 0;
            gEventCallbacksConnect = 0;
            gEventCallbacksSend = 0;
            gEventCallbacksReceive = 0;
            gTimerCallbacks = 0;
            gMessageFilterCallbacks = 0;
            gAction = RCF::ClientProgress::Continue;

            client.getClientStub().disconnect();
            s = client.echo(s0, 1000*5);
            RCF_CHECK(gEventCallbacks == 0);
            RCF_CHECK(gEventCallbacksConnect == 0);
            RCF_CHECK(gEventCallbacksSend == 0);
            RCF_CHECK(gEventCallbacksReceive == 0);
            RCF_CHECK(gTimerCallbacks > 0);
            RCF_CHECK(gMessageFilterCallbacks == 0); // no UI thread running anyway...
        }

        {
            clientProgressPtr->mTriggerMask = RCF::ClientProgress::UiMessage;
            clientProgressPtr->mTimerIntervalMs = 500;
            clientProgressPtr->mUiMessageFilter = QS_PAINT;
            clientProgressPtr->mProgressCallback = RCF::ClientProgress::ProgressCallback(&myCallback);

            gEventCallbacks = 0;
            gEventCallbacksConnect = 0;
            gEventCallbacksSend = 0;
            gEventCallbacksReceive = 0;
            gTimerCallbacks = 0;
            gMessageFilterCallbacks = 0;
            gAction = RCF::ClientProgress::Continue;

            s = client.echo(s0, 1000*5);
            RCF_CHECK(gEventCallbacks == 0);
            RCF_CHECK(gEventCallbacksConnect == 0);
            RCF_CHECK(gEventCallbacksSend == 0);
            RCF_CHECK(gEventCallbacksReceive == 0);
            RCF_CHECK(gTimerCallbacks == 0);
            RCF_CHECK(gMessageFilterCallbacks == 0); // no UI thread running anyway...
        }

        {
            clientProgressPtr->mTriggerMask = RCF::ClientProgress::Event;
            clientProgressPtr->mTimerIntervalMs = 500;
            clientProgressPtr->mUiMessageFilter = QS_PAINT;
            clientProgressPtr->mProgressCallback = RCF::ClientProgress::ProgressCallback(&myCallback);

            gEventCallbacks = 0;
            gEventCallbacksConnect = 0;
            gEventCallbacksSend = 0;
            gEventCallbacksReceive = 0;
            gTimerCallbacks = 0;
            gMessageFilterCallbacks = 0;
            gAction = RCF::ClientProgress::Cancel;

            try
            {
                s = client.echo(s0, 1000*5);
                RCF_CHECK(1==0);
            }
            catch (const RCF::Exception &e)
            {
                RCF_CHECK(1==1);
                RCF_CHECK(e.getErrorId() == RCF::RcfError_ClientCancel);
                RCF_CHECK(gEventCallbacks > 0);
                RCF_CHECK(gEventCallbacksConnect == 0);
                RCF_CHECK(gEventCallbacksSend == 1);
                RCF_CHECK(gEventCallbacksReceive == 0);
                RCF_CHECK(gTimerCallbacks == 0);
                RCF_CHECK(gMessageFilterCallbacks == 0); // no UI thread running anyway...
            }
        }

        {
            clientProgressPtr->mTriggerMask = RCF::ClientProgress::Timer;
            clientProgressPtr->mTimerIntervalMs = 500;
            clientProgressPtr->mUiMessageFilter = QS_PAINT;
            clientProgressPtr->mProgressCallback = RCF::ClientProgress::ProgressCallback(&myCallback);

            gEventCallbacks = 0;
            gEventCallbacksConnect = 0;
            gEventCallbacksSend = 0;
            gEventCallbacksReceive = 0;
            gTimerCallbacks = 0;
            gMessageFilterCallbacks = 0;
            gAction = RCF::ClientProgress::Cancel;

            try
            {
                s = client.echo(s0, 1000*5);
                RCF_CHECK(1==0);
            }
            catch (const RCF::Exception &e)
            {
                RCF_CHECK(1==1);
                RCF_CHECK(e.getErrorId() == RCF::RcfError_ClientCancel);
                RCF_CHECK(gEventCallbacks == 0);
                RCF_CHECK(gEventCallbacksConnect == 0);
                RCF_CHECK(gEventCallbacksSend == 0);
                RCF_CHECK(gEventCallbacksReceive == 0);
                RCF_CHECK(gTimerCallbacks > 0);
                RCF_CHECK(gMessageFilterCallbacks == 0); // no UI thread running anyway...
            }
        }
    }

    return boost::exit_success;
}
