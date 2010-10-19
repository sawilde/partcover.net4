
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/UnixLocalClientTransport.hpp>

#include <RCF/UnixLocalEndpoint.hpp>

namespace RCF {

    UnixLocalClientTransport::UnixLocalClientTransport(const UnixLocalClientTransport &rhs) : 
        BsdClientTransport(rhs),
        mRemoteAddr(rhs.mRemoteAddr),
        mFileName(rhs.mFileName)
    {}

    UnixLocalClientTransport::UnixLocalClientTransport(const std::string &fileName) :
        BsdClientTransport(),
        mRemoteAddr(),
        mFileName(fileName)
    {
        memset(&mRemoteAddr, 0, sizeof(mRemoteAddr));
    }

    UnixLocalClientTransport::UnixLocalClientTransport(const sockaddr_un &remoteAddr) :
        BsdClientTransport(),
        mRemoteAddr(remoteAddr),
        mFileName()
    {}

    UnixLocalClientTransport::UnixLocalClientTransport(int fd, const std::string & fileName) :
        BsdClientTransport(fd),
        mRemoteAddr(),
        mFileName(fileName)
    {
        memset(&mRemoteAddr, 0, sizeof(mRemoteAddr));
    }

    std::string UnixLocalClientTransport::getPipeName() const
    {
        return mFileName;
    }

    UnixLocalClientTransport::~UnixLocalClientTransport()
    {
        RCF_DTOR_BEGIN
            close();
        RCF_DTOR_END
    }

    ClientTransportAutoPtr UnixLocalClientTransport::clone() const
    {
        return ClientTransportAutoPtr( new UnixLocalClientTransport(*this) );
    }

    void UnixLocalClientTransport::implConnect(unsigned int timeoutMs)
    {
        // close the current connection
        implClose();

        mFd = static_cast<int>( ::socket(AF_UNIX, SOCK_STREAM, 0) );
        int err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(
            mFd != -1,
            Exception(
                _RcfError_Socket(), err, RcfSubsystem_Os, "socket() failed"));
        Platform::OS::BsdSockets::setblocking(mFd, false);

        unsigned int startTimeMs = getCurrentTimeMs();
        mEndTimeMs = startTimeMs + timeoutMs;

        PollingFunctor pollingFunctor(
            mClientProgressPtr,
            ClientProgress::Connect,
            mEndTimeMs);

        err = 0;

        sockaddr_un remote;
        memset(&remote, 0, sizeof(remote));
        remote.sun_family = AF_UNIX;

        std::size_t pipeNameLimit = sizeof(remote.sun_path);
        
        RCF_VERIFY(
            mFileName.length() < pipeNameLimit, 
            Exception(_RcfError_PipeNameTooLong(mFileName, pipeNameLimit)))(pipeNameLimit);

        strcpy(remote.sun_path, mFileName.c_str());
//#ifdef SUN_LEN
//        int remoteLen = SUN_LEN(&remote);
//#else
        int remoteLen = 
            sizeof(remote) 
            - sizeof(remote.sun_path) 
            + strlen(remote.sun_path);
//#endif

        int ret = timedConnect(
            pollingFunctor,
            err,
            mFd,
            (sockaddr*) &remote,
            remoteLen);

        if (ret != 0)
        {
            implClose();

            int rcfErr = (err == 0) ?
                RcfError_ClientConnectTimeout :
                RcfError_ClientConnectFail;

            RCF_THROW(
                Exception(Error(rcfErr), err, RcfSubsystem_Os))
                (ret)(err)(timeoutMs)(mFileName);
        }

    }

    void UnixLocalClientTransport::implConnect(
        I_ClientTransportCallback &clientStub, 
        unsigned int timeoutMs)
    {
        implConnect(timeoutMs);
        clientStub.onConnectCompleted();
    }

    void UnixLocalClientTransport::implConnectAsync(
        I_ClientTransportCallback &clientStub, 
        unsigned int timeoutMs)
    {
        RCF_ASSERT(0);

        // TODO: implement
        // ...
    }

    void UnixLocalClientTransport::implClose()
    {
        if (mFd != -1)
        {
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

    EndpointPtr UnixLocalClientTransport::getEndpointPtr() const
    {
        return EndpointPtr( new UnixLocalEndpoint(mFileName) );
    }

    void UnixLocalClientTransport::setRemoteAddr(const sockaddr_un &remoteAddr)
    {
        mRemoteAddr = remoteAddr;
    }

    const sockaddr_un &UnixLocalClientTransport::getRemoteAddr() const
    {
        return mRemoteAddr;
    }

} // namespace RCF
