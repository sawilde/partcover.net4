
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_CLIENTTRANSPORT_HPP
#define INCLUDE_RCF_CLIENTTRANSPORT_HPP

#include <memory>
#include <string>
#include <vector>

#include <boost/cstdint.hpp>

#include <RCF/AsyncFilter.hpp>
#include <RCF/ByteBuffer.hpp>
#include <RCF/Export.hpp>

namespace RCF {

    class I_Endpoint;
    typedef boost::shared_ptr<I_Endpoint> EndpointPtr;

    class RcfServer;

    class OverlappedAmi;
    typedef boost::shared_ptr<OverlappedAmi> OverlappedAmiPtr;

    class RCF_EXPORT I_ClientTransportCallback
    {
    public:
        I_ClientTransportCallback() : mpAsyncDispatcher(RCF_DEFAULT_INIT) {}
        virtual ~I_ClientTransportCallback() {}
        virtual void onConnectCompleted(bool alreadyConnected = false) = 0;
        virtual void onSendCompleted() = 0;
        virtual void onReceiveCompleted() = 0;
        virtual void onTimerExpired() = 0;
        virtual void onError(const std::exception &e) = 0;

        void setAsyncDispatcher(RcfServer & server);
        RcfServer * getAsyncDispatcher();

    private:
        RcfServer * mpAsyncDispatcher;
    };

    class RCF_EXPORT I_ClientTransport
    {
    public:
        I_ClientTransport();

        virtual ~I_ClientTransport()
        {}

        virtual 
        std::auto_ptr<I_ClientTransport> clone() const = 0;

        virtual 
        EndpointPtr getEndpointPtr() const = 0;
       
        virtual 
        int send(
            I_ClientTransportCallback &     clientStub, 
            const std::vector<ByteBuffer> & data, 
            unsigned int                    timeoutMs) = 0;

        virtual 
        int receive(
            I_ClientTransportCallback &     clientStub, 
            ByteBuffer &                    byteBuffer, 
            unsigned int                    timeoutMs) = 0;

        virtual 
        bool isConnected() = 0;

        virtual 
        void connect(
            I_ClientTransportCallback &     clientStub, 
            unsigned int                    timeoutMs) = 0;

        virtual 
        void disconnect(
            unsigned int                    timeoutMs = 0) = 0;

        virtual 
        void setTransportFilters(
            const std::vector<FilterPtr> &  filters) = 0;
       
        virtual 
        void getTransportFilters(
            std::vector<FilterPtr> &        filters) = 0;


        void setMaxMessageLength(
            std::size_t                     maxMessageLength);

        std::size_t getMaxMessageLength() const;

        // TODO: clean this up a bit
        virtual void setAsync(bool async) = 0;
        virtual void cancel() {}

        typedef std::pair<boost::uint32_t, OverlappedAmiPtr> TimerEntry;

        virtual TimerEntry setTimer(
            boost::uint32_t timeoutMs,
            I_ClientTransportCallback * pClientStub = NULL) = 0;

        virtual void killTimer(const TimerEntry & timerEntry) = 0;

    private:
        std::size_t mMaxMessageLength;

    };

    typedef boost::shared_ptr<I_ClientTransport> ClientTransportPtr;

    typedef std::auto_ptr<I_ClientTransport> ClientTransportAutoPtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_CLIENTTRANSPORT_HPP
