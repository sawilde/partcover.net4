
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/TcpClientTransport.hpp>

#include <boost/bind.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/TimedBsdSockets.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/Tools.hpp>

#ifdef BOOST_WINDOWS
#include <RCF/Iocp.hpp>
#include <RCF/IocpServerTransport.hpp>
#endif

#include <RCF/RcfServer.hpp>

// missing stuff in mingw and vc6 headers
#if defined(__MINGW32__) || (defined(_MSC_VER) && _MSC_VER == 1200)

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

    TcpClientTransport::TcpClientTransport(const TcpClientTransport &rhs) :
        BsdClientTransport(rhs),
        mRemoteAddr(rhs.mRemoteAddr),
        mIp(rhs.mIp),
        mPort(rhs.mPort)
    {}

    void setupRemoteAddr(
        sockaddr_in &           remoteAddr, 
        const std::string &     ip, 
        int                     port)
    {
        memset(&remoteAddr, 0, sizeof(remoteAddr));

        // Returns INADDR_NONE if mIp is not a dotted ip.
        unsigned long ul_addr = ::inet_addr(ip.c_str());

        // MSDN tells us INADDR_ANY may be returned in some circumstances
        // on XP and earlier.
        if (ul_addr == INADDR_ANY)
        {
            ul_addr = INADDR_NONE;
        }

        memset(&remoteAddr, 0, sizeof(remoteAddr));
        remoteAddr.sin_family = AF_INET;
        remoteAddr.sin_addr.s_addr = ul_addr;
        // the :: seems to screw up gcc (!?!)
        //remoteAddr.sin_port = ::htons(mPort);
        remoteAddr.sin_port = htons( static_cast<u_short>(port) );
    }


    TcpClientTransport::TcpClientTransport(const std::string &ip, int port) :
        BsdClientTransport(),
        mRemoteAddr(),
        mIp(ip),
        mPort(port)
    {
        setupRemoteAddr(mRemoteAddr, mIp, mPort);
    }

    TcpClientTransport::TcpClientTransport(const sockaddr_in &remoteAddr) :
        BsdClientTransport(),
        mRemoteAddr(remoteAddr),
        mIp(),
        mPort(RCF_DEFAULT_INIT)
    {}

    TcpClientTransport::TcpClientTransport(int fd) :
        BsdClientTransport(fd),
        mRemoteAddr(),
        mIp(),
        mPort(RCF_DEFAULT_INIT)
    {
        setupRemoteAddr(mRemoteAddr, mIp, mPort);
    }

    int TcpClientTransport::getSocket()
    {
        return mFd;
    }

    TcpClientTransport::~TcpClientTransport()
    {
        RCF_DTOR_BEGIN
            if (mOwn)
            {
                close();
            }
        RCF_DTOR_END
    }

    std::auto_ptr<I_ClientTransport> TcpClientTransport::clone() const
    {
        return ClientTransportAutoPtr( new TcpClientTransport(*this) );
    }

    void TcpClientTransport::createSocket()
    {
        mFd = static_cast<int>( ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP) );
        int err = Platform::OS::BsdSockets::GetLastError();

        RCF_VERIFY(
            mFd != -1,
            Exception(
                _RcfError_Socket(), err, RcfSubsystem_Os, "socket() failed"));

        Platform::OS::BsdSockets::setblocking(mFd, false);
    }

    void TcpClientTransport::implConnect(
        I_ClientTransportCallback &clientStub,
        unsigned int timeoutMs)
    {
        // TODO: replace throw with return, where possible

        RCF_ASSERT(!mAsync);

        // TODO: this should not be necessary, should be able to assume that
        // it is closed already.
        // close the current connection
        implClose();

        createSocket();

        if (mRemoteAddr.sin_addr.s_addr == INADDR_NONE)
        {
            hostent *hostDesc = ::gethostbyname( mIp.c_str() );
            if (hostDesc)
            {
                char *szIp = ::inet_ntoa( * (in_addr*) hostDesc->h_addr_list[0]);
                RCF_VERIFY(szIp, Exception(_RcfError_DnsLookup(mIp), mIp));
                mRemoteAddr.sin_addr.s_addr = ::inet_addr(szIp);
            }
        }

        unsigned int startTimeMs = getCurrentTimeMs();
        mEndTimeMs = startTimeMs + timeoutMs;

        PollingFunctor pollingFunctor(
            mClientProgressPtr,
            ClientProgress::Connect,
            mEndTimeMs);

        int err = 0;

        int ret = timedConnect(
            pollingFunctor,
            err,
            mFd,
            (sockaddr*) &mRemoteAddr,
            sizeof(mRemoteAddr));

        if (ret != 0)
        {
            close();

            if (err == 0)
            {
                RCF_THROW(
                    Exception( _RcfError_ClientConnectTimeout(
                        timeoutMs, 
                        TcpEndpoint(mIp, mPort).asString()) ));
            }
            else
            {
                RCF_THROW(
                    Exception( _RcfError_ClientConnectFail(), err, RcfSubsystem_Os))
                    (mIp)(mPort);
            }
        }

        clientStub.onConnectCompleted();
    }

