
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/AsioServerTransport.hpp>

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Asio.hpp>
#include <RCF/AsyncFilter.hpp>
#include <RCF/ConnectionOrientedClientTransport.hpp>
#include <RCF/CurrentSession.hpp>
#include <RCF/MethodInvocation.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/TimedBsdSockets.hpp>

namespace RCF {

    class AsioDeadlineTimer
    {
    public:
        AsioDeadlineTimer(AsioDemuxer &demuxer) :
            mImpl(demuxer)
        {}

        boost::asio::deadline_timer mImpl;
    };

    // FilterAdapter

    class FilterAdapter : public RCF::IdentityFilter
    {
    public:
        FilterAdapter(AsioSessionState &sessionState) :
            mSessionState(sessionState)
        {}

    private:
        void read(
            const ByteBuffer &byteBuffer,
            std::size_t bytesRequested)
        {
            mSessionState.read(byteBuffer, bytesRequested);
        }

        void write(
            const std::vector<ByteBuffer> &byteBuffers)
        {
            mSessionState.write(byteBuffers);
        }

        void onReadCompleted(
            const ByteBuffer &byteBuffer)
        {
            mSessionState.onReadWrite(byteBuffer.getLength());
        }

        void onWriteCompleted(
            std::size_t bytesTransferred)
        {
            mSessionState.onReadWrite(bytesTransferred);
        }

        const FilterDescription &getFilterDescription() const
        {
            RCF_ASSERT(0);
            return * (const FilterDescription *) NULL;
        }

        AsioSessionState &mSessionState;
    };

    void AsioSessionState::postRead()
    {
        mState = AsioSessionState::ReadingDataCount;
        getUniqueReadBuffer().resize(4);
        mReadBufferRemaining = 4;
        invokeAsyncRead();
    }

    ByteBuffer AsioSessionState::getReadByteBuffer()
    {
        return ByteBuffer(
            &(*mReadBufferPtr)[0],
            (*mReadBufferPtr).size(),
            mReadBufferPtr);
    }

    void AsioSessionState::postWrite(
        std::vector<ByteBuffer> &byteBuffers)
    {
        RCF_ASSERT(mState == Ready)(mState);

        BOOST_STATIC_ASSERT(sizeof(unsigned int) == 4);

        mSlicedWriteByteBuffers.resize(0);
        mWriteByteBuffers.resize(0);

        std::copy(
            byteBuffers.begin(),
            byteBuffers.end(),
            std::back_inserter(mWriteByteBuffers));

        byteBuffers.resize(0);

        int messageSize = 
            static_cast<int>(RCF::lengthByteBuffers(mWriteByteBuffers));
        
        ByteBuffer &byteBuffer = mWriteByteBuffers.front();

        RCF_ASSERT(
            byteBuffer.getLeftMargin() >= 4)
            (byteBuffer.getLeftMargin());

        byteBuffer.expandIntoLeftMargin(4);
        memcpy(byteBuffer.getPtr(), &messageSize, 4);
        RCF::machineToNetworkOrder(byteBuffer.getPtr(), 4, 1);

        mState = AsioSessionState::WritingData;
        
        mWriteBufferRemaining = RCF::lengthByteBuffers(mWriteByteBuffers);
        
        invokeAsyncWrite();
    }

    void AsioSessionState::postClose()
    {
        close();
    }

    I_ServerTransport & AsioSessionState::getServerTransport()
    {
        return mTransport;
    }

    const I_RemoteAddress &
        AsioSessionState::getRemoteAddress()
    {
        return implGetRemoteAddress();
    }

    int AsioSessionState::getNativeHandle() const
    {
        return implGetNative();
    }

    // SessionState

    AsioSessionState::AsioSessionState(
        AsioServerTransport &transport,
        AsioDemuxerPtr demuxerPtr) :
            mState(Ready),
            mReadBufferRemaining(RCF_DEFAULT_INIT),
            mWriteBufferRemaining(RCF_DEFAULT_INIT),
            mTransport(transport),
            mFilterAdapterPtr(new FilterAdapter(*this)),
            mHasBeenClosed(RCF_DEFAULT_INIT),
            mCloseAfterWrite(RCF_DEFAULT_INIT),
            mReflecting(RCF_DEFAULT_INIT)
    {
    }

