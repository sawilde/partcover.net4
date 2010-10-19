
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/UdpClientTransport.hpp>
#include <RCF/UdpEndpoint.hpp>

#include <boost/static_assert.hpp>

#include <RCF/Tools.hpp>

#include <RCF/util/Platform/OS/BsdSockets.hpp>

namespace RCF {

    UdpClientTransport::UdpClientTransport(const std::string &ip, int port) :
        mIp(ip),
        mPort(port),
        mSock(-1),
        mSrcAddr(),
        mDestAddr(),
        mAsync(RCF_DEFAULT_INIT)
    {
        memset(&mSrcAddr, 0, sizeof(mSrcAddr));
        memset(&mDestAddr, 0, sizeof(mDestAddr));
    }

    UdpClientTransport::UdpClientTransport(const sockaddr &dest) :
        mIp(),
        mPort(),
        mSock(-1),
        mSrcAddr(),
        mDestAddr(),
        mAsync(RCF_DEFAULT_INIT)
    {
        memset(&mSrcAddr, 0, sizeof(mSrcAddr));
        memset(&mDestAddr, 0, sizeof(mDestAddr));

        BOOST_STATIC_ASSERT(sizeof(sockaddr) == sizeof(sockaddr_in));
        memcpy(&mDestAddr, &dest, sizeof(dest));
    }

    UdpClientTransport::UdpClientTransport(const UdpClientTransport &rhs) :
        I_ClientTransport(rhs),
        mIp(rhs.mIp),
        mPort(rhs.mPort),
        mSock(-1),
        mSrcAddr(),
        mDestAddr(),
        mAsync(RCF_DEFAULT_INIT)
    {
        memset(&mSrcAddr, 0, sizeof(mSrcAddr));
        memset(&mDestAddr, 0, sizeof(mDestAddr));
    }

    UdpClientTransport::~UdpClientTransport()
    {
        RCF_DTOR_BEGIN
            close();
        RCF_DTOR_END
    }

    ClientTransportAutoPtr UdpClientTransport::clone() const
    {
        return ClientTransportAutoPtr( new UdpClientTransport(*this));
    }

    EndpointPtr UdpClientTransport::getEndpointPtr() const
    {
        return EndpointPtr( new UdpEndpoint(mIp, mPort) );
    }

    void UdpClientTransport::setAsync(bool async)
    {
        mAsync = async;
    }

    UdpClientTransport::TimerEntry UdpClientTransport::setTimer(
        boost::uint32_t timeoutMs,
        I_ClientTransportCallback *pClientStub)
    {
        RCF_UNUSED_VARIABLE(timeoutMs);
        RCF_UNUSED_VARIABLE(pClientStub);
        return TimerEntry();
    }

    void UdpClientTransport::killTimer(
        const TimerEntry & timerEntry)
    {
        RCF_UNUSED_VARIABLE(timerEntry);
    }

