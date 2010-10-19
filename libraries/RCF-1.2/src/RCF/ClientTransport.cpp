
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/ClientTransport.hpp>

#include <RCF/Exception.hpp>
#include <RCF/ServerTransport.hpp>

namespace RCF {

    I_ClientTransport::I_ClientTransport() :
        mMaxMessageLength(getDefaultMaxMessageLength())
    {}

    bool I_ClientTransport::isConnected()
    {
        return true;
    }

    void I_ClientTransport::setMaxMessageLength(std::size_t maxMessageLength)
    {
        mMaxMessageLength = maxMessageLength;
    }

    std::size_t I_ClientTransport::getMaxMessageLength() const
    {
        return mMaxMessageLength;
    }

    void I_ClientTransportCallback::setAsyncDispatcher(RcfServer & server)
    {
        RCF_ASSERT(!mpAsyncDispatcher);
        mpAsyncDispatcher = &server;
    }

    RcfServer * I_ClientTransportCallback::getAsyncDispatcher()
    {
        return mpAsyncDispatcher;
    }

} // namespace RCF
