
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_IPADDRESS_HPP
#define INCLUDE_RCF_IPADDRESS_HPP

#include <string>

#include <RCF/Export.hpp>
#include <RCF/ServerTransport.hpp>

#include <RCF/util/Platform/OS/BsdSockets.hpp>

namespace RCF {

    class I_IpAddress : public I_RemoteAddress
    {
    public:
        virtual std::string             getIp() const = 0;
        virtual int                     getPort() const = 0;
        virtual const sockaddr_in &     getSockAddr() const = 0;        
    };

    class RCF_EXPORT IpAddress : public I_IpAddress
    {
    public:
        IpAddress();
        IpAddress(const sockaddr_in &addr);
        std::string                     getIp() const;
        int                             getPort() const;
        const sockaddr_in &             getSockAddr() const;
        std::string                     string() const;

    private:
        sockaddr_in                     mAddr;
        mutable std::string             mIp;
        mutable int                     mPort;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_IPADDRESS_HPP
