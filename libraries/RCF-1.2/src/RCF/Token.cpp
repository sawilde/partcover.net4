
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/Token.hpp>

#ifdef RCF_USE_SF_SERIALIZATION
#include <SF/Archive.hpp>
#endif

#include <iostream>

namespace RCF {

    //*****************************************
    // Token

    Token::Token() :
        mId(RCF_DEFAULT_INIT)
    {}

    bool operator<(const Token &lhs, const Token &rhs)
    {
        return (lhs.getId() < rhs.getId());
    }

    bool operator==(const Token &lhs, const Token &rhs)
    {
        return lhs.getId() == rhs.getId();
    }

    bool operator!=(const Token &lhs, const Token &rhs)
    {
        return ! (lhs == rhs);
    }

    int Token::getId() const
    {
        return mId;
    }
   
    std::ostream &operator<<(std::ostream &os, const Token &token)
    {
        os << "( id = " << token.getId() << " )";
        return os;
    }

    Token::Token(int id) :
        mId(id)
    {}

#ifdef RCF_USE_SF_SERIALIZATION

    void Token::serialize(SF::Archive &ar)
    {
        ar & mId;
    }

#endif

    // TokenFactory

#if defined(_MSC_VER) && _MSC_VER <= 1200
#define for if (0) {} else for
#endif

    TokenFactory::TokenFactory(int tokenCount) :
        mMutex(WriterPriority)
    {
        for (int i=0; i<tokenCount; i++)
        {
            mTokenSpace.push_back( Token(i+1) );
        }

#if defined(_MSC_VER) && _MSC_VER < 1310 && !(defined(__SGI_STL_PORT) || defined(_STLPORT_VERSION))

        //for (std::size_t i = mTokenSpace.size() - 1; i >= 0; --i)
        //{
        //    mAvailableTokens.push_back(mTokenSpace[i]);
        //}

        std::for_each(
            mTokenSpace.rbegin(),
            mTokenSpace.rend(),
            boost::bind(
                &std::vector<Token>::push_back,
                boost::ref(mAvailableTokens),
                _1));

#else

        mAvailableTokens.assign( mTokenSpace.rbegin(), mTokenSpace.rend() );

#endif

    }

#if defined(_MSC_VER) && _MSC_VER <= 1200
#undef for
#endif

    bool TokenFactory::requestToken(Token &token)
    {
        WriteLock writeLock(mMutex);
        RCF_UNUSED_VARIABLE(writeLock);
        if (mAvailableTokens.empty())
        {
            RCF_TRACE("no more tokens available")
                (mAvailableTokens.size())(mTokenSpace.size());
            return false;
        }
        else
        {
            Token myToken = mAvailableTokens.back();
            mAvailableTokens.pop_back();
            token = myToken;
            return true;
        }
    }

    void TokenFactory::returnToken(const Token &token)
    {
        // TODO: perhaps should verify that the token is part of the token 
        // space as well...
        if (token != Token())
        {
            WriteLock writeLock(mMutex);
            RCF_UNUSED_VARIABLE(writeLock);
            mAvailableTokens.push_back(token);
        }
    }

    const std::vector<Token> &TokenFactory::getTokenSpace()
    {
        return mTokenSpace;
    }

    std::size_t TokenFactory::getAvailableTokenCount()
    {
        ReadLock readLock( mMutex );
        RCF_UNUSED_VARIABLE(readLock);
        return mAvailableTokens.size();
    }

    bool TokenFactory::isAvailable(const Token & token)
    {
        ReadLock readLock( mMutex );
        return 
            std::find(mAvailableTokens.begin(), mAvailableTokens.end(), token) 
            != mAvailableTokens.end();
    }
   
} // namespace RCF
