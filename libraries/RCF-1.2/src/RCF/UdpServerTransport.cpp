
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/UdpServerTransport.hpp>

#include <RCF/MethodInvocation.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/Tools.hpp>

#include <RCF/util/Platform/OS/BsdSockets.hpp>

namespace RCF {

    UdpServerTransport::UdpServerTransport(int port) :
        mpSessionManager(RCF_DEFAULT_INIT),
        mPort(port),
        mFd(-1),
        mStopFlag(RCF_DEFAULT_INIT),
        mPollingDelayMs(RCF_DEFAULT_INIT),
        mEnableSharedAddressBinding(RCF_DEFAULT_INIT)
    {
    }

    UdpServerTransport::UdpServerTransport(
        const std::string &networkInterface, 
        int port,
        const std::string &multicastIp) :
            mpSessionManager(RCF_DEFAULT_INIT),
            mPort(port),
            mFd(-1),
            mStopFlag(RCF_DEFAULT_INIT),
            mPollingDelayMs(RCF_DEFAULT_INIT),
            mMulticastIp(multicastIp),
            mEnableSharedAddressBinding(RCF_DEFAULT_INIT)
    {
        setNetworkInterface(networkInterface);
    }

    UdpServerTransport & UdpServerTransport::enableSharedAddressBinding()
    {
        mEnableSharedAddressBinding = true;
        return *this;
    }

    ServerTransportPtr UdpServerTransport::clone()
    {
        return ServerTransportPtr( new UdpServerTransport(mPort) );
    }

    void UdpServerTransport::setSessionManager(I_SessionManager &sessionManager)
    {
        mpSessionManager = &sessionManager;
    }

    I_SessionManager &UdpServerTransport::getSessionManager()
    {
        RCF_ASSERT(mpSessionManager);
        return *mpSessionManager;
    }

    void UdpServerTransport::setPort(int port)
    {
        this->mPort = port;
    }

    int UdpServerTransport::getPort() const
    {
        return mPort;
    }

