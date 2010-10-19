
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/IpServerTransport.hpp>

namespace RCF {

    I_IpServerTransport::I_IpServerTransport() :
        mReadWriteMutex(WriterPriority),
        mNetworkInterface("127.0.0.1")
    {}

    I_IpServerTransport::~I_IpServerTransport() 
    {}

    void I_IpServerTransport::setNetworkInterface(
        const std::string &networkInterface)
    {
        WriteLock writeLock(mReadWriteMutex);
        mNetworkInterface = networkInterface;
    }

    std::string I_IpServerTransport::getNetworkInterface() const
    {
        ReadLock readLock(mReadWriteMutex);
        return mNetworkInterface;
    }

    void I_IpServerTransport::setAllowedClientIps(
        const std::vector<std::string> &allowedClientIps)
    {
        WriteLock writeLock(mReadWriteMutex);
        mAllowedIps.assign(allowedClientIps.begin(), allowedClientIps.end());

        // translate the ips into 32 bit values, network order
        mAllowedAddrs.clear();
        for (unsigned int i=0; i<mAllowedIps.size(); i++)
        {
            const std::string &ip = mAllowedIps[i];
            hostent *hostDesc = ::gethostbyname( ip.c_str() );
            if (hostDesc == NULL)
            {
                int err = Platform::OS::BsdSockets::GetLastError();
                std::string strErr = Platform::OS::GetErrorString(err);
                RCF_TRACE("gethostbyname() failed")(ip)(err)(strErr);
                mAllowedAddrs.push_back(INADDR_NONE);
            }
            else
            {
                in_addr addr = *((in_addr*) hostDesc->h_addr_list[0]);
                mAllowedAddrs.push_back(addr.s_addr);
            }
        }
    }

    std::vector<std::string> I_IpServerTransport::getAllowedClientIps() const
    {
        ReadLock readLock(mReadWriteMutex);
        return mAllowedIps;
    }

    bool I_IpServerTransport::isClientIpAllowed(const std::string &ip) const
    {
        ReadLock readLock(mReadWriteMutex);
        return
            mAllowedIps.empty() ||
            (std::find(mAllowedIps.begin(), mAllowedIps.end(), ip) !=
                mAllowedIps.end());
    }

    bool I_IpServerTransport::isClientAddrAllowed(const sockaddr_in &addr) const
    {
        ReadLock readLock(mReadWriteMutex);
        return
            mAllowedAddrs.empty() ||
            (std::find(
                mAllowedAddrs.begin(),
                mAllowedAddrs.end(),
                addr.sin_addr.s_addr) != mAllowedAddrs.end());
    }

    int getFdPort(int fd)
    {
        sockaddr_in addr = {0};
        Platform::OS::BsdSockets::socklen_t nameLen = sizeof(addr);
        int ret = getsockname(fd, (sockaddr *) &addr, &nameLen);
        return ret >= 0 ? ntohs(addr.sin_port) : 0;
    }

    

} // namespace RCF
