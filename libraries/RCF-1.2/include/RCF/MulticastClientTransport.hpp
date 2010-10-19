
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_MULTICASTCLIENTTRANSPORT_HPP
#define INCLUDE_RCF_MULTICASTCLIENTTRANSPORT_HPP

#include <list>
#include <memory>
#include <string>

#include <boost/shared_ptr.hpp>

#include <RCF/ClientTransport.hpp>
#include <RCF/Export.hpp>
#include <RCF/ThreadLibrary.hpp>

namespace RCF {

    /// Special purpose client transport for sending messages in parallel on multiple sub-transports.
    class RCF_EXPORT MulticastClientTransport : public I_ClientTransport
    {
    public:
        std::auto_ptr<I_ClientTransport> clone() const;

        EndpointPtr getEndpointPtr() const;

        int         send(
                        I_ClientTransportCallback &     clientStub, 
                        const std::vector<ByteBuffer> & data, 
                        unsigned int                    timeoutMs);

        int         receive(
                        I_ClientTransportCallback &     clientStub, 
                        ByteBuffer &                    byteBuffer, 
                        unsigned int                    timeoutMs);

        bool        isConnected();

        void        connect(
                        I_ClientTransportCallback &     clientStub, 
                        unsigned int                    timeoutMs);

        void        disconnect(
                        unsigned int                    timeoutMs);

        void        addTransport(
                        const ClientTransportPtr &      clientTransportPtr);

        void        setTransportFilters(
                        const std::vector<FilterPtr> &  filters);

        void        getTransportFilters(
                        std::vector<FilterPtr> &        filters);

        void        setAsync(bool async);
        TimerEntry  setTimer(boost::uint32_t timeoutMs, I_ClientTransportCallback *pClientStub);
        void        killTimer(const TimerEntry & timerEntry);

    private:
        typedef std::list< ClientTransportPtr >     ClientTransportPtrList;
        Mutex                                       mMutex;
        ClientTransportPtrList                      mClientTransports;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_MULTICASTCLIENTTRANSPORT_HPP
