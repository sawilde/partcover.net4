
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/TimedBsdSockets.hpp>

#include <algorithm> //std::min/max

#include <RCF/BsdClientTransport.hpp>
#include <RCF/ClientStub.hpp>
#include <RCF/ThreadLocalCache.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    // return -2 for timeout, -1 for error, 0 for ready
    int pollSocket(unsigned int endTimeMs, int fd, int &err, bool bRead)
    {
        ClientStub & clientStub = *getCurrentClientStubPtr();

        while (true)
        {

            fd_set fdSet;
            FD_ZERO(&fdSet);
            FD_SET( static_cast<SOCKET>(fd), &fdSet);
            
            unsigned int timeoutMs = generateTimeoutMs(endTimeMs);
            timeoutMs = clientStub.generatePollingTimeout(timeoutMs);
            
            timeval timeout = {0};
            timeout.tv_sec = timeoutMs/1000;
            timeout.tv_usec = 1000*(timeoutMs%1000);
            RCF_ASSERT(timeout.tv_usec >= 0)(timeout.tv_usec);
            
            int selectRet = bRead ?
                Platform::OS::BsdSockets::select(fd+1, &fdSet, NULL, NULL, &timeout) :
                Platform::OS::BsdSockets::select(fd+1, NULL, &fdSet, NULL, &timeout);
            
            err = Platform::OS::BsdSockets::GetLastError();

            // Handle timeout.
            if (selectRet == 0)
            {
                clientStub.onPollingTimeout();

                if (generateTimeoutMs(endTimeMs) != 0)
                {
                    continue;
                }
            }

            // Some socket gymnastics to determine whether a nonblocking connect 
            // has failed or not.
            if (selectRet == 1 && !bRead)
            {
                int errorOpt = 0;
                Platform::OS::BsdSockets::socklen_t len = sizeof(int); 
                int ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)(&errorOpt), &len);
                err = Platform::OS::BsdSockets::GetLastError();

                RCF_VERIFY(
                    ret == 0, 
                    Exception(_RcfError_Socket(), err, RcfSubsystem_Os, "getsockopt() failed"));

                if (errorOpt == 0)
                {
                    return 0;
                }
                else if (   errorOpt == Platform::OS::BsdSockets::ERR_EWOULDBLOCK
                        ||  errorOpt == Platform::OS::BsdSockets::ERR_EINPROGRESS)
                {
                    continue;
                }
                else
                { 
                    err = errorOpt;
                    return -1;
                }
            }

            switch (selectRet)
            {
            case 0:  return -2;
            case 1:  return  0;
            default: return -1;
            };
        }
    }

    //******************************************************
    // nonblocking socket routines

    // returns -2 for timeout, -1 for error, otherwise 0
    int timedConnect(
        I_PollingFunctor &pollingFunctor,
        int &err,
        int fd,
        const sockaddr *addr,
        int addrLen)
    {
        int ret = Platform::OS::BsdSockets::connect(fd, addr, addrLen);
        err = Platform::OS::BsdSockets::GetLastError();
        if (
            (ret == -1 && err == Platform::OS::BsdSockets::ERR_EWOULDBLOCK) ||
            (ret == -1 && err == Platform::OS::BsdSockets::ERR_EINPROGRESS))
        {
            return pollingFunctor(fd, err, false);
        }
        else if (ret == 0)
        {
            err = 0;
            return 0;
        }
        else
        {
            err = Platform::OS::BsdSockets::GetLastError();
            return -1;
        }
    }

#ifdef BOOST_WINDOWS

    void appendWsabuf(std::vector<WSABUF> &wsabufs, const ByteBuffer &byteBuffer)
    {
        WSABUF wsabuf = {0};
        wsabuf.buf = byteBuffer.getPtr() ;
        wsabuf.len = static_cast<u_long>(byteBuffer.getLength());
        wsabufs.push_back(wsabuf);
    }

#else

    void appendWsabuf(std::vector<WSABUF> &wsabufs, const ByteBuffer &byteBuffer)
    {
        WSABUF wsabuf = {0};
        wsabuf.iov_base = static_cast<void *>( byteBuffer.getPtr() );
        wsabuf.iov_len = static_cast<std::size_t>(byteBuffer.getLength());
        wsabufs.push_back(wsabuf);
    }

