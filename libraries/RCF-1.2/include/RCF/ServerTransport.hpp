
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_SERVERTRANSPORT_HPP
#define INCLUDE_RCF_SERVERTRANSPORT_HPP

#include <memory>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <RCF/ByteBuffer.hpp>
#include <RCF/Export.hpp>

namespace RCF {

    class Filter;
    class I_Endpoint;
    class I_ClientTransport;
    class I_Proactor;
    class I_Session;
    class StubEntry;

    typedef boost::shared_ptr<Filter>               FilterPtr;
    typedef boost::shared_ptr<I_Proactor>           ProactorPtr;
    typedef std::auto_ptr<I_ClientTransport>        ClientTransportAutoPtr;
    typedef boost::shared_ptr<StubEntry>            StubEntryPtr;

    class I_ServerTransport;
    typedef boost::shared_ptr<I_ServerTransport>    ServerTransportPtr;
    typedef std::auto_ptr<I_ServerTransport>        ServerTransportAutoPtr;

    class I_RemoteAddress
    {
    public:
        virtual ~I_RemoteAddress()
        {}

        virtual std::string string() const
        {
            return "";
        }
    };

    class NoRemoteAddress : public I_RemoteAddress
    {};
   
    /// Interface by which the session manager reads/writes data on a given session.
    /// Each session has a polymorphic I_Proactor object.
    class I_Proactor
    {
    public:

        I_Proactor() :
            mEnableReconnect(true),
            mDropReceiveBuffer(false)
        {
        }

        /// Virtual destructor.
        virtual ~I_Proactor() {}

        /// Posts a read request to the server transport.
        virtual void postRead() = 0;
        virtual ByteBuffer getReadByteBuffer() = 0;

        /// Posts a write request to the server transport.
        virtual void postWrite(std::vector<ByteBuffer> &byteBuffers) = 0;

        /// Posts a close request to the server transport. Not implemented.
        virtual void postClose() = 0;
       
        /// Returns a reference to the server transport.
        /// \return Reference to the server transport.
        virtual I_ServerTransport &getServerTransport() = 0;

        /// Returns a I_RemoteAddress interface containing information on the remote address of the client of this session.
        /// \return I_RemoteAddress reference.
        virtual const I_RemoteAddress &getRemoteAddress() = 0;

        /// Sets the transport filter sequence of this session, which will be used on the next read or write operation.
        /// \param filters Sequence of filters to be used as transport filters on the session.
        virtual void setTransportFilters(const std::vector<FilterPtr> &filters) = 0;

        virtual void getTransportFilters(std::vector<FilterPtr> &filters) = 0;

        void setEnableReconnect(bool enableReconnect)
        {
            mEnableReconnect = enableReconnect;
        }
        bool getEnableReconnect()
        {
            return mEnableReconnect;
        }

        void setDropReceiveBuffer(bool dropReceiveBuffer)
        {
            mDropReceiveBuffer = dropReceiveBuffer;
        }

        bool getDropReceiveBuffer()
        {
            return mDropReceiveBuffer;
        }

    protected:
        bool mEnableReconnect;
        bool mDropReceiveBuffer;
    };

    typedef boost::shared_ptr<I_Proactor> ProactorPtr;

    /// Interface for a generic server session.
    class I_Session
    {
    public:
        virtual ~I_Session() {}

        I_Proactor & getProactor() const
        {
            return *mpProactor;
        }

        void setProactor(I_Proactor & proactor)
        {
            mpProactor = &proactor;
        }

    private:
        I_Proactor * mpProactor;
    };

    typedef boost::shared_ptr<I_Session> SessionPtr;

    /// Base class of all server transport services.
    class RCF_EXPORT I_ServerTransport
    {
    public:
        /// Constructor.
        I_ServerTransport();

        /// Virtual destructor.
        virtual ~I_ServerTransport() {}

        /// Clones the server transport
        /// \return A cloned copy of this transport.
        virtual ServerTransportPtr clone() = 0;

        void            setMaxMessageLength(std::size_t maxMessageLength);
        std::size_t     getMaxMessageLength() const;

        std::size_t     getConnectionLimit() const;
        void            setConnectionLimit(std::size_t connectionLimit);
        
    private:

        mutable ReadWriteMutex      mReadWriteMutex;
        std::size_t                 mMaxMessageLength;
        std::size_t                 mConnectionLimit;
    };

    /// Additional base class for server transport services with extended stream-oriented functionality.
    ///
    /// Twoway, stream-oriented server transport services that are to be used with publish/subscribe need to implement I_ServerTransportEx.
    /// The functions in I_ServerTransportEx are all synchronized and can safely be called from any thread.
    class I_ServerTransportEx
    {
    public:

        /// Virtual destructor.
        virtual ~I_ServerTransportEx() {}

        /// Creates a client transport to the given endpoint.
        /// \param endpoint Endpoint describing a remote server.
        /// \return Auto pointer to client transport accessing the remote server described by the endpoint.
        virtual ClientTransportAutoPtr createClientTransport(
            const I_Endpoint &endpoint) = 0;
       
        /// Creates a server session dual to the given client transport.
        /// \param clientTransportAutoPtr Auto pointer to client transport, from which the server session should be generated.
        /// \return Server session.
        virtual SessionPtr createServerSession(
            ClientTransportAutoPtr clientTransportAutoPtr,
            StubEntryPtr stubEntryPtr) = 0;

        /// Creates a client transport dual to the given server session.
        /// \param sessionPtr Server session from which to generate the client transport.
        /// \return Auto pointer to client transport.
        virtual ClientTransportAutoPtr createClientTransport(
            SessionPtr sessionPtr) = 0;
       
        /// Requests the server transport to start reflecting data between the two given sessions. All data subsequently read on the first session will
        /// be sent out on the second session, and vice versa.
        /// \param sessionPtr1 First server session
        /// \param sessionPtr2 Second server session.
        /// \return true if server transport acquiesces, false otherwise.
        virtual bool reflect(
            const SessionPtr &sessionPtr1,
            const SessionPtr &sessionPtr2) = 0;
       
        /// Attempts to determine whether a server session is still connected. Returns true, unless it can be determined that the
        /// physical transport instance is no longer valid. For a TCP server session, this amounts to checking if the underlying socket is ready
        /// to receive and has no errors queued, in other words that the TCP connection is still open.
        /// \param sessionPtr Session to check.
        /// \return Boolean value indicating the server transports best-effort knowledge of the state of the session.
        virtual bool isConnected(const SessionPtr &sessionPtr) = 0;
    };
   
    /// Base class for session managers, in particular RcfServer.
    ///
    /// Server transports use I_SessionManager to communicate with the RcfServer class.
    class I_SessionManager
    {
    public:
        /// Virtual destructor.
        virtual ~I_SessionManager() {}

        /// Requests the session manager to create a new session.
        /// \return Pointer to newly created session.
        virtual SessionPtr createSession() = 0;

        /// Callback, by which the server transport indicates that a read operation has completed on the session.
        /// \param sessionPtr Session for which the read operation has completed.
        virtual void onReadCompleted(const SessionPtr &sessionPtr) = 0;

        /// Callback, by which the server transport indicates that a write operation has completed on the session.
        /// \param sessionPtr Session for which the write operation has completed.
        virtual void onWriteCompleted(const SessionPtr &sessionPtr) = 0;
    };

    RCF_EXPORT std::size_t  getDefaultMaxMessageLength();

    RCF_EXPORT void         setDefaultMaxMessageLength(
                                std::size_t maxMessageLength);

} // namespace RCF

#endif // ! INCLUDE_RCF_SERVERTRANSPORT_HPP