    void UdpClientTransport::connect(
        I_ClientTransportCallback &clientStub, 
        unsigned int timeoutMs)
    {
        RCF_TRACE("")(mSock)(mIp)(mPort);
        RCF_UNUSED_VARIABLE(timeoutMs);

        RCF_ASSERT(!mAsync);

        // TODO: replace throw with return value
        if (mSock == -1)
        {
            int ret = 0;
            int err = 0;

            mSock = static_cast<int>(socket(AF_INET,SOCK_DGRAM,0));
            err = Platform::OS::BsdSockets::GetLastError();
            RCF_VERIFY(
                mSock != -1,
                Exception(
                    _RcfError_Socket(), err, RcfSubsystem_Os,
                    "socket() failed"));

            // local address
            memset(&mSrcAddr, 0, sizeof(mSrcAddr));
            mSrcAddr.sin_family = AF_INET;
            mSrcAddr.sin_port = htons(0);
            mSrcAddr.sin_addr.s_addr = INADDR_ANY;

            // remote address
            if (mIp != "")
            {
                RCF_ASSERT(mIp != "");
                RCF_ASSERT(mPort > 0);

                unsigned long ul_addr = ::inet_addr(mIp.c_str());
                if (ul_addr == INADDR_NONE)
                {
                    hostent *hostDesc = ::gethostbyname( mIp.c_str() );
                    if (hostDesc)
                    {
                        char *szIp = ::inet_ntoa(
                            * (in_addr*) hostDesc->h_addr_list[0]);

                        ul_addr = ::inet_addr(szIp);
                    }
                }

                memset(&mDestAddr,0,sizeof(mDestAddr));
                mDestAddr.sin_family = AF_INET;
                mDestAddr.sin_port = htons( static_cast<u_short>(mPort) );
                mDestAddr.sin_addr.s_addr = ul_addr;
            }

            ret = ::bind(mSock, (sockaddr *) &mSrcAddr, sizeof(mSrcAddr));
            err = Platform::OS::BsdSockets::GetLastError();
            RCF_VERIFY(
                ret == 0,
                Exception(
                    _RcfError_Socket(), err, RcfSubsystem_Os,
                    "bind() failed"));

            // check for a broadcast address
            if ((ntohl(mDestAddr.sin_addr.s_addr) & 0x000000FF) == 0x000000FF)
            {
                // set socket option to allow transmission of broadcast messages
                int enable = 1;
                int ret = setsockopt(mSock, SOL_SOCKET, SO_BROADCAST, (char *) &enable, sizeof(enable));
                int err = Platform::OS::BsdSockets::GetLastError();

                RCF_VERIFY(
                    ret ==  0,
                    Exception(
                        _RcfError_Socket(),
                        err,
                        RcfSubsystem_Os,
                        "setsockopt() with SO_BROADCAST failed"));
            }

            // check for a multicast address
            if ((ntohl(mDestAddr.sin_addr.s_addr) & 0xE0000000) == 0xE0000000)
            {
                // char for Solaris, int for everyone else.
#if defined(__SVR4) && defined(__sun)
                char hops = 16;
#else
                int hops = 16;
#endif
                int ret = setsockopt(mSock, IPPROTO_IP, IP_MULTICAST_TTL, (char *) &hops, sizeof (hops));
                int err = Platform::OS::BsdSockets::GetLastError();

                RCF_VERIFY(
                    ret ==  0,
                    Exception(
                        _RcfError_Socket(),
                        err,
                        RcfSubsystem_Os,
                        "setsockopt() with IPPROTO_IP/IP_MULTICAST_TTL failed"))(hops);
            }
        }

        clientStub.onConnectCompleted();
    }

    int UdpClientTransport::send(
        I_ClientTransportCallback &clientStub, 
        const std::vector<ByteBuffer> &data,
        unsigned int timeoutMs)
    {
        RCF_TRACE("")(mSock)(mIp)(mPort);
        RCF_UNUSED_VARIABLE(timeoutMs);

        RCF_ASSERT(!mAsync);

        // TODO: optimize for case of single byte buffer with left margin

        if (mWriteVecPtr.get() == NULL || !mWriteVecPtr.unique())
        {
            mWriteVecPtr.reset( new std::vector<char>());
        }

        std::vector<char> &buffer = *mWriteVecPtr;
        buffer.resize(lengthByteBuffers(data));

        copyByteBuffers(data, &buffer[0]);

        int len = sendto(
            mSock,
            &buffer[0],
            static_cast<int>(buffer.size()),
            0,
            (struct sockaddr *) &mDestAddr, sizeof(mDestAddr));

        int err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(
            len > 0,
            Exception(
                _RcfError_Socket(),
                err,
                RcfSubsystem_Os,
                "sendto() failed"));

        clientStub.onSendCompleted();

        return 1;
    }

