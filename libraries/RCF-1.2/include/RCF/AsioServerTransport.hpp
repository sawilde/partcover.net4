
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_ASIOSERVERTRANSPORT_HPP
#define INCLUDE_RCF_ASIOSERVERTRANSPORT_HPP

#include <set>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/IpAddress.hpp>
#include <RCF/IpServerTransport.hpp>
#include <RCF/ServerTransport.hpp>
#include <RCF/Service.hpp>
#include <RCF/ThreadLibrary.hpp>

namespace boost {
    namespace asio {
        class io_service;
    }
    namespace system {
        class error_code;
    }
}

namespace RCF {

    class RcfServer;
    class TcpClientTransport;
    class AsioSessionState;
    class AsioAcceptor;
    class AsioDeadlineTimer;
    class AsioServerTransport;

    typedef boost::asio::io_service    AsioDemuxer;

    typedef boost::shared_ptr<AsioDemuxer>              AsioDemuxerPtr;
    typedef boost::shared_ptr<AsioAcceptor>             AsioAcceptorPtr;
    typedef boost::shared_ptr<AsioDeadlineTimer>        AsioDeadlineTimerPtr;
    typedef boost::shared_ptr<AsioSessionState>         AsioSessionStatePtr;
    typedef boost::weak_ptr<AsioSessionState>           AsioSessionStateWeakPtr;

    class AsioAcceptor
    {
    public:
        virtual ~AsioAcceptor()
        {}
    };

    class RCF_EXPORT AsioServerTransport :
        public I_ServerTransport,
        public I_ServerTransportEx,
        public I_Service
    {
    private:

        // Needs to call open().
        friend class TcpAsioTransportFactory;

        typedef boost::weak_ptr<I_Session>              SessionWeakPtr;

        AsioSessionStatePtr createSessionState();
        
        void                notifyClose(
                                AsioSessionStateWeakPtr sessionStateWeakPtr);

        void                closeSessionState(
                                AsioSessionStateWeakPtr sessionStateWeakPtr);

        

        // I_ServerTransportEx implementation
        ClientTransportAutoPtr  
                            createClientTransport(
                                const I_Endpoint &endpoint);

        SessionPtr          createServerSession(
                                ClientTransportAutoPtr clientTransportAutoPtr, 
                                StubEntryPtr stubEntryPtr);

        ClientTransportAutoPtr  
                            createClientTransport(
                                SessionPtr sessionPtr);

        bool                reflect(
                                const SessionPtr &sessionPtr1, 
                                const SessionPtr &sessionPtr2);

        bool                isConnected(const SessionPtr &sessionPtr);

        // I_Service implementation
        void                open();
        void                close();

        bool                cycle(
                                int timeoutMs, 
                                const volatile bool &stopFlag, 
                                bool returnEarly);

        void                stop();
        void                stopCycle(const boost::system::error_code &error);

        void                onServiceAdded(     RcfServer & server);
        void                onServiceRemoved(   RcfServer & server);
        void                onServerOpen(       RcfServer & server);
        void                onServerClose(      RcfServer & server);
        void                onServerStart(      RcfServer & server);
        void                onServerStop(       RcfServer & server);
        void                setServer(          RcfServer & server);

        RcfServer &         getServer();
        I_SessionManager &  getSessionManager();

    private:

        void                registerSession(AsioSessionStateWeakPtr session);
        void                unregisterSession(AsioSessionStateWeakPtr session);
        void                cancelOutstandingIo();

        friend class AsioSessionState;
        friend class FilterAdapter;

    protected:

        AsioServerTransport();
        
        AsioDemuxerPtr                  mDemuxerPtr;
        AsioAcceptorPtr                 mAcceptorPtr;

    private:
        
        AsioDeadlineTimerPtr            mCycleTimerPtr;
        bool                            mInterrupt;
        volatile bool                   mStopFlag;
        RcfServer *                     mpServer;

        Mutex                               mSessionsMutex;
        std::set<AsioSessionStateWeakPtr>   mSessions;

    private:

        virtual AsioSessionStatePtr implCreateSessionState() = 0;
        virtual void implOpen() = 0;
        virtual ClientTransportAutoPtr implCreateClientTransport(
            const I_Endpoint &endpoint) = 0;

    public:

        AsioAcceptorPtr getAcceptorPtr();
    };


