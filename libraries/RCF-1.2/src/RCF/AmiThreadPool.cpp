
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/AmiThreadPool.hpp>

#include <RCF/ConnectionOrientedClientTransport.hpp>
#include <RCF/Exception.hpp>
#include <RCF/ThreadManager.hpp>

#ifdef BOOST_WINDOWS

#if defined(__MINGW32__) || (defined(_MSC_VER) && _MSC_VER == 1200) || (defined(_WIN32_WINNT) && _WIN32_WINNT <= 0x0500)

typedef
BOOL
(PASCAL FAR * LPFN_CONNECTEX) (
                               IN SOCKET s,
                               IN const struct sockaddr FAR *name,
                               IN int namelen,
                               IN PVOID lpSendBuffer OPTIONAL,
                               IN DWORD dwSendDataLength,
                               OUT LPDWORD lpdwBytesSent,
                               IN LPOVERLAPPED lpOverlapped
                               );

#define WSAID_CONNECTEX \
{0x25a207b9,0xddf3,0x4660,{0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}}

#endif


namespace RCF {

    Mutex gAmiInitRefCountMutex;
    std::size_t gAmiInitRefCount = 0;

    void amiInit()
    {
        Lock lock(gAmiInitRefCountMutex);
        if (gAmiInitRefCount == 0)
        {
            gAmiThreadPoolPtr.reset( new AmiThreadPool() );
            AmiThreadPool::start(1);
        }
        ++gAmiInitRefCount;
    }

    void amiDeinit()
    {
        Lock lock(gAmiInitRefCountMutex);
        --gAmiInitRefCount;
        if (gAmiInitRefCount == 0)
        {
            AmiThreadPool::stop();
            gAmiThreadPoolPtr.reset();
        }
    }

    AmiThreadPool::AmiThreadPool() :
        mStopFlag()
    {
    }

    AmiThreadPool::~AmiThreadPool()
    {
    }

    void AmiThreadPool::cycleIo()
    {
        // Set our thread name.
        setWin32ThreadName( DWORD(-1), "RCF AMI IO Thread");

        while (!mStopFlag)
        {
            try
            {
                DWORD           dwMilliseconds  = 500;
                DWORD           dwNumBytes      = 0;
                ULONG_PTR       completionKey   = 0;
                OVERLAPPED *    pOverlapped     = 0;

                BOOL ret = mIocp.GetStatus(
                    &completionKey, 
                    &dwNumBytes, 
                    &pOverlapped, 
                    dwMilliseconds);

                DWORD dwErr= GetLastError();

                // TODO: proper error handling here.
                // ...

                RCF_UNUSED_VARIABLE(ret);

                RCF_ASSERT(
                    pOverlapped || (!pOverlapped && dwErr == WAIT_TIMEOUT))
                    (pOverlapped)(dwErr);

                I_OverlappedAmi *pAmi = static_cast<I_OverlappedAmi *>(pOverlapped);

                if (pAmi && ret)
                {        
                    pAmi->onCompletion(dwNumBytes);
                }
                else if (pAmi && !ret)
                {
                    RCF::Exception e(_RcfError_Socket(), dwErr);
                    pAmi->onError(e);
                }
            }
            catch(...)
            {
            }
        }
    }

    AmiThreadPool::TimerEntry AmiThreadPool::addTimerEntry(
        OverlappedAmiPtr overlappedAmiptr, 
        boost::uint32_t timeoutMs)
    {
        Lock lock(mTimerMutex);
        boost::uint32_t timeNowMs = Platform::OS::getCurrentTimeMs();
        boost::uint32_t endTimeMs = timeNowMs + timeoutMs;

        TimerEntry timerEntry(endTimeMs, overlappedAmiptr);
        mTimerHeap.add(timerEntry);
        mTimerCondition.notify_all();
        return timerEntry;
    }

    void AmiThreadPool::removeTimerEntry(
        const TimerEntry & timerEntry)
    {
        mTimerHeap.remove(timerEntry);
    }

    void AmiThreadPool::cycleTimer()
    {
        // Set our thread name.
        setWin32ThreadName( DWORD(-1), "RCF AMI Timer Thread");

        while (!mStopFlag)
        {

            TimerEntry timerEntry;

            while (mTimerHeap.popExpiredEntry(timerEntry))
            {
                OverlappedAmiPtr overlappedAmiPtr = timerEntry.second;

                RCF_ASSERT(overlappedAmiPtr);

                overlappedAmiPtr->onTimerExpired(timerEntry);
            }

            boost::uint32_t timeoutMs = RCF_MIN(
                boost::uint32_t(1000), 
                mTimerHeap.getNextEntryTimeoutMs());

            if (!mStopFlag)
            {
                Lock lock(mTimerMutex);
                mTimerCondition.timed_wait(lock, timeoutMs);
            }

        }
    }

