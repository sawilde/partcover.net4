
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/ServerStub.hpp>

#include <iterator>

#include <RCF/RcfClient.hpp>

namespace RCF {

    void ServerStub::invoke(
        const std::string &         subInterface,
        int                         fnId,
        RcfSession &                session)
    {
        // no mutex here, since there is never anyone writing to mInvokeFunctorMap

        RCF_VERIFY(
            mInvokeFunctorMap.find(subInterface) != mInvokeFunctorMap.end(),
            Exception(_RcfError_UnknownInterface(subInterface)))
            (subInterface)(fnId)(mInvokeFunctorMap.size())(mMergedStubs.size());

        mInvokeFunctorMap[subInterface](fnId, session);
    }

    void ServerStub::merge(RcfClientPtr rcfClientPtr)
    {
        InvokeFunctorMap &invokeFunctorMap =
            rcfClientPtr->getServerStub().mInvokeFunctorMap;

        std::copy(
            invokeFunctorMap.begin(),
            invokeFunctorMap.end(),
            std::insert_iterator<InvokeFunctorMap>(
                mInvokeFunctorMap,
                mInvokeFunctorMap.begin()));

        invokeFunctorMap.clear();

        mMergedStubs.push_back(rcfClientPtr);
    }

} // namespace RCF