    AsioSessionState::~AsioSessionState()
    {
        RCF_DTOR_BEGIN

        // TODO: invoke accept if appropriate
        // TODO: need a proper acceptex strategy in the first place
        //RCF_ASSERT(mState != Accepting);

        mTransport.unregisterSession(mWeakThisPtr);

        // close reflecting session if appropriate
        if (mReflecting)
        {
            AsioSessionStatePtr sessionStatePtr(mReflecteeWeakPtr.lock());
            if (sessionStatePtr)
            {
                sessionStatePtr->close();
            }
        }

        RCF_DTOR_END;
    }

    std::vector<char> &AsioSessionState::getReadBuffer()
    {
        if (!mReadBufferPtr)
        {
            mReadBufferPtr.reset( new std::vector<char>() );
        }
        return *mReadBufferPtr;
    }

    std::vector<char> &
        AsioSessionState::getUniqueReadBuffer()
    {
        if (!mReadBufferPtr || !mReadBufferPtr.unique())
        {
            mReadBufferPtr.reset( new std::vector<char>() );
        }
        return *mReadBufferPtr;
    }

    ByteBuffer AsioSessionState::getReadByteBuffer() const
    {
        return ByteBuffer(
            &(*mReadBufferPtr)[0],
            (*mReadBufferPtr).size(),
            mReadBufferPtr);
    }

    std::vector<char> &
        AsioSessionState::getReadBufferSecondary()
    {
        if (!mReadBufferSecondaryPtr)
        {
            mReadBufferSecondaryPtr.reset( new std::vector<char>() );
        }
        return *mReadBufferSecondaryPtr;
    }

    std::vector<char> &
        AsioSessionState::getUniqueReadBufferSecondary()
    {
        if (!mReadBufferSecondaryPtr || !mReadBufferSecondaryPtr.unique())
        {
            mReadBufferSecondaryPtr.reset( new std::vector<char>() );
        }
        return *mReadBufferSecondaryPtr;
    }

    ByteBuffer 
        AsioSessionState::getReadByteBufferSecondary() const
    {
        return ByteBuffer(
            &(*mReadBufferSecondaryPtr)[0],
            (*mReadBufferSecondaryPtr).size(),
            mReadBufferSecondaryPtr);
    }

    void AsioSessionState::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {

        RCF2_TRACE("")(this);

        if (byteBuffer.getLength() == 0)
        {
            std::vector<char> &vec = getUniqueReadBufferSecondary();
            vec.resize(bytesRequested);
            mTempByteBuffer = getReadByteBufferSecondary();
        }
        else
        {
            mTempByteBuffer = ByteBuffer(byteBuffer, 0, bytesRequested);
        }

        RCF_ASSERT(
            bytesRequested <= mTempByteBuffer.getLength())
            (bytesRequested)(mTempByteBuffer.getLength());

        char *buffer = mTempByteBuffer.getPtr();
        std::size_t bufferLen = mTempByteBuffer.getLength();

        Lock lock(mMutex);
        if (!mHasBeenClosed)
        {
            implRead(buffer, bufferLen);
        }
    }

    void AsioSessionState::write(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        RCF2_TRACE("")(this);

        // TODO: send all the buffers at once
        // ...

        RCF_ASSERT(!byteBuffers.empty());
        char *buffer = byteBuffers.front().getPtr();
        std::size_t bufferLen = byteBuffers.front().getLength();

        Lock lock(mMutex);
        if (!mHasBeenClosed)
        {
            implWrite(buffer, bufferLen);
        }
    }

    // TODO: merge onReadCompletion/onWriteCompletion into one function

