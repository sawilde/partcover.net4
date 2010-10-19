
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/ServerTransport.hpp>

namespace RCF {

    I_ServerTransport::I_ServerTransport() :
        mReadWriteMutex(WriterPriority),
        mMaxMessageLength(getDefaultMaxMessageLength()),
        mConnectionLimit(RCF_DEFAULT_INIT)
    {}

    void I_ServerTransport::setMaxMessageLength(std::size_t maxMessageLength)
    {
        WriteLock writeLock(mReadWriteMutex);
        mMaxMessageLength = maxMessageLength;
    }

    std::size_t I_ServerTransport::getMaxMessageLength() const
    {
        ReadLock readLock(mReadWriteMutex);
        return mMaxMessageLength;
    }

    std::size_t I_ServerTransport::getConnectionLimit() const
    {
        ReadLock readLock(mReadWriteMutex);
        return mConnectionLimit;
    }

    void I_ServerTransport::setConnectionLimit(std::size_t connectionLimit)
    {
        WriteLock writeLock(mReadWriteMutex);
        mConnectionLimit = connectionLimit;
    }

    std::size_t gDefaultMaxMessageLength = 10*1024; // 10 kb

    std::size_t getDefaultMaxMessageLength()
    {
        return gDefaultMaxMessageLength;
    }

    void setDefaultMaxMessageLength(std::size_t maxMessageLength)
    {
        gDefaultMaxMessageLength = maxMessageLength;
    }

} // namespace RCF
