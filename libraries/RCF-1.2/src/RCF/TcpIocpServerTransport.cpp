
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/TcpIocpServerTransport.hpp>

#include <RCF/TcpClientTransport.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/TimedBsdSockets.hpp>

namespace RCF {

    void resetSessionStatePtr(IocpSessionStatePtr &sessionStatePtr);

    // TcpIocpSessionState

    TcpIocpSessionState::TcpIocpSessionState(
        TcpIocpServerTransport & transport) : 
            IocpSessionState(transport),
            mTransport(transport),
            mFd(-1)
    {}

    TcpIocpSessionState::TcpIocpSessionStatePtr 
    TcpIocpSessionState::create(
        TcpIocpServerTransport & transport)
    {
        TcpIocpSessionStatePtr sessionStatePtr( 
            new TcpIocpSessionState(transport));

        SessionPtr sessionPtr = transport.getSessionManager().createSession();
        sessionPtr->setProactor(*sessionStatePtr);
        sessionStatePtr->mSessionPtr = sessionPtr;
        sessionStatePtr->mWeakThisPtr = IocpSessionStateWeakPtr(sessionStatePtr);

        return sessionStatePtr;
    }

    TcpIocpSessionState::~TcpIocpSessionState()
    {
        RCF_DTOR_BEGIN

            // close the socket, if appropriate
            RCF2_TRACE("")(mFd);
            RCF_ASSERT(mFd != -1);
            postClose();

            // adjust number of queued accepts, if appropriate
            if (mPreState == IocpSessionState::Accepting)
            {
                // TODO: this code should be in the transport
                Lock lock(mTransport.mQueuedAcceptsMutex);
                --mTransport.mQueuedAccepts;
                if (mTransport.mQueuedAccepts < mTransport.mQueuedAcceptsThreshold)
                {
                    mTransport.mQueuedAcceptsCondition.notify_one();
                }
            }

        RCF_DTOR_END
    }

    const I_RemoteAddress & TcpIocpSessionState::getRemoteAddress()
    {
        return mRemoteAddress;
    }

    void TcpIocpSessionState::accept()
    {
        mTransport.registerSession(mWeakThisPtr);

        Fd fd = static_cast<Fd>( socket(
            AF_INET,
            SOCK_STREAM,
            IPPROTO_TCP));

        int error = Platform::OS::BsdSockets::GetLastError();

        RCF_VERIFY(
            fd != -1,
            Exception(
                _RcfError_Socket(),
                error,
                RcfSubsystem_Os,
                "socket() failed"));

        Platform::OS::BsdSockets::setblocking(fd, false);

        mFd = fd;

        DWORD dwBytes = 0;

        for (std::size_t i=0; i<mAcceptExBuffer.size(); ++i)
        {
            mAcceptExBuffer[i] = 0;
        }

        clearOverlapped();

        mThisPtr = mWeakThisPtr.lock();

        BOOL ret = mTransport.mlpfnAcceptEx(
            mTransport.mAcceptorFd,
            mFd,
            &mAcceptExBuffer[0],
            0,
            sizeof(sockaddr_in) + 16,
            sizeof(sockaddr_in) + 16,
            &dwBytes,
            static_cast<OVERLAPPED *>(this));

        int err = WSAGetLastError();

        if (ret == FALSE && err == ERROR_IO_PENDING)
        {
            // async accept initiated successfully
            
        }
        else if (dwBytes > 0)
        {
            RCF_ASSERT(0);
            mThisPtr.reset();
            transition();
        }
        else
        {
            mThisPtr.reset();
            int err = Platform::OS::BsdSockets::GetLastError();
            RCF_THROW(Exception(
                _RcfError_Socket(),
                err,
                RcfSubsystem_Os,
                "AcceptEx() failed"))
            (err);
        }
    }