    void AsioSessionState::onReadCompletion(
        boost::system::error_code error, size_t bytesTransferred)
    {
        RCF2_TRACE("")(this)(bytesTransferred);

        ThreadTouchGuard threadTouchGuard;

#ifdef BOOST_WINDOWS

        if (error.value() == ERROR_OPERATION_ABORTED)
        {
            error.clear();
        }

#endif

        if (!error && !mTransport.mStopFlag)
        {
            if (mReflecting)
            {
                boost::system::error_code ec;
                onReflectedReadWrite(ec, bytesTransferred);
            }
            else
            {
                SetCurrentSessionGuard guard(mSessionPtr);

                mTempByteBuffer = ByteBuffer(
                    mTempByteBuffer, 
                    0, 
                    bytesTransferred);

                mTransportFilters.empty() ?
                    onReadWrite(bytesTransferred) : 
                    mTransportFilters.back()->onReadCompleted(mTempByteBuffer);
            }
        }
    }

    void AsioSessionState::onWriteCompletion(
        boost::system::error_code error, 
        size_t bytesTransferred)
    {
        RCF2_TRACE("")(this)(bytesTransferred);

        ThreadTouchGuard threadTouchGuard;

#ifdef BOOST_WINDOWS

        if (error.value() == ERROR_OPERATION_ABORTED)
        {
            error.clear();
        }

#endif

        if (!error && !mTransport.mStopFlag)
        {
            if (mReflecting)
            {
                if (mReflecteePtr)
                {
                    mReflecteePtr.reset();
                }
                boost::system::error_code ec;
                onReflectedReadWrite(ec, bytesTransferred);
            }
            else
            {
                SetCurrentSessionGuard guard(mSessionPtr);
                mTransportFilters.empty() ?
                    onReadWrite(bytesTransferred) :
                    mTransportFilters.back()->onWriteCompleted(bytesTransferred);
            }
        }
    }

    void AsioSessionState::setTransportFilters(
        const std::vector<FilterPtr> &filters)
    {

        mTransportFilters.assign(filters.begin(), filters.end());
        connectFilters(mTransportFilters);
        if (!mTransportFilters.empty())
        {
            mTransportFilters.front()->setPreFilter( *mFilterAdapterPtr );
            mTransportFilters.back()->setPostFilter( *mFilterAdapterPtr );
        }
    }

    void AsioSessionState::getTransportFilters(
        std::vector<FilterPtr> &filters)
    {
        filters = mTransportFilters;
    }

    void AsioSessionState::invokeAsyncRead()
    {
        RCF2_TRACE("")(this);

        mReadByteBuffer = ByteBuffer(
            getReadByteBuffer(),
            getReadByteBuffer().getLength()-mReadBufferRemaining);

        mTransportFilters.empty() ?
            read(mReadByteBuffer, mReadBufferRemaining) :
            mTransportFilters.front()->read(mReadByteBuffer, mReadBufferRemaining);
    }

    void AsioSessionState::invokeAsyncWrite()
    {
        RCF2_TRACE("")(this);

        mSlicedWriteByteBuffers.resize(0);

        sliceByteBuffers(
            mSlicedWriteByteBuffers,
            mWriteByteBuffers,
            lengthByteBuffers(mWriteByteBuffers)-mWriteBufferRemaining);

        mTransportFilters.empty() ?
            write(mSlicedWriteByteBuffers) :
            mTransportFilters.front()->write(mSlicedWriteByteBuffers);

    }

    void AsioSessionState::invokeAsyncAccept()
    {
        RCF2_TRACE("")(this);

        mState = Accepting;
        implAccept();
    }

