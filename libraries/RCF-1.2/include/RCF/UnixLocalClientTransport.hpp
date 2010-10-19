
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_UNIXLOCALCLIENTTRANSPORT_HPP
#define INCLUDE_RCF_UNIXLOCALCLIENTTRANSPORT_HPP

#if defined(BOOST_WINDOWS)
#error Unix domain sockets not supported on Windows.
#endif

#include <RCF/BsdClientTransport.hpp>
#include <RCF/Export.hpp>
#include <RCF/IpClientTransport.hpp>

#include <sys/un.h>

namespace RCF {

    class RCF_EXPORT UnixLocalClientTransport : 
        public BsdClientTransport
    {
    public:
        UnixLocalClientTransport(const UnixLocalClientTransport &rhs);
        UnixLocalClientTransport(const std::string &fileName);
        UnixLocalClientTransport(const sockaddr_un &remoteAddr);
        UnixLocalClientTransport(int fd, const std::string & fileName);

        ~UnixLocalClientTransport();

        ClientTransportAutoPtr clone() const;

        void                    implConnect(unsigned int timeoutMs);

        void                    implConnect(
                                    I_ClientTransportCallback &clientStub, 
                                    unsigned int timeoutMs);

        void                    implConnectAsync(
                                    I_ClientTransportCallback &clientStub, 
                                    unsigned int timeoutMs);

        void                    implClose();
        EndpointPtr             getEndpointPtr() const;

        void                    setRemoteAddr(const sockaddr_un &remoteAddr);
        const sockaddr_un &     getRemoteAddr() const;

        std::string             getPipeName() const;

    private:
        sockaddr_un             mRemoteAddr;
        const std::string       mFileName;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_UNIXLOCALCLIENTTRANSPORT_HPP
