
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_SERVICE_HPP
#define INCLUDE_RCF_SERVICE_HPP

#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/ServerTask.hpp>
#include <RCF/ThreadLibrary.hpp>

namespace RCF {

    class                                   I_Service;
    class                                   RcfServer;
    class                                   StubEntry;
    class                                   Token;
    typedef boost::shared_ptr<I_Service>    ServicePtr;
    typedef boost::shared_ptr<StubEntry>    StubEntryPtr;

    /// Base class for RcfServer plug-in services.
    class RCF_EXPORT I_Service
    {
    public:
        /// Constructor.
        I_Service();

        // Virtual destructor.
        virtual ~I_Service()
        {}
       
        /// Invoked when the service is added to a server.
        /// \param server RcfServer object to which the service is being added.
        virtual void onServiceAdded(RcfServer &server);

        /// Invoked when the service is removed from a server.
        /// \param server RcfServer object from which the service is being removed.
        virtual void onServiceRemoved(RcfServer &server);

        /// Invoked when the server is opened.
        /// \param server RcfServer object to which the service has been added.
        virtual void onServerOpen(RcfServer &server);

        /// Invoked when the server is closed.
        /// \param server RcfServer object to which the service has been added.
        virtual void onServerClose(RcfServer &server);

        /// Invoked when the server is started, before any server transport threads are spawned.
        /// \param server RcfServer object to which the service has been added.
        virtual void onServerStart(RcfServer &server);

        /// Invoked when the server is stopped, after all server transport threads have been stopped.
        /// \param server RcfServer object to which the service has been added.
        virtual void onServerStop(RcfServer &server);

        ReadWriteMutex &    getTaskEntriesMutex();
        TaskEntries &       getTaskEntries();

    private:
        ReadWriteMutex      mTaskEntriesMutex;
        TaskEntries         mTaskEntries;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_SERVICE_HPP
