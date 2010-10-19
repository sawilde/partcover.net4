
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_IPSERVERTRANSPORT_HPP
#define INCLUDE_RCF_IPSERVERTRANSPORT_HPP

#include <string>
#include <vector>

#include <RCF/Export.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/Tools.hpp>
#include <RCF/util/Platform/OS/BsdSockets.hpp>

namespace RCF {

    class RCF_EXPORT I_IpServerTransport
    {
    public:
                        I_IpServerTransport();
        virtual         ~I_IpServerTransport();

        std::string     getNetworkInterface() const;
        virtual int     getPort() const = 0;
        bool            isClientIpAllowed(const std::string &ip) const;
        bool            isClientAddrAllowed(const sockaddr_in &addr) const;

        void            setNetworkInterface(
                            const std::string &networkInterface);

        void            setAllowedClientIps(
                            const std::vector<std::string> &allowedClientIps);

        std::vector<std::string> 
                        getAllowedClientIps() const;

    private:

        mutable ReadWriteMutex      mReadWriteMutex;
        std::string                 mNetworkInterface;
        std::vector<std::string>    mAllowedIps;
        std::vector<u_long>         mAllowedAddrs;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_IPSERVERTRANSPORT_HPP
