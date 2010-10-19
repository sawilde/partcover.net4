
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_IPCLIENTTRANSPORT_HPP
#define INCLUDE_RCF_IPCLIENTTRANSPORT_HPP

#include <string>

#include <RCF/Export.hpp>

namespace RCF {

    class RCF_EXPORT I_IpClientTransport
    {
    public:
        virtual         ~I_IpClientTransport();
        virtual int     getSocket() = 0;
        std::string     getLocalIp();
        int             getLocalPort();
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_IPCLIENTTRANSPORT_HPP