    int UdpClientTransport::receive(
        I_ClientTransportCallback &clientStub, 
        ByteBuffer &byteBuffer,
        unsigned int timeoutMs)
    {
        // try to receive a UDP message from server, within the current timeout
        RCF_TRACE("")(mSock)(mIp)(mPort);

        RCF_ASSERT(!mAsync);

        unsigned int startTimeMs = getCurrentTimeMs();
        unsigned int endTimeMs = startTimeMs + timeoutMs;

        while (true)
        {
            unsigned int timeoutMs = generateTimeoutMs(endTimeMs);
            fd_set fdSet;
            FD_ZERO(&fdSet);
            FD_SET( static_cast<SOCKET>(mSock), &fdSet);
            timeval timeout;
            timeout.tv_sec = timeoutMs/1000;
            timeout.tv_usec = 1000*(timeoutMs%1000);

            int ret = Platform::OS::BsdSockets::select(
                mSock+1,
                &fdSet,
                NULL,
                NULL,
                &timeout);

            int err = Platform::OS::BsdSockets::GetLastError();

            RCF_ASSERT(-1 <= ret && ret <= 1)(ret);
            if (ret == -1)
            {
                RCF_THROW(
                    Exception(
                        _RcfError_Socket(),
                        err,
                        RcfSubsystem_Os,
                        "udp client select() failed"));
            }
            else if (ret == 0)
            {
                RCF_THROW( Exception( _RcfError_ClientReadTimeout() ) );
            }
            RCF_ASSERT(ret == 1)(ret);

            if (mReadVecPtr.get() == NULL || !mReadVecPtr.unique())
            {
                mReadVecPtr.reset( new std::vector<char>());
            }

            // TODO: optimize
            std::vector<char> &buffer = *mReadVecPtr;
            buffer.resize(4);
            sockaddr_in fromAddr;
            memset(&fromAddr, 0, sizeof(fromAddr));
            int fromAddrLen = sizeof(fromAddr);

            int len = Platform::OS::BsdSockets::recvfrom(
                mSock,
                &buffer[0],
                4,
                MSG_PEEK,
                (sockaddr *) &fromAddr,
                &fromAddrLen);

            err = Platform::OS::BsdSockets::GetLastError();
            if (len == 4 ||
                (len == -1 && err == Platform::OS::BsdSockets::ERR_EMSGSIZE))
            {
                if (fromAddr.sin_addr.s_addr == mDestAddr.sin_addr.s_addr &&
                    fromAddr.sin_port == mDestAddr.sin_port)
                {
                    unsigned int dataLength = 0;
                    memcpy( &dataLength, &buffer[0], 4);
                    RCF::networkToMachineOrder(&dataLength, 4, 1);

                    RCF_VERIFY(
                        0 < dataLength && dataLength <= getMaxMessageLength(),
                        Exception(_RcfError_ClientMessageLength()))
                        (dataLength)(getMaxMessageLength());

                    buffer.resize(4+dataLength);
                    memset(&fromAddr, 0, sizeof(fromAddr));
                    fromAddrLen = sizeof(fromAddr);

                    len = Platform::OS::BsdSockets::recvfrom(
                        mSock,
                        &buffer[0],
                        dataLength+4,
                        0,
                        (sockaddr *) &fromAddr,
                        &fromAddrLen);

                    if (len == static_cast<int>(dataLength+4))
                    {
                        byteBuffer = ByteBuffer(
                            &buffer[4],
                            dataLength,
                            4,
                            mReadVecPtr);

                        clientStub.onReceiveCompleted();

                        return 1;
                    }
                }
            }
        }
    }

    void UdpClientTransport::close()
    {
        if (mSock != -1)
        {
            int ret = Platform::OS::BsdSockets::closesocket(mSock);
            int err = Platform::OS::BsdSockets::GetLastError();
            if (ret < 0)
            {
                RCF_ASSERT(0)(mSock)(ret)(err);
            }
            mSock = -1;
        }
    }

    bool UdpClientTransport::isConnected()
    {
        //return true;

        return mSock != -1;
    }

    void UdpClientTransport::disconnect(unsigned int timeoutMs)
    {
        RCF_UNUSED_VARIABLE(timeoutMs);
    }

    void UdpClientTransport::setTransportFilters(
        const std::vector<FilterPtr> &filters)
    {
        if (!filters.empty())
        {
            RCF_ASSERT(0);
        }
    }

    void UdpClientTransport::getTransportFilters(
        std::vector<FilterPtr> &filters)
    {
        RCF_UNUSED_VARIABLE(filters);
    }

    int UdpClientTransport::getSocket() 
    { 
        return mSock; 
    }

    int UdpClientTransport::getNativeHandle() const
    {
        return mSock;
    }

} // namespace RCF
