
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_STUBENTRY_HPP
#define INCLUDE_RCF_STUBENTRY_HPP

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/ThreadLibrary.hpp>

namespace RCF {

    class                                       I_RcfClient;
    class                                       StubEntry;
    typedef boost::shared_ptr<I_RcfClient>      RcfClientPtr;
    typedef boost::shared_ptr<StubEntry>        StubEntryPtr;

    class TokenMapped
    {
    public:

        TokenMapped();

        virtual ~TokenMapped()
        {}

        virtual void            touch();
        virtual unsigned int    getElapsedTimeS() const;

    private:
        mutable Mutex   mMutex;
        unsigned int    mTimeStamp;
    };

    typedef boost::shared_ptr<TokenMapped> TokenMappedPtr;

    // TODO: collapse this class into a RcfClientPtr.
    class RCF_EXPORT StubEntry : 
        public TokenMapped,
        boost::noncopyable
    {
    public:
        StubEntry(const RcfClientPtr &rcfClientPtr);
        RcfClientPtr    getRcfClientPtr() const;

    private:
        RcfClientPtr    mRcfClientPtr;        
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_STUBENTRY_HPP
