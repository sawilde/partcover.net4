
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2007. All rights reserved.
// Consult your license for conditions of use.
// Developed by Jarl Lindrud.
// Contact: jlindrud@hotmail.com .
//******************************************************************************

#include <iostream>

#include <RCF/test/TestMinimal.hpp>
#include <RCF/test/TransportFactories.hpp>

#include <RCF/FilterService.hpp>
#include <RCF/Idl.hpp>
#include <RCF/NamedPipeEndpoint.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/ThreadManager.hpp>

#include <RCF/test/ThreadGroup.hpp>
#include <RCF/test/Usernames.hpp>

#include <RCF/util/Tchar.hpp>

#ifdef BOOST_WINDOWS
#include <RCF/SspiFilter.hpp>
#endif

class Echo
{
public:

    std::string echo(const std::string &s)
    {
        return s;
    }

    std::string echo(const std::string &s, boost::uint32_t waitMs)
    {
        Platform::OS::SleepMs(waitMs);
        return s;
    }

#ifdef BOOST_WINDOWS

    std::string whatsMyWin32UserName()
    {
        // Return clients username, by impersonating and then asking Windows
        // for the name of the currently logged on user.

        RCF::Win32NamedPipeImpersonator impersonator;
        RCF::tstring myUserName = RCF::getMyUserName();
        return util::toString(myUserName);
    }

#else

    std::string whatsMyWin32UserName()
    {
        return "";
    }

#endif

    RCF::ByteBuffer echo(RCF::ByteBuffer s)
    {
        return s;
    }
};

RCF_BEGIN(I_Echo, "I_Echo")
    RCF_METHOD_R1(std::string, echo, const std::string &)
    RCF_METHOD_R2(std::string, echo, const std::string &, boost::uint32_t)
    RCF_METHOD_R0(std::string, whatsMyWin32UserName)
    RCF_METHOD_R1(RCF::ByteBuffer, echo, RCF::ByteBuffer)
RCF_END(I_Echo)

RCF::Mutex gIoMutex;

void clientTask(const RCF::tstring & pipeName, boost::uint32_t waitMs)
{
    RCF::NamedPipeEndpoint ep(pipeName);
    RcfClient<I_Echo> client(ep);

    boost::uint32_t t0 = RCF::getCurrentTimeMs();

    std::string s1 = "asdf";
    std::string s2 = client.echo(s1, waitMs);
    RCF_CHECK(s1 == s2);

    boost::uint32_t t1 = RCF::getCurrentTimeMs();

    RCF::Lock lock(gIoMutex);
    std::cout << "Waited " << t1-t0 << " ms..." << std::endl;
}

