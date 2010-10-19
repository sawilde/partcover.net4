
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/IpAddress.hpp>

namespace RCF {

    IpAddress::IpAddress() :
        mIp(),
        mPort(RCF_DEFAULT_INIT)
    {
        memset(&mAddr, 0, sizeof(mAddr));
    }

    IpAddress::IpAddress(const sockaddr_in &addr) :
        mAddr(addr),
        mIp(),
        mPort(RCF_DEFAULT_INIT)
    {
    }

    std::string IpAddress::getIp() const
    {
        if (mIp == "")
        {
            mIp = inet_ntoa(mAddr.sin_addr);
        }
        return mIp;
    }

    int IpAddress::getPort() const
    {
        if (mPort == 0)
        {
            mPort = ntohs(mAddr.sin_port);
        }
        return mPort;
    }

    const sockaddr_in &IpAddress::getSockAddr() const
    {
        return mAddr;
    }

    std::string IpAddress::string() const
    {
        std::ostringstream os;
        os << getIp() << ":" << getPort();
        return os.str();
    }

} // namespace RCF
