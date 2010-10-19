
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/StubEntry.hpp>

#include <RCF/Exception.hpp>
#include <RCF/Tools.hpp>

#include <time.h>

namespace RCF {

    // time in s since ca 1970, may fail after year 2038
    inline unsigned int getCurrentTimeS()
    {
        return static_cast<unsigned int>(time(NULL));
    }

    TokenMapped::TokenMapped() :
        mTimeStamp(getCurrentTimeS())
    {
    }

    StubEntry::StubEntry(const RcfClientPtr &rcfClientPtr) :
        mRcfClientPtr(rcfClientPtr)
    {
        RCF_ASSERT(rcfClientPtr);
    }

    RcfClientPtr StubEntry::getRcfClientPtr() const
    {
        return mRcfClientPtr;
    }

    void TokenMapped::touch()
    {
        // TODO: if we need sync at all for this, then InterlockedExchange etc
        // would be better

        Lock lock(mMutex);
        mTimeStamp = getCurrentTimeS();
    }

    unsigned int TokenMapped::getElapsedTimeS() const
    {
        Lock lock(mMutex);
        if (mTimeStamp == 0)
        {
            return 0;
        }
        else
        {
            unsigned int currentTimeS = getCurrentTimeS();
            return currentTimeS > mTimeStamp ?
                currentTimeS - mTimeStamp :
                mTimeStamp - currentTimeS;
        }
    }

} // namespace RCF