int test_main(int, char**)
{

    RCF::RcfInitDeinit rcfinitDeinit;

#ifdef BOOST_WINDOWS
    RCF::tstring pipeName = RCF_T("TestPipe");
#else
    RCF::tstring pipeName = RCF_TEMP_DIR "TestPipe";
#endif

    RCF::NamedPipeEndpoint ep(pipeName);

    Echo echo;
    RCF::RcfServer server(ep);
    
    // 10 server threads
    server.setPrimaryThreadCount(10);

    server.bind( (I_Echo *) 0, echo);
    server.start();

    {
        // Make a single call, no waiting

        std::string s1 = "test";
        RCF::NamedPipeEndpoint ep(pipeName);
        RcfClient<I_Echo> client(ep);
        std::string s2 = client.echo(s1);
        RCF_CHECK( s1 == s2 );
    }

    {
        // Make 5 calls, in parallel, each one waiting 2 seconds

        boost::uint32_t t0 = RCF::getCurrentTimeMs();

        boost::uint32_t waitMs = 2000;
        ThreadGroup clients;
        for (std::size_t i=0; i<5; ++i)
        {
            clients.push_back( RCF::ThreadPtr( new RCF::Thread(
                boost::bind(clientTask, pipeName, waitMs))));
        }
        joinThreadGroup(clients);

        // Total time should be little more than 2 seconds
        boost::uint32_t t1 = RCF::getCurrentTimeMs();
        std::cout << "Total time: " << t1-t0 << " ms..." << std::endl;
        RCF_CHECK(t1-t0 < 2*waitMs);
    }

#ifdef BOOST_WINDOWS

    {
        // Named pipe impersonation - use our own credentials

        RCF::tstring tUserName = RCF::getMyUserName();
        std::string userName(util::toString(tUserName));
        RCF::NamedPipeEndpoint ep(pipeName);
        RcfClient<I_Echo> client(ep);
        std::string s = client.whatsMyWin32UserName();
        RCF_CHECK(s == userName);
    }

    {
        // SSPI impersonation - use our own credentials

        RCF::FilterServicePtr filterServicePtr(new RCF::FilterService());
        filterServicePtr->addFilterFactory( 
            RCF::FilterFactoryPtr( new RCF::NtlmFilterFactory()));
        server.addService(filterServicePtr);

        RCF::tstring tUserName = RCF::getMyUserName();
        std::string userName(util::toString(tUserName));
        RCF::NamedPipeEndpoint ep(pipeName);
        RcfClient<I_Echo> client(ep);

        RCF::FilterPtr ntlmFilterPtr( new RCF::NtlmFilter() );
        client.getClientStub().requestTransportFilters(ntlmFilterPtr);

        client.getClientStub().setRemoteCallTimeoutMs(1000*3600);
        std::string s = client.whatsMyWin32UserName();
        RCF_CHECK(s == userName);

        server.removeService(filterServicePtr);
    }

#endif

    {
        // Test error reporting, by exceeding the max message length.
        RCF::ByteBuffer byteBuffer(50*1024);
        for (std::size_t i=0; i<byteBuffer.getLength(); ++i)
        {
            byteBuffer.getPtr()[i] = i%256 ;
        }
        RCF::NamedPipeEndpoint ep(pipeName);
        RcfClient<I_Echo> client(ep);
        try
        {
            RCF_CHECK( byteBuffer == client.echo(byteBuffer) );
            RCF_CHECK(1==0);
        }
        catch(RCF::Exception &e)
        {
            RCF_CHECK(1==1);

#ifdef BOOST_WINDOWS
            RCF_CHECK( e.getErrorId() == RCF::RcfError_ClientWriteFail )(e.getError());
#else
            RCF_CHECK( e.getErrorId() == RCF::RcfError_ServerMessageLength )(e.getError());
#endif

        }

        // This time, turn up the message length and check that it goes through.
        server.getServerTransport().setMaxMessageLength(-1);
        client.getClientStub().getTransport().setMaxMessageLength(-1);
        RCF_CHECK( byteBuffer == client.echo(byteBuffer) );

    }

    /*
    {
        // Some performance testing. RCF Win32 named pipe server sessions have a 
        // reuse-instead-of-delete feature that needs some exercise.

        // Several threads, concurrently connecting and disconnecting, especially
        // disconnecting while a call is in progress.

        void clientTask2(const std::string & pipeName);

        std::vector<RCF::ThreadPtr> threads;
        for (std::size_t i=0; i<10; ++i)
        {
            threads.push_back( RCF::ThreadPtr( new RCF::Thread(
                boost::bind(clientTask2, pipeName))));
        }

        joinThreadGroup(threads);
    }
    */

    return 0;
}

void progressCallback(const boost::uint32_t & targetTimeMs, RCF::ClientProgress::Action &action)
{
    boost::uint32_t timeMs = RCF::getCurrentTimeMs();
    if (targetTimeMs - 100 <= timeMs && timeMs <= targetTimeMs + 100)
    {
        if (rand() % 100 < 50)
        {
            action = RCF::ClientProgress::Cancel;
            return;
        }
    }
    action = RCF::ClientProgress::Continue;
}

void clientTask2(const RCF::tstring & pipeName)
{
    RCF::NamedPipeEndpoint ep(pipeName);
    RcfClient<I_Echo> client(ep);

    std::string s1 = "asdf";
    boost::uint32_t waitMs = 1000;
    boost::uint32_t targetTimeMs = 0;

    RCF::ClientProgressPtr clientProgressPtr( new RCF::ClientProgress() );
    clientProgressPtr->mTriggerMask = RCF::ClientProgress::Timer;
    clientProgressPtr->mTimerIntervalMs = 50;
    clientProgressPtr->mProgressCallback = boost::bind(progressCallback, boost::cref(targetTimeMs), _5);
    client.getClientStub().setClientProgressPtr(clientProgressPtr);

    while (true)
    {
        try
        {
            targetTimeMs = RCF::getCurrentTimeMs() + waitMs;
            std::string s2 = client.echo(s1, waitMs);
        }
        catch(const RCF::Exception & e)
        {
            RCF_UNUSED_VARIABLE(e);
        }
    }
}