    class AsioSessionState :
        public boost::enable_shared_from_this<AsioSessionState>,
        public I_Proactor,
        boost::noncopyable
    {
    public:

        typedef boost::weak_ptr<AsioSessionState>       AsioSessionStateWeakPtr;
        typedef boost::shared_ptr<AsioSessionState>     AsioSessionStatePtr;

        AsioSessionState(
            AsioServerTransport &transport,
            AsioDemuxerPtr demuxerPtr);

        virtual ~AsioSessionState();

        void            setSessionPtr(SessionPtr sessionPtr);
        SessionPtr      getSessionPtr();

        void            close();
        void            invokeAsyncAccept();

        int             getNativeHandle() const;

    protected:

        void            onReadCompletion(
                            boost::system::error_code error, 
                            size_t bytesTransferred);

        void            onWriteCompletion(
                            boost::system::error_code error, 
                            size_t bytesTransferred);
    private:

        void            read(
                            const ByteBuffer &byteBuffer, 
                            std::size_t bytesRequested);

        void            write(
                            const std::vector<ByteBuffer> &byteBuffers);

        

        void            setTransportFilters(
                            const std::vector<FilterPtr> &filters);

        void            getTransportFilters(
                            std::vector<FilterPtr> &filters);

        void            invokeAsyncRead();
        void            invokeAsyncWrite();
        void            onAccept(const boost::system::error_code& error);

        void            onReadWrite(
                            size_t bytesTransferred);

        void            sendServerError(int error);

        void            onReflectedReadWrite(
                            const boost::system::error_code& error, 
                            size_t bytesTransferred);

        std::vector<char> &         getReadBuffer();
        std::vector<char> &         getUniqueReadBuffer();
        ByteBuffer                  getReadByteBuffer() const;

        std::vector<char> &         getReadBufferSecondary();
        std::vector<char> &         getUniqueReadBufferSecondary();
        ByteBuffer                  getReadByteBufferSecondary() const;

        // TODO: too many friends
        friend class    AsioServerTransport;
        friend class    TcpAsioSessionState;
        friend class    UnixLocalSessionState;
        friend class    FilterAdapter;

        enum State
        {
            Ready,
            Accepting,
            ReadingDataCount,
            ReadingData,
            WritingData
        };

        State                       mState;
        std::size_t                 mReadBufferRemaining;
        std::size_t                 mWriteBufferRemaining;
        SessionPtr                  mSessionPtr;
        std::vector<FilterPtr>      mTransportFilters;
        AsioDemuxerPtr              mDemuxerPtr;

        AsioServerTransport &       mTransport;

        std::vector<ByteBuffer>     mWriteByteBuffers;
        std::vector<ByteBuffer>     mSlicedWriteByteBuffers;

        typedef boost::shared_ptr<std::vector<char> > VecPtr;

        VecPtr                      mReadBufferPtr;
        VecPtr                      mReadBufferSecondaryPtr;

        ByteBuffer                  mReadByteBuffer;
        ByteBuffer                  mTempByteBuffer;

        FilterPtr                   mFilterAdapterPtr;

        Mutex                       mMutex;
        bool                        mHasBeenClosed;
        bool                        mCloseAfterWrite;
        AsioSessionStateWeakPtr     mReflecteeWeakPtr;
        AsioSessionStatePtr         mReflecteePtr;
        bool                        mReflecting;

        AsioSessionStateWeakPtr     mWeakThisPtr;

        // I_Proactor

    private:
        
        void                    postRead();
        ByteBuffer              getReadByteBuffer();
        void                    postWrite(std::vector<ByteBuffer> &byteBuffers);
        void                    postClose();
        I_ServerTransport &     getServerTransport();
        const I_RemoteAddress & getRemoteAddress();

    private:

        virtual const I_RemoteAddress & implGetRemoteAddress() = 0;
        virtual void implRead(char * buffer, std::size_t bufferLen) = 0;
        virtual void implWrite(const char * buffer, std::size_t bufferLen) = 0;
        virtual void implWrite(AsioSessionState &toBeNotified, const char * buffer, std::size_t bufferLen) = 0;
        virtual void implAccept() = 0;
        virtual bool implOnAccept() = 0;
        virtual int implGetNative() const = 0;
        virtual boost::function0<void> implGetCloseFunctor() = 0;
        virtual void implClose() = 0;
        virtual void implTransferNativeFrom(I_ClientTransport & clientTransport) = 0;
        virtual ClientTransportAutoPtr implCreateClientTransport() = 0;
    };

    typedef AsioSessionState AsioProactor;

} // namespace RCF


#endif // ! INCLUDE_RCF_ASIOSERVERTRANSPORT_HPP