    void AsioSessionState::onAccept(
        const boost::system::error_code& error)
    {
        RCF2_TRACE("")(this);

        if (mTransport.mStopFlag)
        {
            return;
        }

        if (!error)
        {
            // save the remote address in the SessionState object
            bool clientAddrAllowed = implOnAccept();
            mState = WritingData;

            // create a new SessionState, and do an accept on that
            mTransport.createSessionState()->invokeAsyncAccept();

            // set current RCF session
            SetCurrentSessionGuard guard(mSessionPtr);

            if (clientAddrAllowed)
            {
                // Check the connection limit.
                bool allowConnect = true;
                std::size_t connectionLimit = mTransport.getConnectionLimit();
                if (connectionLimit)
                {
                    Lock lock(mTransport.mSessionsMutex);
                    
                    RCF_ASSERT(
                        mTransport.mSessions.size() <= 1 + 1 + connectionLimit);

                    if (mTransport.mSessions.size() == 1 + 1 + connectionLimit)
                    {
                        allowConnect = false;
                    }
                }

                if (allowConnect)
                {
                    // start things rolling by faking a completed write operation
                    onReadWrite(0);
                }
                else
                {
                    sendServerError(RcfError_ConnectionLimitExceeded);
                }
            }
        }
        else if (
            error == boost::asio::error::connection_aborted ||
            error == boost::asio::error::operation_aborted)
        {
            invokeAsyncAccept();
        }
    }

    void onError(
        boost::system::error_code &error1, 
        const boost::system::error_code &error2)
    {
        error1 = error2;
    }

    void AsioSessionState::sendServerError(int error)
    {
        mState = Ready;
        mCloseAfterWrite = true;
        std::vector<ByteBuffer> byteBuffers(1);
        encodeServerError(byteBuffers.front(), error);
        mSessionPtr->getProactor().postWrite(byteBuffers);
    }

    void AsioSessionState::onReadWrite(
        size_t bytesTransferred)
    {
        RCF_ASSERT(!mReflecting);
        {
            switch(mState)
            {
            case ReadingDataCount:
            case ReadingData:

                RCF_ASSERT(
                    bytesTransferred <= mReadBufferRemaining)
                    (bytesTransferred)(mReadBufferRemaining);

                mReadBufferRemaining -= bytesTransferred;
                if (mReadBufferRemaining > 0)
                {
                    invokeAsyncRead();
                }
                else
                {
                    RCF_ASSERT(mReadBufferRemaining == 0)(mReadBufferRemaining);
                    if (mState == ReadingDataCount)
                    {
                        std::vector<char> &readBuffer = getReadBuffer();
                        RCF_ASSERT(readBuffer.size() == 4)(readBuffer.size());

                        unsigned int packetLength = 0;
                        memcpy(&packetLength, &readBuffer[0], 4);
                        networkToMachineOrder(&packetLength, 4, 1);
                        
                        if (    0 < packetLength 
                            &&  packetLength <= mTransport.getMaxMessageLength())
                        {
                            readBuffer.resize(packetLength);
                            mReadBufferRemaining = packetLength;
                            mState = ReadingData;
                            invokeAsyncRead();
                        }
                        else
                        {
                            sendServerError(RcfError_ServerMessageLength);
                        }

                    }
                    else if (mState == ReadingData)
                    {
                        mState = Ready;

                        mTransport.getSessionManager().onReadCompleted(
                            getSessionPtr());

                        if (mTransport.mInterrupt)
                        {
                            mTransport.mInterrupt = false;
                            mTransport.mDemuxerPtr->stop();
                        }
                    }
                }
                break;


            case WritingData:

                RCF_ASSERT(
                    bytesTransferred <= mWriteBufferRemaining)
                    (bytesTransferred)(mWriteBufferRemaining);

                mWriteBufferRemaining -= bytesTransferred;
                if (mWriteBufferRemaining > 0)
                {
                    invokeAsyncWrite();
                }
                else
                {
                    if (mCloseAfterWrite)
                    {
                        int fd = implGetNative();
                        const int BufferSize = 8*1024;
                        char buffer[BufferSize];
                        while (recv(fd, buffer, BufferSize, 0) > 0);
                    }
                    else
                    {
                        mState = Ready;

                        mSlicedWriteByteBuffers.resize(0);
                        mWriteByteBuffers.resize(0);

                        mTransport.getSessionManager().onWriteCompleted(
                            getSessionPtr());

                        if (mTransport.mInterrupt)
                        {
                            mTransport.mInterrupt = false;
                            mTransport.mDemuxerPtr->stop();
                        }
                    }
                }
                break;

            default:
                RCF_ASSERT(0);
            }
        }
    }

