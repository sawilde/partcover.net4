
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/IpClientTransport.hpp>

#include <RCF/Exception.hpp>
#include <RCF/Tools.hpp>
#include <RCF/util/Platform/OS/BsdSockets.hpp>

namespace RCF {

    I_IpClientTransport::~I_IpClientTransport()
    {
    }

    std::string I_IpClientTransport::getLocalIp()
    {
        int fd = getSocket();
        sockaddr_in addr = {0};
        Platform::OS::BsdSockets::socklen_t len = sizeof(addr);
        int ret = getsockname(fd, (sockaddr *) &addr, &len);
        int err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(ret != -1, Exception(_RcfError_Socket()))(ret)(err);
        return std::string(inet_ntoa(addr.sin_addr));
    }

    int I_IpClientTransport::getLocalPort()
    {
        int fd = getSocket();
        sockaddr_in addr = {0};
        Platform::OS::BsdSockets::socklen_t len = sizeof(addr);
        int ret = getsockname(fd, (sockaddr *) &addr, &len);
        int err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(ret != -1, Exception(_RcfError_Socket()))(ret)(err);
        return ntohs(addr.sin_port);
    }

} // namespace RCF
