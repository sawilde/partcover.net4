
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/CurrentSerializationProtocol.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    SerializationProtocolIn *getCurrentSerializationProtocolIn()
    {
        ClientStubPtr clientStubPtr = RCF::getCurrentClientStubPtr();
        RcfSessionPtr rcfSessionPtr = RCF::getCurrentRcfSessionPtr();
        if (clientStubPtr)
        {
            return &clientStubPtr->getSpIn();
        }
        else if (rcfSessionPtr)
        {
            return &rcfSessionPtr->getSpIn();
        }
        else
        {
            return NULL;
        }
    }

    SerializationProtocolOut *getCurrentSerializationProtocolOut()
    {
        ClientStubPtr clientStubPtr = RCF::getCurrentClientStubPtr();
        RcfSessionPtr rcfSessionPtr = RCF::getCurrentRcfSessionPtr();
        if (clientStubPtr)
        {
            return &clientStubPtr->getSpOut();
        }
        else if (rcfSessionPtr)
        {
            return &rcfSessionPtr->getSpOut();
        }
        else
        {
            return NULL;
        }
    }

} // namespace RCF