    void UdpServerTransport::open()
    {
        RCF_TRACE("")(mPort)(getNetworkInterface());

        // create and bind a socket for receiving UDP messages
        if (mFd == -1 && mPort >= 0)
        {
            int ret = 0;
            int err = 0;

            // create the socket
            mFd = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
            if (mFd == -1)
            {
                err = Platform::OS::BsdSockets::GetLastError();
                RCF_THROW(Exception(
                    _RcfError_Socket(), err, RcfSubsystem_Os, "socket() failed"))
                    (mFd);
            }

            // setup the address
            std::string networkInterface = getNetworkInterface();
            sockaddr_in serverAddr;
            memset(&serverAddr, 0, sizeof(serverAddr));
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons( static_cast<u_short>(mPort) );
            if (networkInterface.size() > 0)
            {
                if (isdigit(networkInterface.at(0)))
                {
                    serverAddr.sin_addr.s_addr = inet_addr(
                        networkInterface.c_str());
                }
                else
                {
                    hostent *h = gethostbyname(networkInterface.c_str());
                    if (h)
                    {
                        serverAddr.sin_addr = * (in_addr *) h->h_addr_list[0];
                    }
                }
            }
            else
            {
                serverAddr.sin_addr.s_addr = INADDR_ANY;
            }

            // enable reception of broadcast messages
            int enable = 1;
            ret = setsockopt(mFd, SOL_SOCKET, SO_BROADCAST, (char *) &enable, sizeof(enable));
            err = Platform::OS::BsdSockets::GetLastError();
            if (ret)
            {
                RCF_TRACE("Failed to set SO_BROADCAST on listening udp socket")(ret)(err);
            }

            // Share the address binding, if appropriate.
            if (mEnableSharedAddressBinding)
            {
                enable = 1;

                // Set SO_REUSEADDR socket option.
                ret = setsockopt(mFd, SOL_SOCKET, SO_REUSEADDR, (char *) &enable, sizeof(enable));
                err = Platform::OS::BsdSockets::GetLastError();
                if (ret)
                {
                    RCF_TRACE("Failed to set SO_REUSEADDR on listening udp multicast socket")(ret)(err);
                }

                // On OS X and BSD variants, need to set SO_REUSEPORT as well.

#if (defined(__MACH__) && defined(__APPLE__)) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) 

                ret = setsockopt(mFd, SOL_SOCKET, SO_REUSEPORT, (char *) &enable, sizeof(enable));
                err = Platform::OS::BsdSockets::GetLastError();
                if (ret)
                {
                    RCF_TRACE("Failed to set SO_REUSEPORT on listening udp multicast socket")(ret)(err);
                }
#endif

            }
            

            // bind the socket
            ret = ::bind(mFd, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
            if (ret < 0)
            {
                err = Platform::OS::BsdSockets::GetLastError();
                RCF_THROW(Exception(
                    _RcfError_Socket(), err, RcfSubsystem_Os, "bind() failed"))
                    (mFd)(mPort)(networkInterface)(ret);
            }
            RCF_ASSERT( mFd != -1 )(mFd);

            if (mMulticastIp.size() > 0)
            {
                // set socket option for receiving multicast messages
            
                ip_mreq imr;
                imr.imr_multiaddr.s_addr = inet_addr(mMulticastIp.c_str());
                imr.imr_interface.s_addr = INADDR_ANY;
                int ret = setsockopt(mFd,IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*) &imr, sizeof(imr));
                int err = Platform::OS::BsdSockets::GetLastError();

                RCF_VERIFY(
                    ret ==  0,
                    Exception(
                        _RcfError_Socket(),
                        err,
                        RcfSubsystem_Os,
                        "setsockopt() with IPPROTO_IP/IP_ADD_MEMBERSHIP failed"))
                        (mMulticastIp)(networkInterface);

                // TODO: enable source-filtered multicast messages
                //ip_mreq_source imr;
                //imr.imr_multiaddr.s_addr = inet_addr("232.5.6.7");
                //imr.imr_sourceaddr.s_addr = INADDR_ANY;//inet_addr("10.1.1.2");
                //imr.imr_interface.s_addr = INADDR_ANY;
                //int ret = setsockopt(mFd,IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (const char*) &imr, sizeof(imr));
                //int err = Platform::OS::BsdSockets::GetLastError();
            }

            // set the socket to nonblocking mode
            Platform::OS::BsdSockets::setblocking(mFd, false);

            // retrieve the port number, if it's generated by the system
            if (mPort == 0 && (mPort = getFdPort(mFd)) == 0)
            {
                err = Platform::OS::BsdSockets::GetLastError();
                RCF_THROW(Exception(
                    _RcfError_Socket(), err, RcfSubsystem_Os, "getsockname() failed"))
                    (mFd)(mPort)(networkInterface)(ret);
            }
        }
    }

    void UdpServerTransport::close()
    {
        if (mFd != -1)
        {
            int ret = Platform::OS::BsdSockets::closesocket(mFd);
            int err = Platform::OS::BsdSockets::GetLastError();
            RCF_VERIFY(
                ret == 0,
                Exception(
                    _RcfError_SocketClose(), err, RcfSubsystem_Os,
                    "closesocket() failed"))(mFd);
            mFd = -1;
        }
    }

    void discardPacket(int fd)
    {
        char buffer[1];
        int len = recvfrom(fd, buffer, 1, 0, NULL, NULL);
        int err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(
            len == 1 ||
            (len == -1 && err == Platform::OS::BsdSockets::ERR_EMSGSIZE) ||
            (len == -1 && err == Platform::OS::BsdSockets::ERR_ECONNRESET),
            Exception(
                _RcfError_Socket(),
                err,
                RcfSubsystem_Os,
                "recvfrom() failed"));
    }