    void AsioSessionState::onReflectedReadWrite(
        const boost::system::error_code& error,
        size_t bytesTransferred)
    {
        RCF2_TRACE("")(this);
        RCF_UNUSED_VARIABLE(error);

        RCF_ASSERT(
            mState == ReadingData ||
            mState == ReadingDataCount ||
            mState == WritingData)
            (mState);

        RCF_ASSERT(mReflecting);

        if (bytesTransferred == 0)
        {
            // Previous operation was aborted for some reason (probably because
            // of a thread exiting). Reissue the operation.

            mState = (mState == WritingData) ? ReadingData : WritingData;
        }

        if (mState == WritingData)
        {
            mState = ReadingData;
            std::vector<char> &readBuffer = getReadBuffer();
            readBuffer.resize(8*1024);

            char *buffer = &readBuffer[0];
            std::size_t bufferLen = readBuffer.size();

            Lock lock(mMutex);
            if (!mHasBeenClosed)
            {
                implRead(buffer, bufferLen);
            }
        }
        else if (
            mState == ReadingData ||
            mState == ReadingDataCount)
        {
            mState = WritingData;
            std::vector<char> &readBuffer = getReadBuffer();

            char *buffer = &readBuffer[0];
            std::size_t bufferLen = bytesTransferred;

            // mReflecteePtr will be nulled in onWriteCompletion(), otherwise 
            // we could easily end up with a cycle
            RCF_ASSERT(!mReflecteePtr);
            mReflecteePtr = mReflecteeWeakPtr.lock();
            if (mReflecteePtr)
            {
                RCF_ASSERT(mReflecteePtr->mReflecting);

                Lock lock(mReflecteePtr->mMutex);
                if (!mReflecteePtr->mHasBeenClosed)
                {
                    // TODO: if this can throw, then we need a scope_guard
                    // to reset mReflecteePtr
                    mReflecteePtr->implWrite(*this, buffer, bufferLen);
                }
            }
        }
    }

    // AsioServerTransport

    AsioSessionStatePtr AsioServerTransport::createSessionState()
    {
        RCF2_TRACE("");

        AsioSessionStatePtr sessionStatePtr( implCreateSessionState() );
        SessionPtr sessionPtr( getSessionManager().createSession() );
        sessionPtr->setProactor( *sessionStatePtr );
        sessionStatePtr->setSessionPtr(sessionPtr);
        sessionStatePtr->mWeakThisPtr = sessionStatePtr;
        registerSession(sessionStatePtr->mWeakThisPtr);
        return sessionStatePtr;
    }

    // I_ServerTransportEx implementation

    ClientTransportAutoPtr AsioServerTransport::createClientTransport(
        const I_Endpoint &endpoint)
    {
        RCF2_TRACE("");

        return implCreateClientTransport(endpoint);
    }

    SessionPtr AsioServerTransport::createServerSession(
        ClientTransportAutoPtr clientTransportAutoPtr,
        StubEntryPtr stubEntryPtr)
    {
        RCF2_TRACE("");

        AsioSessionStatePtr sessionStatePtr(createSessionState());
        SessionPtr sessionPtr(sessionStatePtr->getSessionPtr());
        sessionStatePtr->implTransferNativeFrom(*clientTransportAutoPtr);

        if (stubEntryPtr)
        {
            RcfSessionPtr rcfSessionPtr = 
                boost::static_pointer_cast<RcfSession>( 
                sessionStatePtr->mSessionPtr );

            rcfSessionPtr->setDefaultStubEntryPtr(stubEntryPtr);
        }

        sessionStatePtr->mState = AsioSessionState::WritingData;
        sessionStatePtr->onReadWrite(0);
        return sessionPtr;
    }

