
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_CONNECTIONORIENTEDCLIENTTRANSPORT_HPP
#define INCLUDE_RCF_CONNECTIONORIENTEDCLIENTTRANSPORT_HPP

#include <boost/enable_shared_from_this.hpp>

#include <RCF/AmiThreadPool.hpp>
#include <RCF/AsyncFilter.hpp>
#include <RCF/ByteOrdering.hpp>
#include <RCF/ClientProgress.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/Export.hpp>
#include <RCF/RecursionLimiter.hpp>

namespace RCF {

    class ConnectionOrientedClientTransport;

    class ClientFilterProxy;

    class TcpClientTransport;
    typedef boost::shared_ptr<TcpClientTransport> TcpClientTransportPtr;
    class TcpClientFilterProxy;

    class OverlappedAmi;
    typedef boost::shared_ptr<OverlappedAmi> OverlappedAmiPtr;

    class OverlappedAmi : 
        public I_OverlappedAmi,
        public boost::enable_shared_from_this<OverlappedAmi>
    {
    public:
        OverlappedAmi(ConnectionOrientedClientTransport *pTcpClientTransport) : 
            mpTransport(pTcpClientTransport)
        {
        }

        ~OverlappedAmi()
        {
        }

        void onCompletion(std::size_t numBytes);

        void onError(const RCF::Exception & e);

        void onTimerExpired(const TimerEntry & timerEntry);

        Mutex                                   mMutex;
        ConnectionOrientedClientTransport *     mpTransport;
        OverlappedAmiPtr                        mThisPtr;
    };

    void clearSelfReference(OverlappedAmiPtr & overlappedAmiPtr);

    class ClientStubCallbackPtr
    {
    public:
        ClientStubCallbackPtr();

        void            reset(
                            I_ClientTransportCallback *pClientStub = NULL);

        void            onConnectCompleted(
                            bool alreadyConnected = false);

        void            onSendCompleted();
        void            onReceiveCompleted();
        void            onTimerExpired();
        void            onError(const std::exception &e);
        RcfServer *     getAsyncDispatcher();

    private:

        I_ClientTransportCallback * mpClientStub;
        std::size_t                 mCount;
    };