    void UdpServerTransport::cycle(
        int timeoutMs,
        const volatile bool &) //stopFlag
    {
        // poll the UDP socket for messages, and read a message if one is available

        fd_set fdSet;
        FD_ZERO(&fdSet);
        FD_SET( static_cast<SOCKET>(mFd), &fdSet);
        timeval timeout;
        timeout.tv_sec = timeoutMs/1000;
        timeout.tv_usec = 1000*(timeoutMs%1000);

        int ret = Platform::OS::BsdSockets::select(
            mFd+1,
            &fdSet,
            NULL,
            NULL,
            timeoutMs < 0 ? NULL : &timeout);

        int err = Platform::OS::BsdSockets::GetLastError();
        if (ret == 1)
        {
            SessionStatePtr sessionStatePtr = getCurrentUdpSessionStatePtr();
            if (sessionStatePtr.get() == NULL)
            {
                sessionStatePtr = SessionStatePtr(new SessionState(*this));
                SessionPtr sessionPtr = getSessionManager().createSession();
                sessionPtr->setProactor(*sessionStatePtr);
                sessionStatePtr->mSessionPtr = sessionPtr;
                setCurrentUdpSessionStatePtr(sessionStatePtr);

                RcfSessionPtr rcfSessionPtr = 
                    boost::static_pointer_cast<RcfSession>(sessionPtr);
            }

            {
                // read a message

                boost::shared_ptr<std::vector<char> > &readVecPtr =
                    sessionStatePtr->mReadVecPtr;

                if (readVecPtr.get() == NULL || !readVecPtr.unique())
                {
                    readVecPtr.reset( new std::vector<char>());
                }
                std::vector<char> &buffer = *readVecPtr;

                sockaddr from;
                int fromlen = sizeof(from);
                memset(&from, 0, sizeof(from));
                buffer.resize(4);

                int len = Platform::OS::BsdSockets::recvfrom(
                    mFd,
                    &buffer[0],
                    4,
                    MSG_PEEK,
                    &from,
                    &fromlen);

                err = Platform::OS::BsdSockets::GetLastError();
                if (isClientAddrAllowed( *(sockaddr_in *) &from ) &&
                    (len == 4 || (len == -1 && err == Platform::OS::BsdSockets::ERR_EMSGSIZE)))
                {
                    sockaddr_in *remoteAddr = reinterpret_cast<sockaddr_in*>(&from);
                    sessionStatePtr->remoteAddress = IpAddress(*remoteAddr);
                    unsigned int dataLength = 0;
                    memcpy(&dataLength, &buffer[0], 4);
                    networkToMachineOrder(&dataLength, 4, 1);
                    if (    0 < dataLength 
                        &&  dataLength <= static_cast<unsigned int>(getMaxMessageLength()))
                    {
                        buffer.resize(4+dataLength);
                        memset(&from, 0, sizeof(from));
                        fromlen = sizeof(from);

                        len = Platform::OS::BsdSockets::recvfrom(
                            mFd,
                            &buffer[0],
                            4+dataLength,
                            0,
                            &from,
                            &fromlen);

                        if (static_cast<unsigned int>(len) == 4+dataLength)
                        {
                            getSessionManager().onReadCompleted(sessionStatePtr->mSessionPtr);
                        }
                    }
                    else
                    {
                        ByteBuffer byteBuffer;
                        encodeServerError(byteBuffer, RcfError_ServerMessageLength);
                        byteBuffer.expandIntoLeftMargin(4);

                        * (boost::uint32_t *) ( byteBuffer.getPtr() ) =
                            static_cast<boost::uint32_t>(byteBuffer.getLength()-4);

                        RCF::machineToNetworkOrder(byteBuffer.getPtr(), 4, 1);

                        char *buffer = byteBuffer.getPtr();
                        std::size_t bufferLen = byteBuffer.getLength();

                        const sockaddr_in &remoteAddr =
                            sessionStatePtr->remoteAddress.getSockAddr();

                        int len = sendto(
                            mFd,
                            buffer,
                            static_cast<int>(bufferLen),
                            0,
                            (const sockaddr *) &remoteAddr,
                            sizeof(remoteAddr));

                        RCF_UNUSED_VARIABLE(len);
                        discardPacket(mFd);
                    }
                }
                else
                {
                    // discard the message (sender ip not allowed, or message format bad)
                    discardPacket(mFd);
                }
            }
        }
        else if (ret == 0)
        {
            RCF_TRACE("server udp poll - no messages")(mFd)(mPort);
        }
        else if (ret == -1)
        {
            RCF_THROW(
                Exception(
                    _RcfError_Socket(),
                    err,
                    RcfSubsystem_Os,
                    "udp server select() failed "))
                (mFd)(mPort)(err);
        }

    }

