
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/Service.hpp>

#include <RCF/RcfServer.hpp>

namespace RCF {

    I_Service::I_Service() :
        mTaskEntriesMutex(WriterPriority)
    {}

    ReadWriteMutex &I_Service::getTaskEntriesMutex()
    {
        return mTaskEntriesMutex;
    }

    TaskEntries &I_Service::getTaskEntries()
    {
        return mTaskEntries;
    }

    void I_Service::onServiceAdded(RcfServer &server) 
    {
        RCF_UNUSED_VARIABLE(server);
    }

    void I_Service::onServiceRemoved(RcfServer &server) 
    {
        RCF_UNUSED_VARIABLE(server);
    }

    void I_Service::onServerOpen(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);
    }
   
    void I_Service::onServerClose(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);
    }
   
    void I_Service::onServerStart(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);
    }
   
    void I_Service::onServerStop(RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);
    }

} // namespace RCF
