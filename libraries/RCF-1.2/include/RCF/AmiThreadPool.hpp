
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_AMITHREADPOOL_HPP
#define INCLUDE_RCF_AMITHREADPOOL_HPP

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

#include <RCF/Heap.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/Tools.hpp>

#ifdef BOOST_WINDOWS

#include <RCF/Iocp.hpp>

namespace RCF {

    class OverlappedAmi;
    typedef boost::shared_ptr<OverlappedAmi> OverlappedAmiPtr;

    class I_OverlappedAmi : public IocpOverlapped
    {
    public:
        virtual void onCompletion(
            BOOL ret, 
            DWORD dwErr,
            ULONG_PTR completionKey, 
            DWORD dwNumBytes)
        {
            RCF_UNUSED_VARIABLE(ret);
            RCF_UNUSED_VARIABLE(dwErr);
            RCF_UNUSED_VARIABLE(completionKey);

            onCompletion(dwNumBytes);
        }

        virtual void onCompletion(std::size_t numBytes) = 0;

        virtual void onError(const RCF::Exception & e) = 0;

        typedef std::pair<boost::uint32_t, OverlappedAmiPtr> TimerEntry;

        virtual void onTimerExpired(const TimerEntry & timerEntry) = 0;
    };

    class RCF_EXPORT AmiThreadPool
    {
    public:
        AmiThreadPool();
        ~AmiThreadPool();

        static void start(std::size_t threadCount);
        static void stop();

        typedef std::pair<boost::uint32_t, OverlappedAmiPtr> TimerEntry;

        TimerEntry addTimerEntry(
            OverlappedAmiPtr overlappedAmiptr, 
            boost::uint32_t timeoutMs);

        void removeTimerEntry(
            const TimerEntry & timerEntry);

        // TODO: make private (currently used to associate sockets with the iocp)
        Iocp                            mIocp;

        Exception connect(int fd, sockaddr_in & addr, OverlappedAmiPtr overlappedAmiPtr);
        void cancelConnect(int fd);
          
    private:

        void cycleIo();
        void cycleTimer();
        void cycleConnect();

        Exception addConnectFd(int fd, OverlappedAmiPtr overlappedAmiPtr);
        void removeConnectFd(int fd);


        
        RCF::ThreadPtr                  mIoThreadPtr;
        RCF::ThreadPtr                  mTimerThreadPtr;
        RCF::ThreadPtr                  mConnectThreadPtr;
        bool                            mStopFlag;

        Mutex                           mTimerMutex;
        Condition                       mTimerCondition;

        TimerHeap<OverlappedAmiPtr>     mTimerHeap;

        typedef std::map<int,OverlappedAmiPtr> Fds;

        Mutex                           mConnectFdsMutex;
        Fds                             mConnectFds;
    };

    
    RCF_EXPORT void amiInit();
    RCF_EXPORT void amiDeinit();

    class RCF_EXPORT AmiInitDeinit
    {
    public:
        AmiInitDeinit()
        {
            amiInit();
        }
        ~AmiInitDeinit()
        {
            amiDeinit();
        }
    };

} // namespace RCF

#else // BOOST_WINDOWS

namespace RCF {

    // TODO: Non-Windows implementation.

    class OverlappedAmi;
    typedef boost::shared_ptr<OverlappedAmi> OverlappedAmiPtr;

    class I_OverlappedAmi
    {
    public:
        I_OverlappedAmi() {}
        virtual ~I_OverlappedAmi() {}
        virtual void onCompletion(std::size_t numBytes) = 0;

        typedef std::pair<boost::uint32_t, OverlappedAmiPtr> TimerEntry;

        virtual void onTimerExpired(const TimerEntry & timerEntry) = 0;
    };

    class AmiThreadPool
    {
    public:
        void cancelConnect(int fd)
        {
        }
    };


} // namespace RCF

#endif // !BOOST_WINDOWS

namespace RCF {

    extern boost::scoped_ptr<AmiThreadPool> gAmiThreadPoolPtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_AMITHREADPOOL_HPP