    void AmiThreadPool::start(std::size_t threadCount)
    {
        RCF_UNUSED_VARIABLE(threadCount);

        AmiThreadPool & amiThreadPool = *gAmiThreadPoolPtr;

        amiThreadPool.mStopFlag = false;

        amiThreadPool.mIoThreadPtr.reset( new RCF::Thread( boost::bind(
            &AmiThreadPool::cycleIo,
            &amiThreadPool)));

        amiThreadPool.mTimerThreadPtr.reset( new RCF::Thread( boost::bind(
            &AmiThreadPool::cycleTimer,
            &amiThreadPool)));

        amiThreadPool.mConnectThreadPtr.reset( new RCF::Thread( boost::bind(
            &AmiThreadPool::cycleConnect,
            &amiThreadPool)));
    }

    void AmiThreadPool::stop()
    {
        AmiThreadPool & amiThreadPool = *gAmiThreadPoolPtr;

        amiThreadPool.mStopFlag = true;

        amiThreadPool.mIoThreadPtr->join();
        amiThreadPool.mIoThreadPtr.reset();

        amiThreadPool.mTimerThreadPtr->join();
        amiThreadPool.mTimerThreadPtr.reset();

        amiThreadPool.mConnectThreadPtr->join();
        amiThreadPool.mConnectThreadPtr.reset();
    }


    Exception AmiThreadPool::connect(int fd, sockaddr_in & addr, OverlappedAmiPtr overlappedAmiPtr)
    {
        bool useConnectEx = false;

        if (useConnectEx)
        {
            // TODO: need to figure out which local network interface to bind to!
            sockaddr_in local;
            local.sin_family = AF_INET;
            local.sin_addr.s_addr = inet_addr("127.0.0.1");
            local.sin_port = htons(0);
            int ret = ::bind(fd, (sockaddr *) &local, sizeof(local));
            int err = WSAGetLastError();

            if (ret != 0)
            {
                return Exception(_RcfError_Socket(), err);
            }

            LPFN_CONNECTEX lpfnConnectEx = NULL;

            GUID GuidConnectEx = WSAID_CONNECTEX;
            DWORD dwBytes = 0;

            ret = WSAIoctl(
                fd,
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                &GuidConnectEx,
                sizeof(GuidConnectEx),
                &lpfnConnectEx,
                sizeof(lpfnConnectEx),
                &dwBytes,
                NULL,
                NULL);

            err = Platform::OS::BsdSockets::GetLastError();

            if (ret != 0)
            {
                return Exception(_RcfError_Socket(), err);
            }
            
            DWORD dwSent = 0;

            BOOL ok = lpfnConnectEx(
                fd,
                (sockaddr *) &addr,
                sizeof(addr),
                NULL,
                0,
                &dwSent,
                overlappedAmiPtr.get());

            err = WSAGetLastError();

            if (ok || (!ok && err == ERROR_IO_PENDING))
            {
                // OK
            }
            else
            {
                return Exception(_RcfError_Socket(), err);
            }

            // TODO: according to MSDN docs for ConnectEx(), we need to call the
            // the following code when the connect completes:

            //err = setsockopt( s,
            //    SOL_SOCKET,
            //    SO_UPDATE_CONNECT_CONTEXT,
            //    NULL,
            //    0 );            

        }
        else
        {
            int ret = Platform::OS::BsdSockets::connect(
                fd, 
                (sockaddr *) &addr, 
                sizeof(addr));

            int err = Platform::OS::BsdSockets::GetLastError();

            RCF_ASSERT( ret != 0 );

            if (
                    (ret == -1 && err == Platform::OS::BsdSockets::ERR_EWOULDBLOCK)
                ||  (ret == -1 && err == Platform::OS::BsdSockets::ERR_EINPROGRESS))
            {
                return addConnectFd(fd, overlappedAmiPtr);
            }
            else
            {
                return Exception(_RcfError_Socket(), err);
            }
        }

        return Exception();
    }

    void AmiThreadPool::cancelConnect(int fd)
    {
        OverlappedAmiPtr overlappedAmiPtr;
        {
            Lock lock(mConnectFdsMutex);
            Fds::iterator iter = mConnectFds.find(fd);
            if (iter != mConnectFds.end())
            {
                overlappedAmiPtr = iter->second;
            }
        }
        if (overlappedAmiPtr)
        {
            Lock lock(overlappedAmiPtr->mMutex);
            overlappedAmiPtr->mThisPtr.reset();
        }

        removeConnectFd(fd);
    }