    void UdpSessionState::postWrite(
        std::vector<ByteBuffer> &byteBuffers)
    {
        // prepend data length and send the data

        boost::shared_ptr<std::vector<char> > &writeVecPtr = mWriteVecPtr;

        if (writeVecPtr.get() == NULL || !writeVecPtr.unique())
        {
            writeVecPtr.reset( new std::vector<char>());
        }

        std::vector<char> &writeBuffer = *writeVecPtr;

        unsigned int dataLength = static_cast<unsigned int>(
            lengthByteBuffers(byteBuffers));

        writeBuffer.resize(4+dataLength);
        memcpy( &writeBuffer[0], &dataLength, 4);
        machineToNetworkOrder(&writeBuffer[0], 4, 1);
        copyByteBuffers(byteBuffers, &writeBuffer[4]);
        byteBuffers.resize(0);

        const sockaddr_in &remoteAddr = remoteAddress.getSockAddr();
       
        int len = sendto(
            mTransport.mFd,
            &writeBuffer[0],
            static_cast<int>(writeBuffer.size()),
            0,
            (const sockaddr *) &remoteAddr,
            sizeof(remoteAddr));

        if (len != static_cast<int>(writeBuffer.size()))
        {
            int err = Platform::OS::BsdSockets::GetLastError();
            RCF_THROW(Exception(
                _RcfError_Socket(), err, RcfSubsystem_Os, "sendto() failed"))
                (mTransport.mFd)(len)(writeBuffer.size());
        }

        SessionStatePtr sessionStatePtr = getCurrentUdpSessionStatePtr();

        SessionPtr sessionPtr = sessionStatePtr->mSessionPtr;

        RcfSessionPtr rcfSessionPtr = 
            boost::static_pointer_cast<RcfSession>(sessionPtr);
    }

    void UdpSessionState::postRead()
    {
    }

    bool UdpServerTransport::cycleTransportAndServer(
        RcfServer &server,
        int timeoutMs,
        const volatile bool &stopFlag)
    {
        if (!stopFlag && !mStopFlag)
        {
            cycle(timeoutMs/2, stopFlag);
            server.cycleSessions(timeoutMs/2, stopFlag);
        }
        return stopFlag || mStopFlag;
    }

    void UdpServerTransport::onServiceAdded(RcfServer &server)
    {
        setSessionManager(server);
        WriteLock writeLock( getTaskEntriesMutex() );
        getTaskEntries().clear();

        getTaskEntries().push_back(
            TaskEntry(
                boost::bind(
                    &UdpServerTransport::cycleTransportAndServer,
                    this,
                    boost::ref(server),
                    _1,
                    _2),
                StopFunctor(),
                "RCF Udp server"));

        mStopFlag = false;
    }

    void UdpServerTransport::onServiceRemoved(RcfServer &)
    {}

    UdpSessionState::UdpSessionState(UdpServerTransport & transport) :
        mTransport(transport)
    {}

    void UdpServerTransport::onServerOpen(RcfServer &)
    {
        open();
    }

    void UdpServerTransport::onServerClose(RcfServer &)
    {
        close();
    }

    void UdpServerTransport::onServerStart(RcfServer &)
    {
    }

    void UdpServerTransport::onServerStop(RcfServer &)
    {
        mStopFlag = false;
    }
   
    const I_RemoteAddress &UdpSessionState::getRemoteAddress() const
    {
        return remoteAddress;
    }

    I_ServerTransport & UdpSessionState::getServerTransport()
    {
        return mTransport;
    }

    const I_RemoteAddress & UdpSessionState::getRemoteAddress()
    {
        return remoteAddress;
    }

    void UdpSessionState::setTransportFilters(const std::vector<FilterPtr> &filters)
    {
        RCF_UNUSED_VARIABLE(filters);
        RCF_ASSERT(0);
    }

    void UdpSessionState::getTransportFilters(std::vector<FilterPtr> &filters)
    {
        RCF_UNUSED_VARIABLE(filters);
        RCF_ASSERT(0);
    }

    ByteBuffer UdpSessionState::getReadByteBuffer()
    {
        return ByteBuffer(
            &mReadVecPtr->front() + 4,
            mReadVecPtr->size() - 4,
            4,
            mReadVecPtr);
    }

    void UdpSessionState::postClose()
    {
    }

    int UdpSessionState::getNativeHandle() const
    {
        return mTransport.mFd;
    }

} // namespace RCF
