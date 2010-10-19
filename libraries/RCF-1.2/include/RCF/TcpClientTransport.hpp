
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TCPCLIENTTRANSPORT_HPP
#define INCLUDE_RCF_TCPCLIENTTRANSPORT_HPP

#include <RCF/AsyncFilter.hpp>
#include <RCF/BsdClientTransport.hpp>
#include <RCF/ByteOrdering.hpp>
#include <RCF/ClientProgress.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/Exception.hpp>
#include <RCF/Export.hpp>
#include <RCF/IpClientTransport.hpp>

namespace RCF {

    class RCF_EXPORT TcpClientTransport : 
        public BsdClientTransport, 
        public I_IpClientTransport
    {
    public:
        TcpClientTransport(const TcpClientTransport &rhs);
        TcpClientTransport(const std::string &ip, int port);
        TcpClientTransport(const sockaddr_in &remoteAddr);
        TcpClientTransport(int fd);

        ~TcpClientTransport();

        ClientTransportAutoPtr clone() const;

        // I_IpClientTransport
        int                     getSocket();

        void                    implConnect(
                                    I_ClientTransportCallback &clientStub, 
                                    unsigned int timeoutMs);

        void                    implConnectAsync(
                                    I_ClientTransportCallback &clientStub, 
                                    unsigned int timeoutMs);

        void                    implClose();
        EndpointPtr             getEndpointPtr() const;

        void                    setRemoteAddr(const sockaddr_in &remoteAddr);
        const sockaddr_in &     getRemoteAddr() const;

        

    private:

        void                    createSocket();

        void                    beginDnsLookup();
        void                    endDnsLookup();

        static void             dnsLookupTask(
                                    OverlappedAmiPtr overlappedPtr,
                                    const std::string & ip);

        sockaddr_in             mRemoteAddr;
        std::string             mIp;
        int                     mPort;
    };
    
} // namespace RCF

#endif // ! INCLUDE_RCF_TCPCLIENTTRANSPORT_HPP
