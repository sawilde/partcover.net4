
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/UnixLocalServerTransport.hpp>

#include <RCF/Asio.hpp>
#include <RCF/UnixLocalClientTransport.hpp>
#include <RCF/UnixLocalEndpoint.hpp>

#include <RCF/util/Platform/OS/BsdSockets.hpp>

// std::remove
#include <cstdio>

namespace RCF {

    using boost::asio::local::stream_protocol;

    class UnixLocalAcceptor : public AsioAcceptor
    {
    public:
        UnixLocalAcceptor(AsioDemuxer & demuxer, const std::string & fileName) : 
            mFileName(fileName),
            mAcceptor(demuxer, stream_protocol::endpoint(fileName))
            
        {}

        ~UnixLocalAcceptor()
        {
            mAcceptor.close();

            // Delete the underlying file as well.
            int ret = std::remove(mFileName.c_str());
            int err = Platform::OS::BsdSockets::GetLastError();

            if (ret != 0)
            {
                // Couldn't delete it, not a whole lot we can do about it.
                RCF_TRACE("Failed to delete underlying file of unix domain socket.")
                    (mFileName)(err)(Platform::OS::GetErrorString(err));
            }
        }

        std::string mFileName;
        stream_protocol::acceptor mAcceptor;
    };

    typedef stream_protocol::socket                 UnixLocalSocket;
    typedef boost::shared_ptr<UnixLocalSocket>      UnixLocalSocketPtr;

    // UnixLocalSessionState

    class UnixLocalSessionState : public AsioSessionState
    {
    public:
        UnixLocalSessionState(
            UnixLocalServerTransport &transport,
            AsioDemuxerPtr demuxerPtr) :
                AsioSessionState(transport, demuxerPtr),
                mSocketPtr(new UnixLocalSocket(*demuxerPtr))
        {}

        const I_RemoteAddress & implGetRemoteAddress()
        {
            RCF_THROW(Exception("Not implemented."));
            return * (I_RemoteAddress *) NULL;
        }

        void implRead(char * buffer, std::size_t bufferLen)
        {
            mSocketPtr->async_receive(
                boost::asio::buffer( buffer, bufferLen),
                0,
                boost::bind(
                    &AsioSessionState::onReadCompletion,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }

        void implWrite(const char * buffer, std::size_t bufferLen)
        {
            boost::asio::async_write(
                *mSocketPtr,
                boost::asio::buffer(buffer, bufferLen),
                boost::bind(
                    &AsioSessionState::onWriteCompletion,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }

        void implWrite(AsioSessionState &toBeNotified, const char * buffer, std::size_t bufferLen)
        {
            boost::asio::async_write(
                *mSocketPtr,
                boost::asio::buffer(buffer, bufferLen),
                boost::bind(
                    &AsioSessionState::onWriteCompletion,
                    toBeNotified.shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }

        void implAccept()
        {
            UnixLocalAcceptor & unixLocalAcceptor = 
                static_cast<UnixLocalAcceptor &>(*mTransport.getAcceptorPtr());

            unixLocalAcceptor.mAcceptor.async_accept(
                *mSocketPtr,
                boost::bind(
                    &AsioSessionState::onAccept,
                    shared_from_this(),
                    boost::asio::placeholders::error));
        }

        bool implOnAccept()
        {
            return true;
        }

        int implGetNative() const
        {
            return mSocketPtr->native();
        }

        void implSetNative(int fd)
        {
            mSocketPtr->assign(
                stream_protocol(),
                fd);
        }

        boost::function0<void> implGetCloseFunctor()
        {
            return boost::bind(
                &UnixLocalSessionState::closeSocket,
                mSocketPtr);
        }

        void implClose()
        {
            mSocketPtr->close();
        }

        ClientTransportAutoPtr implCreateClientTransport()
        {
            int fd = implGetNative();

            std::auto_ptr<UnixLocalClientTransport> unixLocalClientTransport(
                new UnixLocalClientTransport(fd, mRemoteFileName));

            return ClientTransportAutoPtr(unixLocalClientTransport.release());
        }

        void implTransferNativeFrom(I_ClientTransport & clientTransport)
        {
            UnixLocalClientTransport *pUnixLocalClientTransport =
                dynamic_cast<UnixLocalClientTransport *>(&clientTransport);

            if (pUnixLocalClientTransport == NULL)
            {
                RCF_THROW(
                    Exception("incompatible client transport"))
                    (typeid(clientTransport));
            }

            UnixLocalClientTransport & unixLocalClientTransport = *pUnixLocalClientTransport;

            // TODO: exception safety
            mSocketPtr->assign(
                stream_protocol(),
                unixLocalClientTransport.releaseFd());

            mRemoteFileName = unixLocalClientTransport.getPipeName();
        }
        static void closeSocket(UnixLocalSocketPtr socketPtr)
        {
            socketPtr->close();
        }

    private:
        UnixLocalSocketPtr            mSocketPtr;
        std::string                    mRemoteFileName;
    };

    // UnixLocalServerTransport

    std::string UnixLocalServerTransport::getPipeName() const
    {
        return mFileName;
    }

    UnixLocalServerTransport::UnixLocalServerTransport(
        const std::string &fileName) :
            mFileName(fileName)
    {}

    ServerTransportPtr UnixLocalServerTransport::clone()
    {
        return ServerTransportPtr(new UnixLocalServerTransport(mFileName));
    }

    AsioSessionStatePtr UnixLocalServerTransport::implCreateSessionState()
    {
        return AsioSessionStatePtr( new UnixLocalSessionState(*this, mDemuxerPtr) );
    }

    void UnixLocalServerTransport::implOpen()
    {
        if ( !mFileName.empty() )
        {
            boost::shared_ptr<UnixLocalAcceptor> acceptorPtr(
                new UnixLocalAcceptor(*mDemuxerPtr, mFileName));

            mAcceptorPtr = acceptorPtr;
        }
    }

    ClientTransportAutoPtr UnixLocalServerTransport::implCreateClientTransport(
        const I_Endpoint &endpoint)
    {
        const UnixLocalEndpoint & unixLocalEndpoint = 
            dynamic_cast<const UnixLocalEndpoint &>(endpoint);

        ClientTransportAutoPtr clientTransportAutoPtr(
            new UnixLocalClientTransport(unixLocalEndpoint.getPipeName()));

        return clientTransportAutoPtr;
    }

} // namespace RCF