#endif

    // returns -2 for timeout, -1 for error, otherwise number of bytes sent (> 0)
    int timedSend(
        I_PollingFunctor &pollingFunctor,
        int &err,
        int fd,
        const std::vector<ByteBuffer> &byteBuffers,
        std::size_t maxSendSize,
        int flags)
    {
        RCF_UNUSED_VARIABLE(flags);
        std::size_t bytesRemaining = lengthByteBuffers(byteBuffers);
        std::size_t bytesSent = 0;
        while (true)
        {
            std::size_t bytesToSend = RCF_MIN(bytesRemaining, maxSendSize);

            ThreadLocalCached< std::vector<WSABUF> > tlcWsabufs;
            std::vector<WSABUF> &wsabufs = tlcWsabufs.get();

            forEachByteBuffer(
                boost::bind(&appendWsabuf, boost::ref(wsabufs), _1),
                byteBuffers,
                bytesSent,
                bytesToSend);

            int count = 0;
            int myErr = 0;

#ifdef BOOST_WINDOWS
            {
                DWORD cbSent = 0;
                int ret = WSASend(
                    fd, 
                    &wsabufs[0], 
                    static_cast<DWORD>(wsabufs.size()), 
                    &cbSent, 
                    0, 
                    NULL, 
                    NULL);

                count = (ret == 0) ? cbSent : -1;
                myErr = Platform::OS::BsdSockets::GetLastError();
            }            
#else
            {
                msghdr hdr = {0};
                hdr.msg_iov = &wsabufs[0];
                hdr.msg_iovlen = wsabufs.size();
                count = sendmsg(fd, &hdr, 0);
                myErr = Platform::OS::BsdSockets::GetLastError();
            }
#endif

            //int myErr = WSAGetLastError();
            //int myErr = Platform::OS::BsdSockets::GetLastError()
            //if (ret == 0)
            if (count >= 0)
            {
                RCF_ASSERT(
                    count <= static_cast<int>(bytesRemaining))
                    (count)(bytesRemaining);

                bytesRemaining -= count;//cbSent;
                bytesSent += count;//cbSent;
                err = 0;
                return static_cast<int>(bytesSent);
            }
            //else if (myErr == WSAEWOULDBLOCK)
            else if (myErr == Platform::OS::BsdSockets::ERR_EWOULDBLOCK)
            {
                // can't get WSA_IO_PENDING here, since the socket isn't overlapped
                int ret = pollingFunctor(fd, myErr, false);
                if (ret  != 0)
                {
                    err = myErr;
                    return ret;
                }
            }
            else
            {
                err = myErr;
                return -1;
            }


        }
    }

    // -2 for timeout, -1 for error, 0 for peer closure, otherwise size of packet read
    int timedRecv(
        BsdClientTransport &clientTransport,
        I_PollingFunctor &pollingFunctor,
        int &err,
        int fd,
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested,
        int flags)
    {
        RCF_ASSERT(!byteBuffer.isEmpty());
       
        std::size_t bytesToRead =
            RCF_MIN(byteBuffer.getLength(), bytesRequested);

        while (true)
        {
            int ret = Platform::OS::BsdSockets::recv(
                fd,
                byteBuffer.getPtr(),
                static_cast<int>(bytesToRead),
                flags);

            err = Platform::OS::BsdSockets::GetLastError();
            if (ret >= 0)
            {
                err = 0;
                clientTransport.onTimedRecvCompleted(ret, err);
                return ret;
            }
            else if (
                ret == -1 &&
                err == Platform::OS::BsdSockets::ERR_EWOULDBLOCK)
            {
                int ret = pollingFunctor(fd, err, true);
                if (ret  != 0)
                {
                    clientTransport.onTimedRecvCompleted(ret, err);
                    return ret;
                }
            }
            else if (ret == -1)
            {
                err = Platform::OS::BsdSockets::GetLastError();
                clientTransport.onTimedRecvCompleted(-1, err);
                return -1;
            }
        }
    }

    bool isFdConnected(int fd)
    {
        bool connected = false;
        if (fd != -1)
        {
            timeval tv = {0,0};
            fd_set readFds;
            FD_ZERO(&readFds);
            FD_SET( static_cast<SOCKET>(fd), &readFds);

            int ret = Platform::OS::BsdSockets::select(
                fd+1,
                &readFds,
                NULL,
                NULL,
                &tv);

            if (ret == 0)
            {
                connected = true;
            }
            else if (ret == 1)
            {
                const int length = 1;
                char buffer[length];

                int ret = Platform::OS::BsdSockets::recv(
                    fd,
                    buffer,
                    length,
                    MSG_PEEK);

                if (ret == -1)
                {
                    int err = Platform::OS::BsdSockets::GetLastError();
                    if (
                        err != Platform::OS::BsdSockets::ERR_ECONNRESET &&
                        err != Platform::OS::BsdSockets::ERR_ECONNABORTED &&
                        err != Platform::OS::BsdSockets::ERR_ECONNREFUSED)
                    {
                        connected = true;
                    }
                }
                else if (ret == 0)
                {
                    connected = false;
                }
                else if (ret > 0)
                {
                    connected = true;
                }
            }
        }
        return connected;
    }

    std::pair<std::string, std::vector<std::string> > getLocalIps()
    {
        std::vector<char> hostname(80);
        int ret = gethostname(&hostname[0], static_cast<int>(hostname.size()));
        int err = Platform::OS::BsdSockets::GetLastError();

        RCF_VERIFY(
            ret != -1, 
            RCF::Exception( _RcfError_Socket(), err, RcfSubsystem_Os))(ret)(err);

        hostent *phe = gethostbyname(&hostname[0]);
        err = Platform::OS::BsdSockets::GetLastError();

        RCF_VERIFY(
            phe, 
            RCF::Exception( _RcfError_Socket(), err, RCF::RcfSubsystem_Os))(err);

        std::vector<std::string> ips;
        for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
            struct in_addr addr;
            memcpy(&addr, phe->h_addr_list[i], sizeof( in_addr));
            ips.push_back(inet_ntoa(addr));
        }

        return std::make_pair( std::string(&hostname[0]), ips);
    }

} // namespace RCF