    ClientTransportAutoPtr AsioServerTransport::createClientTransport(
        SessionPtr sessionPtr)
    {
        RCF2_TRACE("");

        AsioSessionState & sessionState = 
            dynamic_cast<AsioSessionState &>(sessionPtr->getProactor());

        AsioSessionStatePtr sessionStatePtr = sessionState.shared_from_this();

        sessionStatePtr->mDemuxerPtr = mDemuxerPtr;

        ClientTransportAutoPtr clientTransportPtr =
            sessionStatePtr->implCreateClientTransport();

        ConnectionOrientedClientTransport & coClientTransport =
            static_cast<ConnectionOrientedClientTransport &>(
                *clientTransportPtr);

        coClientTransport.setNotifyCloseFunctor( boost::bind(
            &AsioServerTransport::notifyClose,
            this,
            AsioSessionStateWeakPtr(sessionStatePtr)));

        coClientTransport.setCloseFunctor( 
            sessionStatePtr->implGetCloseFunctor() );

        return clientTransportPtr;
    }

    bool AsioServerTransport::reflect(
        const SessionPtr &sessionPtr1, 
        const SessionPtr &sessionPtr2)
    {
        RCF2_TRACE("");

        AsioSessionState & sessionState1 = 
            dynamic_cast<AsioSessionState &>(sessionPtr1->getProactor());

        AsioSessionStatePtr sessionStatePtr1 = sessionState1.shared_from_this();

        AsioSessionState & sessionState2 = 
            dynamic_cast<AsioSessionState &>(sessionPtr2->getProactor());

        AsioSessionStatePtr sessionStatePtr2 = sessionState2.shared_from_this();

        sessionStatePtr1->mReflecteeWeakPtr = sessionStatePtr2;
        sessionStatePtr2->mReflecteeWeakPtr = sessionStatePtr1;

        sessionStatePtr1->mReflecting = true;
        sessionStatePtr2->mReflecting = true;

        return true;
    }

    bool AsioServerTransport::isConnected(const SessionPtr &sessionPtr)
    {
        AsioSessionState & sessionState = 
            dynamic_cast<AsioSessionState &>(sessionPtr->getProactor());

        AsioSessionStatePtr sessionStatePtr = sessionState.shared_from_this();

        // TODO: what to do for non-TCP sockets
        return 
            sessionStatePtr.get() 
            && isFdConnected(sessionStatePtr->implGetNative());
    }

    // I_Service implementation

    void AsioServerTransport::open()
    {
        RCF2_TRACE("");

        if (!mDemuxerPtr)
        {
            mInterrupt = false;
            mStopFlag = false;
            mDemuxerPtr.reset( new AsioDemuxer() );
            mCycleTimerPtr.reset( new AsioDeadlineTimer(*mDemuxerPtr) );

            implOpen();
        }
    }

    void AsioSessionState::setSessionPtr(
        SessionPtr sessionPtr)    
    { 
        mSessionPtr = sessionPtr; 
    }
    
    SessionPtr AsioSessionState::getSessionPtr()
    { 
        return mSessionPtr; 
    }

    void AsioSessionState::close()
    {
        RCF2_TRACE("")(this);

        Lock lock(mMutex);
        if (!mHasBeenClosed)
        {
            implClose();
            mDemuxerPtr.reset();
            mHasBeenClosed = true;
        }
    }

    void AsioServerTransport::notifyClose(
        AsioSessionStateWeakPtr sessionStateWeakPtr)
    {
        AsioSessionStatePtr sessionStatePtr(sessionStateWeakPtr.lock());
        if (sessionStatePtr)
        {
            Lock lock(sessionStatePtr->mMutex);
            sessionStatePtr->mHasBeenClosed = true;
        }
    }

    void AsioServerTransport::close()
    {
        RCF2_TRACE("");

        mAcceptorPtr.reset();
        mCycleTimerPtr.reset();

        mStopFlag = true;
        cancelOutstandingIo();

        mDemuxerPtr->reset();
        std::size_t items = mDemuxerPtr->run();
        while (items)
        {
            mDemuxerPtr->reset();
            items = mDemuxerPtr->run();
        }

        mDemuxerPtr.reset();
    }