#ifdef BOOST_WINDOWS

    void TcpClientTransport::dnsLookupTask(
        OverlappedAmiPtr overlappedPtr,
        const std::string & ip)
    {
        
        setWin32ThreadName( DWORD(-1), "RCF DNS Lookup Thread");

        unsigned long ul_addr = INADDR_NONE;

        hostent * hostDesc = ::gethostbyname( ip.c_str() );
        if (hostDesc)
        {
            char *szIp = ::inet_ntoa( * (in_addr*) hostDesc->h_addr_list[0]);

            // Returns INADDR_NONE if mIp is not a dotted ip.
            ul_addr = ::inet_addr(szIp);

            // MSDN tells us INADDR_ANY may be returned in some circumstances
            // on XP and earlier.
            if (ul_addr == INADDR_ANY)
            {
                ul_addr = INADDR_NONE;
            }            
        }

        Lock lock(overlappedPtr->mMutex);

        TcpClientTransport *pTcpClientTransport = 
            static_cast<TcpClientTransport *>(overlappedPtr->mpTransport);

        if (pTcpClientTransport)
        {
            pTcpClientTransport->mRemoteAddr.sin_addr.s_addr = ul_addr;
            pTcpClientTransport->endDnsLookup();
        }
    }

    void TcpClientTransport::beginDnsLookup()
    {
        // Fire and forget.
        Thread thread( boost::bind( 
            &TcpClientTransport::dnsLookupTask, 
            getOverlappedPtr(), 
            mIp));
    }

    void TcpClientTransport::endDnsLookup()
    {
        if (mRemoteAddr.sin_addr.s_addr == INADDR_NONE)
        {
            mClientStubCallbackPtr.onError(Exception(_RcfError_DnsLookup(mIp), mIp));
            return;
        }

        OverlappedAmiPtr overlappedPtr = getOverlappedPtr();

        overlappedPtr->mThisPtr = overlappedPtr;

        using namespace boost::multi_index::detail;
        scope_guard clearSelfReferenceGuard =
            make_guard(clearSelfReference, boost::ref(overlappedPtr->mThisPtr));

        RCF::Exception e = gAmiThreadPoolPtr->connect(mFd, mRemoteAddr, overlappedPtr);
        if (e.bad())
        {
            mClientStubCallbackPtr.onError(e);
            return;
        }

        clearSelfReferenceGuard.dismiss();
    }

    void TcpClientTransport::implConnectAsync(
        I_ClientTransportCallback &clientStub,
        unsigned int timeoutMs)
    {
        // TODO: sort this out
        RCF_UNUSED_VARIABLE(timeoutMs);

        RCF_ASSERT(mAsync);

        implClose();

        createSocket();

        if (!mRegisteredForAmi)
        {

            RcfServer * preferred = clientStub.getAsyncDispatcher();
            if (preferred)
            {
                I_ServerTransport & transport = preferred->getServerTransport();
                IocpServerTransport & iocpTransport = dynamic_cast<IocpServerTransport &>(transport);
                iocpTransport.associateSocket(mFd);
            }
            else
            {
                gAmiThreadPoolPtr->mIocp.AssociateSocket(mFd, mFd);
            }

            mRegisteredForAmi = true;
        }

        mClientStubCallbackPtr.reset(&clientStub);

        if (mRemoteAddr.sin_addr.s_addr == INADDR_NONE)
        {
            beginDnsLookup();
        }
        else
        {
            endDnsLookup();
        }
    }

#else

    void TcpClientTransport::implConnectAsync(
        I_ClientTransportCallback &clientStub,
        unsigned int timeoutMs)
    {
        RCF_ASSERT(0);
    }

    void TcpClientTransport::endDnsLookup()
    {
        RCF_ASSERT(0);
    }

    void TcpClientTransport::beginDnsLookup()
    {
        RCF_ASSERT(0);
    }

    void TcpClientTransport::dnsLookupTask(
        OverlappedAmiPtr overlappedPtr,
        const std::string & ip)
    {
        RCF_ASSERT(0);
    }

#endif

    void TcpClientTransport::implClose()
    {
        if (mFd != -1)
        {
            if (mRegisteredForAmi && gAmiThreadPoolPtr.get())
            {
                gAmiThreadPoolPtr->cancelConnect(mFd);
            }

            int ret = Platform::OS::BsdSockets::closesocket(mFd);
            int err = Platform::OS::BsdSockets::GetLastError();

            RCF_VERIFY(
                ret == 0,
                Exception(
                _RcfError_Socket(),
                err,
                RcfSubsystem_Os,
                "closesocket() failed"))
                (mFd);
        }

        mFd = -1;
    }



    EndpointPtr TcpClientTransport::getEndpointPtr() const
    {
        return EndpointPtr( new TcpEndpoint(mIp, mPort) );
    }

    void TcpClientTransport::setRemoteAddr(const sockaddr_in &remoteAddr)
    {
        mRemoteAddr = remoteAddr;
    }

    const sockaddr_in &TcpClientTransport::getRemoteAddr() const
    {
        return mRemoteAddr;
    }

} // namespace RCF
