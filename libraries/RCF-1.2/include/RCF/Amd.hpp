
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_AMD_HPP
#define INCLUDE_RCF_AMD_HPP

#include <boost/any.hpp>

#include <RCF/Export.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ThreadLocalData.hpp>

// Temporary hack for now
#include <RCF/TcpIocpServerTransport.hpp>

namespace RCF {

    class I_Parameters;

    class RCF_EXPORT AmdImpl
    {
    public:

        AmdImpl()
        {
            mRcfSessionPtr = RCF::getCurrentRcfSessionPtr();
            mRcfSessionPtr->mAutoSend = false;

            mpParametersUntyped = mRcfSessionPtr->mpParameters;

            IocpSessionState & sessionState = 
                dynamic_cast<IocpSessionState &>(
                    mRcfSessionPtr->getProactor());

            mSessionStatePtr = sessionState.shared_from_this();
        }

        void commit()
        {
            mRcfSessionPtr->sendResponse();
            mpParametersUntyped = NULL;
            mRcfSessionPtr.reset();
            mSentry = boost::any();

            mSessionStatePtr.reset();
        }

        void commit(const std::exception &e)
        {
            mRcfSessionPtr->sendResponseException(e);
            mpParametersUntyped = NULL;
            mRcfSessionPtr.reset();
            mSentry = boost::any();

            mSessionStatePtr.reset();
        }

    private:
        boost::any          mSentry;
        RcfSessionPtr       mRcfSessionPtr;
        
        // Temporary hack to keep the session state alive...
        IocpSessionStatePtr mSessionStatePtr;

    protected:
        I_Parameters *      mpParametersUntyped;

    };

    template<
        typename R, 
        typename A1 = Void,
        typename A2 = Void,
        typename A3 = Void,
        typename A4 = Void,
        typename A5 = Void,
        typename A6 = Void,
        typename A7 = Void,
        typename A8 = Void>
    class Amd : public AmdImpl
    {
    public:

        typedef ServerParameters<R, A1, A2, A3, A4, A5, A6, A7, A8> ParametersT;

        Amd()
        {
            RCF_ASSERT( dynamic_cast<ParametersT *>(mpParametersUntyped) );
        }

        ParametersT &parameters()
        {
            return * static_cast<ParametersT *>(mpParametersUntyped);;
        }
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_AMD_HPP