    void TcpIocpSessionState::implOnAccept()
    {
        {
            // associate fd with iocp
            int fd = mFd;
            mTransport.mIocpAutoPtr->AssociateSocket(fd, 0);            
        }

        // check the connection limit
        {
            bool allowConnect = true;
            {
                Lock lock(mTransport.mQueuedAcceptsMutex);
                --mTransport.mQueuedAccepts;
                if (mTransport.mQueuedAccepts < mTransport.mQueuedAcceptsThreshold)
                {
                    mTransport.mQueuedAcceptsCondition.notify_one();
                }
                if (mTransport.mQueuedAccepts == 0)
                {
                    std::size_t connectionLimit = mTransport.getConnectionLimit();
                    if (connectionLimit)
                    {
                        Lock lock(mTransport.mSessionsMutex);
                        if (mTransport.mSessions.size() >= 1 + connectionLimit)
                        {
                            allowConnect = false;
                        }
                    }
                }
            }

            if (!allowConnect)
            {
                sendServerError(RcfError_ConnectionLimitExceeded);
                return;
            }
        }

        // parse the local and remote address info
        {
            SOCKADDR *pLocalAddr = NULL;
            SOCKADDR *pRemoteAddr = NULL;

            int localAddrLen = 0;
            int remoteAddrLen = 0;

            RCF_ASSERT(mTransport.mlpfnGetAcceptExSockAddrs);
            mTransport.mlpfnGetAcceptExSockAddrs(
                &mAcceptExBuffer[0],
                0,
                sizeof(sockaddr_in) + 16,
                sizeof(sockaddr_in) + 16,
                &pLocalAddr,
                &localAddrLen,
                &pRemoteAddr,
                &remoteAddrLen);

            sockaddr_in *pLocalSockAddr =
                reinterpret_cast<sockaddr_in *>(pLocalAddr);

            mLocalAddress = IpAddress(*pLocalSockAddr);

            sockaddr_in *pRemoteSockAddr =
                reinterpret_cast<sockaddr_in *>(pRemoteAddr);

            mRemoteAddress = IpAddress(*pRemoteSockAddr);
        }

        // is this ip allowed?
        if (mTransport.isClientAddrAllowed(mRemoteAddress.getSockAddr()))
        {
            // simulate a completed write to kick things off
            mPreState = IocpSessionState::WritingData;
            mWriteBufferRemaining = 0;
            transition();
        }

    }

    void TcpIocpSessionState::implRead(
        const ByteBuffer &byteBuffer,
        std::size_t bufferLen)
    {
        WSAOVERLAPPED *pOverlapped = static_cast<WSAOVERLAPPED *>(this);

        bufferLen = RCF_MIN(mTransport.getMaxSendRecvSize(), bufferLen);
        WSABUF wsabuf = {0};
        wsabuf.buf = byteBuffer.getPtr();
        wsabuf.len = static_cast<u_long>(bufferLen);
        DWORD dwReceived = 0;
        DWORD dwFlags = 0;
        mError = 0;
        mPostState = Reading;

        // set self-reference
        RCF_ASSERT(!mThisPtr.get());
        mThisPtr = mWeakThisPtr.lock();
        RCF_ASSERT(mThisPtr.get());

        using namespace boost::multi_index::detail;
        scope_guard clearSelfReferenceGuard =
            make_guard(resetSessionStatePtr, boost::ref(mThisPtr));

        RCF2_TRACE("calling WSARecv()")(wsabuf.len);

        Lock lock(mMutex);

        if (!mHasBeenClosed)
        {

            int ret = WSARecv(
                mFd,
                &wsabuf,
                1,
                &dwReceived,
                &dwFlags,
                pOverlapped,
                NULL);

            if (ret == -1)
            {
                mError = WSAGetLastError();
            }

            RCF_ASSERT(ret == -1 || ret == 0);
            if (mError == S_OK || mError == WSA_IO_PENDING)
            {
                mError = 0;
                clearSelfReferenceGuard.dismiss();
            }
        }
    }