    Exception AmiThreadPool::addConnectFd(int fd, OverlappedAmiPtr overlappedAmiPtr)
    {
        Lock lock(mConnectFdsMutex);
        RCF_ASSERT(mConnectFds.find(fd) == mConnectFds.end());

        if (mConnectFds.size() >= FD_SETSIZE)
        {
            std::ostringstream os;
            os << "FD_SETSIZE = " << FD_SETSIZE;
            std::string msg = os.str();

            return Exception(_RcfError_FdSetSize(FD_SETSIZE), msg);
        }

        mConnectFds.insert( std::make_pair(fd, overlappedAmiPtr) );
        return Exception();
        
    }

    void AmiThreadPool::removeConnectFd(int fd)
    {
        Lock lock(mConnectFdsMutex);
        mConnectFds.erase(fd);
    }

    struct Event
    {
        Event() : 
            mFd(RCF_DEFAULT_INIT), 
            mError(RCF_DEFAULT_INIT)
        {
        }

        Event(int fd, int error, OverlappedAmiPtr overlappedAmiPtr) :
            mFd(fd),
            mError(error),
            mOverlappedPtr(overlappedAmiPtr)
        {
        }

        int                 mFd;
        int                 mError;
        OverlappedAmiPtr    mOverlappedPtr;
    };

#if defined(_MSC_VER) && _MSC_VER <= 1200
#define for if (0) {} else for
#endif

    void AmiThreadPool::cycleConnect()
    {
        // Set our thread name.
        setWin32ThreadName( DWORD(-1), "RCF AMI Connect Thread");

        while (!mStopFlag)
        {
            int maxFd = 0;

            fd_set fdSet;
            
            FD_ZERO(&fdSet);

            {
                Lock lock(mConnectFdsMutex);

                RCF_ASSERT(mConnectFds.size() <= FD_SETSIZE)(FD_SETSIZE);

                for (Fds::iterator iter = mConnectFds.begin(); iter != mConnectFds.end(); ++iter)
                {
                    int fd = iter->first;

                    if (fd > maxFd)
                    {
                        maxFd = fd;
                    }
                    
                    SOCKET sock = static_cast<SOCKET>(fd); 
                    FD_SET( sock , &fdSet);
                }                
            }

            // TODO: add a handle that we can use to cut the timeout short.
            // ...

            unsigned int timeoutMs = 100;

            timeval timeout = {0};
            timeout.tv_sec = timeoutMs/1000;
            timeout.tv_usec = 1000*(timeoutMs%1000);

            int readyCount = select(maxFd + 1, NULL, &fdSet, NULL, &timeout);
            int err = Platform::OS::BsdSockets::GetLastError();

            if (readyCount == -1)
            {
                // Either no file descriptors at all, or some are duds. Sleep a little so we 
                // don't get into a hot loop.
                Sleep(100);
            }

            typedef std::vector<Event> Events;
            Events events;

            if (readyCount > 0)
            {
                Lock lock(mConnectFdsMutex);
                for (Fds::iterator iter = mConnectFds.begin(); iter != mConnectFds.end(); ++iter)
                {
                    int fd = iter->first;
                    OverlappedAmiPtr overlappedAmiPtr = iter->second;
                    if ( FD_ISSET(iter->first, &fdSet) )
                    {
                        int connectError = 0;
                        Platform::OS::BsdSockets::socklen_t len = sizeof(int); 
                        int ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)(&connectError), &len);
                        err = Platform::OS::BsdSockets::GetLastError();
                        RCF_ASSERT( ret == 0 );

                        if (    connectError == Platform::OS::BsdSockets::ERR_EWOULDBLOCK
                            ||  connectError == Platform::OS::BsdSockets::ERR_EINPROGRESS)
                        {
                            continue;
                        }
                        
                        events.push_back( Event(fd, connectError, overlappedAmiPtr));
                    }
                }
            }

            for (Events::iterator iter = events.begin(); iter != events.end(); ++iter)
            {
                int fd = iter->mFd;
                removeConnectFd(fd);
            }

            for (Events::iterator iter = events.begin(); iter != events.end(); ++iter)
            {
                
                int fd = iter->mFd;
                RCF_UNUSED_VARIABLE(fd);

                int error = iter->mError;
                OverlappedAmiPtr overlappedAmiPtr = iter->mOverlappedPtr;

                // Call the callback, either OK or with an error.
                if (error == 0)
                {
                    overlappedAmiPtr->onCompletion(0);
                }
                else
                {
                    RCF::Exception e(_RcfError_Socket(), error);
                    overlappedAmiPtr->onError(e);
                }
            }            
        }
    }

#if defined(_MSC_VER) && _MSC_VER <= 1200
#undef for
#endif

} // namespace RCF

#endif // BOOST_WINDOWS

namespace RCF {

    boost::scoped_ptr<AmiThreadPool> gAmiThreadPoolPtr;

} // namespace RCF