    class RCF_EXPORT ConnectionOrientedClientTransport : 
        public I_ClientTransport, 
        public WithProgressCallback
    {
    public:
        typedef boost::function0<void> CloseFunctor;
        typedef boost::function0<void> NotifyCloseFunctor;

        ConnectionOrientedClientTransport(const ConnectionOrientedClientTransport &rhs);
        ConnectionOrientedClientTransport();
        ~ConnectionOrientedClientTransport();

        void                    setCloseFunctor(const CloseFunctor &closeFunctor);
        void                    setNotifyCloseFunctor(const NotifyCloseFunctor &notifyCloseFunctor);
        void                    close();
        void                    setMaxSendSize(std::size_t maxSendSize);
        std::size_t             getMaxSendSize();

    private:

        void                    read(const ByteBuffer &byteBuffer_, std::size_t bytesRequested);
        void                    write(const std::vector<ByteBuffer> &byteBuffers);
        std::size_t             timedSend(const std::vector<ByteBuffer> &data);
        std::size_t             timedReceive(ByteBuffer &byteBuffer, std::size_t bytesRequested);

        void                    setTransportFilters(const std::vector<FilterPtr> &filters);
        void                    getTransportFilters(std::vector<FilterPtr> &filters);
        void                    connectTransportFilters();
        
        void                    connect(I_ClientTransportCallback &clientStub, unsigned int timeoutMs);
        void                    disconnect(unsigned int timeoutMs);
        int                     timedSend(const char *buffer, std::size_t bufferLen);
        int                     timedReceive(char *buffer, std::size_t bufferLen);

    protected:

        void                    onReadCompleted(const ByteBuffer &byteBuffer);
        void                    onWriteCompleted(std::size_t bytes);

        bool                                    mOwn;
        bool                                    mClosed;
        std::size_t                             mMaxSendSize;
        std::size_t                             mBytesTransferred;
        std::size_t                             mBytesSent;
        std::size_t                             mBytesRead;
        std::size_t                             mBytesTotal;
        int                                     mError;
        unsigned int                            mEndTimeMs;

        CloseFunctor                            mCloseFunctor;
        NotifyCloseFunctor                      mNotifyCloseFunctor;
        std::vector<FilterPtr>                  mTransportFilters;
        std::vector<ByteBuffer>                 mByteBuffers;
        std::vector<ByteBuffer>                 mSlicedByteBuffers;
        boost::shared_ptr<std::vector<char> >   mReadBufferPtr;
        boost::shared_ptr<std::vector<char> >   mReadBuffer2Ptr;

        friend class ClientFilterProxy;

    private:

        virtual std::size_t implRead(
            const ByteBuffer &byteBuffer_,
            std::size_t bytesRequested) = 0;

        virtual std::size_t implReadAsync(
            const ByteBuffer &byteBuffer_,
            std::size_t bytesRequested) = 0;

        virtual std::size_t implWrite(
            const std::vector<ByteBuffer> &byteBuffers) = 0;

        virtual std::size_t implWriteAsync(
            const std::vector<ByteBuffer> &byteBuffers) = 0;

        virtual void implConnect(
            I_ClientTransportCallback &clientStub, 
            unsigned int timeoutMs) = 0;

        virtual void implConnectAsync(
            I_ClientTransportCallback &clientStub, 
            unsigned int timeoutMs) = 0;

        virtual void implClose() = 0;

    public:

        enum State {
            Connecting, 
            Reading, 
            Writing
        };

        bool                        mAsync;
        bool                        mRegisteredForAmi;
        State                       mPreState;
        State                       mPostState;
        std::size_t                 mReadBufferPos;
        std::size_t                 mWriteBufferPos;
        
        ClientStubCallbackPtr       mClientStubCallbackPtr;
        
        ByteBuffer *                mpClientStubReadBuffer;
        ByteBuffer                  mReadBuffer;
        std::size_t                 mBytesToRead;
        std::size_t                 mBytesRequested;
        ByteBuffer                  mByteBuffer;

    private:
        Mutex                       mOverlappedPtrMutex;
        OverlappedAmiPtr            mOverlappedPtr;

    public:

        OverlappedAmiPtr                    getOverlappedPtr();

        typedef boost::shared_ptr<Lock>     LockPtr;

        std::pair<LockPtr,OverlappedAmiPtr> detachOverlappedPtr();

        TimerEntry                          mTimerEntry;

        friend class TcpClientFilterProxy;

    public:
        void setAsync(bool async);
        void cancel();

        TimerEntry  setTimer(
                        boost::uint32_t timeoutMs,
                        I_ClientTransportCallback *pClientStub);

        void        killTimer(
                        const TimerEntry & timerEntry);

        void        onTimerExpired(
                        const TimerEntry & timerEntry);

    private:
        RecursionState<std::size_t, int> mRecursionState;

        // TODO: Access control.
    public:

        void        onTransitionCompleted(std::size_t bytesTransferred);

        void        onCompletion(int bytesTransferred);
        void        onTimedRecvCompleted(int ret, int err);
        void        onTimedSendCompleted(int ret, int err);
        void        onConnectCompleted(int err);

        void        transition();
        void        onTransitionCompleted_(std::size_t bytesTransferred);
        void        issueRead(const ByteBuffer &buffer, std::size_t bytesToRead);
        void        issueWrite(const std::vector<ByteBuffer> &byteBuffers);

        int         send(
                        I_ClientTransportCallback &clientStub, 
                        const std::vector<ByteBuffer> &data, 
                        unsigned int timeoutMs);

        int         receive(
                        I_ClientTransportCallback &clientStub, 
                        ByteBuffer &byteBuffer, 
                        unsigned int timeoutMs);

        friend class OverlappedAmi;

    };

} // namespace RCF

#endif // ! INCLUDE_RCF_CONNECTIONORIENTEDCLIENTTRANSPORT_HPP