    bool AsioServerTransport::cycle(
        int timeoutMs,
        const volatile bool &,// stopFlag
        bool returnEarly)
    {
        RCF2_TRACE("Entering cycle()");

        RCF_ASSERT(timeoutMs >= -1)(timeoutMs);

        mInterrupt = returnEarly;
        if (timeoutMs != -1)
        {
            mCycleTimerPtr->mImpl.cancel();

            mCycleTimerPtr->mImpl.expires_from_now(
                boost::posix_time::milliseconds(timeoutMs));

            mCycleTimerPtr->mImpl.async_wait( boost::bind(
                &AsioServerTransport::stopCycle, 
                this, 
                boost::asio::placeholders::error));
        }

        mDemuxerPtr->reset();
        mDemuxerPtr->run();

        RCF2_TRACE("Exiting cycle()");

        return false;
    }

    void AsioServerTransport::stopCycle(
        const boost::system::error_code &error)
    {
        RCF2_TRACE("");

        if (!error)
        {
            mDemuxerPtr->stop();
        }
    }

    void AsioServerTransport::stop()
    {
        mDemuxerPtr->stop();
    }

    void AsioServerTransport::onServiceAdded(RcfServer &server)
    {
        setServer(server);
        WriteLock writeLock( getTaskEntriesMutex() );
        getTaskEntries().clear();
        getTaskEntries().push_back(
            TaskEntry(
            boost::bind(&AsioServerTransport::cycle, this, _1, _2, _3),
            boost::bind(&AsioServerTransport::stop, this),
            "RCF Asio server"));
    }

    void AsioServerTransport::onServiceRemoved(RcfServer &)
    {}

    void AsioServerTransport::onServerOpen(RcfServer &)
    {
        open();
    }

    void AsioServerTransport::onServerClose(RcfServer &)
    {
        close();
    }

    void AsioServerTransport::onServerStart(RcfServer &)
    {
        mStopFlag = false;
        if (mAcceptorPtr)
        {
            createSessionState()->invokeAsyncAccept();
        }
    }

    void AsioServerTransport::onServerStop(RcfServer &)
    {
    }

    void AsioServerTransport::setServer(RcfServer &server)
    {
        mpServer = &server;
    }

    RcfServer &AsioServerTransport::getServer()
    {
        return *mpServer;
    }

    I_SessionManager &AsioServerTransport::getSessionManager()
    {
        return *mpServer;
    }

    AsioServerTransport::AsioServerTransport() :
        mDemuxerPtr(),
        mAcceptorPtr(),
        mCycleTimerPtr(),
        mInterrupt(RCF_DEFAULT_INIT),
        mStopFlag(RCF_DEFAULT_INIT),
        mpServer(RCF_DEFAULT_INIT)
    {
    }

    void AsioServerTransport::registerSession(AsioSessionStateWeakPtr session)
    {
        Lock lock(mSessionsMutex);
        mSessions.insert(session);
    }

    void AsioServerTransport::unregisterSession(AsioSessionStateWeakPtr session)
    {
        Lock lock(mSessionsMutex);
        mSessions.erase(session);
    }

    void AsioServerTransport::cancelOutstandingIo()
    {
        Lock lock(mSessionsMutex);
        std::for_each( 
            mSessions.begin(), 
            mSessions.end(), 
            boost::bind(&AsioServerTransport::closeSessionState, this, _1));
    }

    void AsioServerTransport::closeSessionState(
        AsioSessionStateWeakPtr sessionStateWeakPtr)
    {
        AsioSessionStatePtr sessionStatePtr(sessionStateWeakPtr.lock());
        if (sessionStatePtr)
        {
            sessionStatePtr->close();
        }
    }

    AsioAcceptorPtr AsioServerTransport::getAcceptorPtr()
    {
        return mAcceptorPtr;
    }

} // namespace RCF
