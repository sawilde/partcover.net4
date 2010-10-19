
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/UdpEndpoint.hpp>

#include <RCF/InitDeinit.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/UdpClientTransport.hpp>
#include <RCF/UdpServerTransport.hpp>

#ifdef RCF_USE_SF_SERIALIZATION
#include <SF/Registry.hpp>
#endif

namespace RCF {

    UdpEndpoint::UdpEndpoint() :
        mIp(),
        mPort(RCF_DEFAULT_INIT),
        mEnableSharedAddressBinding(RCF_DEFAULT_INIT)
    {}

    UdpEndpoint::UdpEndpoint(int port) :
        mIp("127.0.0.1"),
        mPort(port),
        mEnableSharedAddressBinding(RCF_DEFAULT_INIT)
    {}

    UdpEndpoint::UdpEndpoint(const std::string &ip, int port) :
        mIp(ip),
        mPort(port),
        mEnableSharedAddressBinding(RCF_DEFAULT_INIT)
    {}

    UdpEndpoint::UdpEndpoint(const UdpEndpoint &rhs) :
        mIp(rhs.mIp),
        mPort(rhs.mPort),
        mMulticastIp(rhs.mMulticastIp),
        mEnableSharedAddressBinding(rhs.mEnableSharedAddressBinding)
    {}

    UdpEndpoint & UdpEndpoint::enableSharedAddressBinding(bool enable)
    {
        mEnableSharedAddressBinding = enable;
        return *this;
    }

    UdpEndpoint & UdpEndpoint::listenOnMulticast(const std::string & multicastIp)
    {
        mMulticastIp = multicastIp;

        if (!mMulticastIp.empty())
        {
            mEnableSharedAddressBinding = true;
        }

        return *this;
    }

    EndpointPtr UdpEndpoint::clone() const
    {
        return EndpointPtr(new UdpEndpoint(*this));
    }

    std::string UdpEndpoint::getIp() const
    {
        return mIp;
    }

    int UdpEndpoint::getPort() const
    {
        return mPort;
    }

    ServerTransportAutoPtr UdpEndpoint::createServerTransport() const
    {
        std::auto_ptr<UdpServerTransport> udpServerTransportPtr(
            new UdpServerTransport(mIp, mPort, mMulticastIp));

        if (mEnableSharedAddressBinding)
        {
            udpServerTransportPtr->enableSharedAddressBinding();
        }

        return ServerTransportAutoPtr(udpServerTransportPtr.release());
    }

    std::auto_ptr<I_ClientTransport> UdpEndpoint::createClientTransport() const
    {
        return std::auto_ptr<I_ClientTransport>(
            new UdpClientTransport(mIp, mPort));
    }

    std::string UdpEndpoint::asString()
    {
        std::ostringstream os;
        os << "UDP endpoint " << mIp << ":" << mPort;
        return os.str();
    }


#ifdef RCF_USE_SF_SERIALIZATION

    void UdpEndpoint::serialize(SF::Archive &ar)
    {
        serializeParent( (I_Endpoint*) 0, ar, *this);
        ar & mIp & mPort;
    }

#endif

    inline void initUdpEndpointSerialization()
    {
#ifdef RCF_USE_SF_SERIALIZATION
        SF::registerType( (UdpEndpoint *) 0, "RCF::UdpEndpoint");
        SF::registerBaseAndDerived( (I_Endpoint *) 0, (UdpEndpoint *) 0);
#endif
    }

    RCF_ON_INIT_NAMED( initUdpEndpointSerialization(), InitUdpEndpointSerialization );

} // namespace RCF
