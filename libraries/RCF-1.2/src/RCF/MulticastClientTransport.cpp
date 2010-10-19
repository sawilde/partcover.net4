
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/MulticastClientTransport.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/Exception.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    ClientTransportAutoPtr MulticastClientTransport::clone() const
    {
        RCF_ASSERT(0);
        return ClientTransportAutoPtr();
    }

    EndpointPtr MulticastClientTransport::getEndpointPtr() const
    {
        RCF_ASSERT(0);
        return EndpointPtr();
    }

    void reswap(
        std::list<ClientTransportPtr> &to,
        std::list<ClientTransportPtr> &from,
        Mutex &mutex)
    {
        Lock lock(mutex);
        to.swap(from);
        if (!from.empty())
        {
            std::copy(
                from.begin(),
                from.end(),
                std::back_inserter(to));
        }
    }

    class DummyCallback : public I_ClientTransportCallback
    {
    public:
        void onConnectCompleted(bool alreadyConnected = false)
        {
            RCF_UNUSED_VARIABLE(alreadyConnected);
        }

        void onSendCompleted()
        {}
        
        void onReceiveCompleted()
        {}

        void onTimerExpired()
        {}
        
        void onError(const std::exception &e)
        {
            RCF_UNUSED_VARIABLE(e);
        }
    };

    int MulticastClientTransport::send(
        I_ClientTransportCallback &clientStub,
        const std::vector<ByteBuffer> &data,
        unsigned int timeoutMs)
    {
        // TODO: in some cases, may need to make a full copy of data for 
        // each individual sub-transport, as they might transform the data.

        ClientTransportPtrList clientTransports;
        {
            Lock lock(mMutex);
            clientTransports.swap(mClientTransports);
        }

        using namespace boost::multi_index::detail;
        scope_guard guard = make_guard(
            reswap,
            boost::ref(mClientTransports),
            boost::ref(clientTransports),
            boost::ref(mMutex));
        RCF_UNUSED_VARIABLE(guard);

        // TODO: hardcoded
        timeoutMs = 1000;
        bool needToRemove = false;
        for (
            ClientTransportPtrList::iterator it = clientTransports.begin();
            it != clientTransports.end();
            ++it)
        {
            try
            {
                if ((*it)->isConnected())
                {
                    // Sending synchronously, so no use for the callback
                    DummyCallback dummyCallback;
                    (*it)->send(dummyCallback, data, timeoutMs);
                }
                else
                {
                    needToRemove = true;
                    (*it).reset();
                }
            }
            catch(const Exception &e)
            {
                RCF_TRACE("")(e);
                needToRemove = true;
                (*it).reset();
            }
        }

        if (needToRemove)
        {
            clientTransports.remove( ClientTransportPtr() );
        }       

        clientStub.onSendCompleted();

        return 1;
    }

    int MulticastClientTransport::receive(
        I_ClientTransportCallback &clientStub,
        ByteBuffer &byteBuffer,
        unsigned int timeoutMs)
    {
        RCF_UNUSED_VARIABLE(clientStub);
        RCF_UNUSED_VARIABLE(byteBuffer);
        RCF_UNUSED_VARIABLE(timeoutMs);
        RCF_ASSERT(0);
        return 1;
    }

    bool MulticastClientTransport::isConnected()
    {
        return true;
    }

    void MulticastClientTransport::connect(I_ClientTransportCallback &clientStub, unsigned int timeoutMs)
    {
        RCF_UNUSED_VARIABLE(clientStub);
        RCF_UNUSED_VARIABLE(timeoutMs);
        clientStub.onConnectCompleted(true);
    }

    void MulticastClientTransport::disconnect(unsigned int timeoutMs)
    {
        RCF_UNUSED_VARIABLE(timeoutMs);
    }

    void MulticastClientTransport::addTransport(
        const ClientTransportPtr &clientTransportPtr)
    {
        Lock lock(mMutex);
        mClientTransports.push_back(clientTransportPtr);
    }

    void MulticastClientTransport::setTransportFilters(
        const std::vector<FilterPtr> &)
    {
        // not supported
    }

    void MulticastClientTransport::getTransportFilters(
        std::vector<FilterPtr> &)
    {
        // not supported
    }

    void MulticastClientTransport::setAsync(bool async)
    {
        RCF_ASSERT(!async);
    }

    MulticastClientTransport::TimerEntry MulticastClientTransport::setTimer(
        boost::uint32_t timeoutMs,
        I_ClientTransportCallback *pClientStub)
    {
        RCF_UNUSED_VARIABLE(timeoutMs);
        RCF_UNUSED_VARIABLE(pClientStub);
        return TimerEntry();
    }

    void MulticastClientTransport::killTimer(const TimerEntry & timerEntry)
    {
        RCF_UNUSED_VARIABLE(timerEntry);
    }

} // namespace RCF