    void TcpIocpSessionState::implWrite(
        const std::vector<ByteBuffer> &byteBuffers,
        IocpSessionState * pReflectee)
    {
        WSAOVERLAPPED *pOverlapped = 
            static_cast<WSAOVERLAPPED *>(this);

        TcpIocpSessionState * pTcpReflectee = 
            static_cast<TcpIocpSessionState *>(pReflectee);

        std::size_t bytesAdded = 0;

        mWsabufs.resize(0);
        for (std::size_t i=0; i<byteBuffers.size(); ++i)
        {
            std::size_t bytesToAdd = RCF_MIN(
                byteBuffers[i].getLength(),
                mTransport.getMaxSendRecvSize() - bytesAdded);

            if (bytesToAdd > 0)
            {
                WSABUF wsabuf = {0};
                wsabuf.buf = byteBuffers[i].getPtr();
                wsabuf.len = static_cast<u_long>(bytesToAdd);
                mWsabufs.push_back(wsabuf);
                bytesAdded += bytesToAdd;
            }
        }

        DWORD dwSent = 0;
        DWORD dwFlags = 0;
        mError = 0;
        mPostState = Writing;

        // set self-reference
        RCF_ASSERT(!mThisPtr.get());
        mThisPtr = mWeakThisPtr.lock();
        RCF_ASSERT(mThisPtr.get());

        using namespace boost::multi_index::detail;
        scope_guard clearSelfReferenceGuard =
            make_guard(resetSessionStatePtr, boost::ref(mThisPtr));

        RCF2_TRACE("calling WSASend()")
            (RCF::lengthByteBuffers(byteBuffers))(bytesAdded);

        Mutex & mutex = pReflectee ? pReflectee->mMutex : mMutex;
        bool & hasBeenClosed = pReflectee ? pReflectee->mHasBeenClosed : mHasBeenClosed;
        Fd & fd = pReflectee ? pTcpReflectee->mFd : mFd;

        Lock lock(mutex);

        if (!hasBeenClosed)
        {
            int ret = WSASend(
                fd,
                &mWsabufs[0],
                static_cast<DWORD>(mWsabufs.size()),
                &dwSent,
                dwFlags,
                pOverlapped,
                NULL);

            if (ret == -1)
            {
                mError = WSAGetLastError();
            }

            RCF_ASSERT(ret == -1 || ret == 0);
            if (mError == S_OK || mError == WSA_IO_PENDING)
            {
                clearSelfReferenceGuard.dismiss();
                mError = 0;
            }
        }
    }

    void TcpIocpSessionState::implDelayCloseAfterSend()
    {
        Fd fd = mFd;
        const int BufferSize = 8*1024;
        char buffer[BufferSize];
        // TODO: upper limit on iterations?
        // Better to do it async, actually.
        while (recv(fd, buffer, BufferSize, 0) > 0);

        Sleep(0);
    }

    void TcpIocpSessionState::implClose()
    {
        int ret = Platform::OS::BsdSockets::closesocket(mFd);
        int err = Platform::OS::BsdSockets::GetLastError();

        RCF_VERIFY(
            ret == 0,
            Exception(
                _RcfError_SocketClose(),
                err,
                RcfSubsystem_Os,
                "closesocket() failed"))
            (mFd);

        mHasBeenClosed = true;
    }

    Fd TcpIocpSessionState::getNativeHandle()
    {
        return mFd;
    }

    bool TcpIocpSessionState::implIsConnected()
    {
        return isFdConnected(mFd);
    }

    ClientTransportAutoPtr TcpIocpSessionState::implCreateClientTransport()
    {
        int fd = -1;
        {
            Lock lock(mMutex);
            if (mOwnFd && !mHasBeenClosed)
            {
                mOwnFd = false;
                fd = mFd;
            }
        }

        std::auto_ptr<TcpClientTransport> tcpClientTransport(
            new TcpClientTransport(fd));

        tcpClientTransport->setNotifyCloseFunctor( boost::bind(
            &IocpSessionState::notifyClose, 
            IocpSessionStateWeakPtr(shared_from_this())));

        tcpClientTransport->setRemoteAddr(
            mRemoteAddress.getSockAddr());

        tcpClientTransport->mRegisteredForAmi = true;

        return ClientTransportAutoPtr(tcpClientTransport.release());
    }

    // TcpIocpServerTransport

    TcpIocpServerTransport::TcpIocpServerTransport(int port) : 
        IocpServerTransport(),
        mPort(port),
        mAcceptorFd(-1),
        mQueuedAccepts(0),
        mQueuedAcceptsThreshold(10),
        mQueuedAcceptsAugment(10),
        mlpfnAcceptEx(RCF_DEFAULT_INIT),
        mlpfnGetAcceptExSockAddrs(RCF_DEFAULT_INIT),
        mMaxPendingConnectionCount(100)
    {
        setNetworkInterface("127.0.0.1");
    }

    TcpIocpServerTransport::TcpIocpServerTransport(
        const std::string &     networkInterface, 
        int                     port) :
            IocpServerTransport(),
            mPort(port),
            mAcceptorFd(-1),
            mQueuedAccepts(0),
            mQueuedAcceptsThreshold(10),
            mQueuedAcceptsAugment(10),
            mlpfnAcceptEx(RCF_DEFAULT_INIT),
            mlpfnGetAcceptExSockAddrs(RCF_DEFAULT_INIT),
            mMaxPendingConnectionCount(100)
    {
        setNetworkInterface(networkInterface);
    }

    ServerTransportPtr TcpIocpServerTransport::clone()
    {
        return ServerTransportPtr( new TcpIocpServerTransport(
            getNetworkInterface(), 
            getPort()));
    }

    void TcpIocpServerTransport::setPort(int port)
    {
        mPort = port;
    }

    int TcpIocpServerTransport::getPort() const
    {
        return mPort;
    }

    void TcpIocpServerTransport::setMaxPendingConnectionCount(
        std::size_t maxPendingConnectionCount)
    {
        mMaxPendingConnectionCount = maxPendingConnectionCount;
    }

    std::size_t TcpIocpServerTransport::getMaxPendingConnectionCount() const
    {
        return mMaxPendingConnectionCount;
    }

    void TcpIocpServerTransport::implOpen()
    {
        // set up a listening socket, if we have a non-negative port number (>0)
        
        RCF_ASSERT(mPort >= -1);
        RCF_ASSERT(mAcceptorFd == -1)(mAcceptorFd);

        mQueuedAccepts = 0;
        
        if (mPort >= 0)
        {
            // create listener socket
            int ret = 0;
            int err = 0;

            mAcceptorFd = static_cast<int>(
                socket(PF_INET, SOCK_STREAM, IPPROTO_TCP));
            
            if (mAcceptorFd == -1)
            {
                err = Platform::OS::BsdSockets::GetLastError();
                RCF_THROW(Exception
                    (_RcfError_Socket(), err, RcfSubsystem_Os, "socket() failed"))
                    (mAcceptorFd);
            }

            // bind listener socket
            std::string networkInterface = getNetworkInterface();
            unsigned long ul_addr = inet_addr( networkInterface.c_str() );
            if (ul_addr == INADDR_NONE)
            {
                hostent *hostDesc = gethostbyname(networkInterface.c_str());
                if (hostDesc)
                {
                    char *szIp = ::inet_ntoa( 
                        * (in_addr*) hostDesc->h_addr_list[0]);

                    ul_addr = ::inet_addr(szIp);
                }
            }
            sockaddr_in serverAddr;
            memset(&serverAddr, 0, sizeof(serverAddr));
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_addr.s_addr = ul_addr;
            serverAddr.sin_port = htons( static_cast<u_short>(mPort) );

            ret = ::bind(
                mAcceptorFd, 
                (struct sockaddr*) &serverAddr, 
                sizeof(serverAddr));

            if (ret < 0)
            {
                err = Platform::OS::BsdSockets::GetLastError();
                if (err == WSAEADDRINUSE)
                {
                    RCF_THROW(Exception(
                        _RcfError_PortInUse(networkInterface, mPort), err, RcfSubsystem_Os, "bind() failed"))
                        (mAcceptorFd)(mPort)(networkInterface)(ret);
                }
                else
                {
                    RCF_THROW(Exception(
                        _RcfError_SocketBind(networkInterface, mPort), err, RcfSubsystem_Os, "bind() failed"))
                        (mAcceptorFd)(mPort)(networkInterface)(ret);
                }
            }

            // listen on listener socket
            ret = listen(
                mAcceptorFd, 
                static_cast<int>(mMaxPendingConnectionCount));

            if (ret < 0)
            {
                err = Platform::OS::BsdSockets::GetLastError();
                RCF_THROW(Exception(
                    _RcfError_Socket(), err, RcfSubsystem_Os, "listen() failed"))
                    (mAcceptorFd)(ret);
            }
            RCF_ASSERT( mAcceptorFd != -1 )(mAcceptorFd);

            // retrieve the port number, if it's generated by the system
            if (mPort == 0 && (mPort = getFdPort(mAcceptorFd)) == 0)
            {
                err = Platform::OS::BsdSockets::GetLastError();
                RCF_THROW(Exception(
                    _RcfError_Socket(), err, RcfSubsystem_Os, "getsockname() failed"))
                    (mAcceptorFd)(mPort)(networkInterface)(ret);
            }

            // load AcceptEx() function
            GUID GuidAcceptEx = WSAID_ACCEPTEX;
            DWORD dwBytes;

            ret = WSAIoctl(
                mAcceptorFd,
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                &GuidAcceptEx,
                sizeof(GuidAcceptEx),
                &mlpfnAcceptEx,
                sizeof(mlpfnAcceptEx),
                &dwBytes,
                NULL,
                NULL);

            err = Platform::OS::BsdSockets::GetLastError();

            RCF_VERIFY(
                ret == 0,
                Exception(_RcfError_Socket(), err, RcfSubsystem_Os,
                "WSAIoctl() failed"));

            // load GetAcceptExSockAddrs() function
            GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

            ret = WSAIoctl(
                mAcceptorFd,
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                &GuidGetAcceptExSockAddrs,
                sizeof(GuidGetAcceptExSockAddrs),
                &mlpfnGetAcceptExSockAddrs,
                sizeof(mlpfnGetAcceptExSockAddrs),
                &dwBytes,
                NULL,
                NULL);

            err = Platform::OS::BsdSockets::GetLastError();

            RCF_VERIFY(
                ret == 0,
                Exception(_RcfError_Socket(), err, RcfSubsystem_Os,
                "WsaIoctl() failed"));

            // associate listener socket to iocp
            mIocpAutoPtr->AssociateSocket( (SOCKET) mAcceptorFd, 0);
        }        
    }

    void TcpIocpServerTransport::implClose()
    {
        // close listener socket
        if (mAcceptorFd != -1)
        {
            int ret = closesocket(mAcceptorFd);
            int err = Platform::OS::BsdSockets::GetLastError();

            RCF_VERIFY(
                ret == 0,
                Exception(
                _RcfError_SocketClose(),
                err,
                RcfSubsystem_Os,
                "closesocket() failed"))
                (mAcceptorFd);

            mAcceptorFd = -1;
        }

        // reset queued accepts count
        mQueuedAccepts = 0;
    }

    void TcpIocpSessionState::assignFd(Fd fd)
    {
        mFd = fd;
    }    

    void TcpIocpSessionState::assignRemoteAddress(
        const IpAddress & remoteAddress)
    {
        mRemoteAddress = remoteAddress;
    }

    IocpSessionStatePtr TcpIocpServerTransport::implCreateServerSession(
        I_ClientTransport & clientTransport)
    {
        TcpClientTransport &tcpClientTransport =
            dynamic_cast<TcpClientTransport &>(clientTransport);

        int fd = tcpClientTransport.releaseFd();
        RCF_ASSERT(fd > 0)(fd);
        
        typedef boost::shared_ptr<TcpIocpSessionState> TcpIocpSessionStatePtr;
        TcpIocpSessionStatePtr sessionStatePtr( TcpIocpSessionState::create(*this));
        sessionStatePtr->assignFd(fd);

        sessionStatePtr->assignRemoteAddress(
            IpAddress(tcpClientTransport.getRemoteAddr()));

        // TODO: If the client transport *is* registered for AMI, then we 
        // really should check which iocp it is associated with, and assert 
        // that it is the iocp of this particular server transport.
        if (!tcpClientTransport.mRegisteredForAmi)
        {
            mIocpAutoPtr->AssociateSocket(fd, 0);
        }

        return IocpSessionStatePtr(sessionStatePtr);
    }

    bool TcpIocpServerTransport::cycleAccepts(
        int timeoutMs,
        const volatile bool &stopFlag)
    {
        if (timeoutMs == 0)
        {
            generateAccepts();
        }
        else
        {
            if (!stopFlag && !mStopFlag)
            {
                {
                    Lock lock(mQueuedAcceptsMutex);
                    if (mQueuedAccepts >= mQueuedAcceptsThreshold)
                    {
                        mQueuedAcceptsCondition.wait(lock);
                    }
                }
                if (!stopFlag && !mStopFlag)
                {
                    generateAccepts();
                }
                else
                {
                    return true;
                }
            }
        }
        return stopFlag || mStopFlag;
    }

    void TcpIocpServerTransport::stopAccepts()
    {
        mStopFlag = true;
        mQueuedAcceptsCondition.notify_one();
    }

    // This function currently assumes mQueuedAcceptsMutex is already locked.
    void TcpIocpServerTransport::generateAccepts()
    {
        unsigned int queuedAccepts = 0;
        {
            Lock lock(mQueuedAcceptsMutex);
            queuedAccepts = mQueuedAccepts;
        }
        if (mAcceptorFd == -1)
        {
            Lock lock(mQueuedAcceptsMutex);
            mQueuedAccepts = mQueuedAcceptsThreshold;
        }
        else if (queuedAccepts < mQueuedAcceptsThreshold)
        {
            std::size_t augment = mQueuedAcceptsAugment;
            std::size_t connectionLimit = getConnectionLimit();
            if (connectionLimit)
            {
                RCF::Lock lock(mSessionsMutex);
                std::size_t connectionsRemaining = 
                    connectionLimit - mSessions.size();

                // One extra connection, so we can report connection limit errors back to clients.
                connectionsRemaining += 1;
                augment = RCF_MIN(augment, connectionsRemaining);
            }
            for (std::size_t i=0; i<augment; i++)
            {
                TcpIocpSessionState::create(*this)->accept();
                {
                    Lock lock(mQueuedAcceptsMutex);
                    ++mQueuedAccepts;
                }
            }
        }
    }

    ClientTransportAutoPtr TcpIocpServerTransport::implCreateClientTransport(
        const I_Endpoint &endpoint)
    {
        const TcpEndpoint &tcpEndpoint =
            dynamic_cast<const TcpEndpoint &>(endpoint);

        std::auto_ptr<TcpClientTransport> tcpClientTransportAutoPtr(
            new TcpClientTransport(
                tcpEndpoint.getIp(),
                tcpEndpoint.getPort()));

        return ClientTransportAutoPtr(tcpClientTransportAutoPtr.release());
    }

    void TcpIocpServerTransport::onServiceAdded(RcfServer &server)
    {
        IocpServerTransport::onServiceAdded(server);

        WriteLock writeLock( getTaskEntriesMutex() );

        getTaskEntries().push_back(
            TaskEntry(
                boost::bind(&TcpIocpServerTransport::cycleAccepts, this, _1, _2),
                boost::bind(&TcpIocpServerTransport::stopAccepts, this),
                "RCF Iocp Acceptor"));
    }


} // namespace RCF
