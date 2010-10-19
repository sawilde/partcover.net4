
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/ConnectionOrientedClientTransport.hpp>

#include <RCF/Exception.hpp>

namespace RCF {

    class ClientFilterProxy : public Filter, boost::noncopyable
    {
    public:
        ClientFilterProxy(
            ConnectionOrientedClientTransport & transport, 
            Filter & filter, 
            bool top) :
                mTransport(transport),
                mFilter(filter),
                mTop(top)
        {}

    private:

        void read(const ByteBuffer &byteBuffer, std::size_t bytesRequested)
        {
            mTop ?
                mFilter.read(byteBuffer, bytesRequested) :
                mTransport.read(byteBuffer, bytesRequested);
        }

        void write(const std::vector<ByteBuffer> &byteBuffers)
        {
            mTop ?
                mFilter.write(byteBuffers) :
                mTransport.write(byteBuffers);
        }

        void onReadCompleted(const ByteBuffer &byteBuffer)
        {
            mTop ?
                mTransport.onReadCompleted(byteBuffer) :
                mFilter.onReadCompleted(byteBuffer);
        }

        void onWriteCompleted(std::size_t bytesTransferred)
        {
            mTop ?
                mTransport.onWriteCompleted(bytesTransferred) :
                mFilter.onWriteCompleted(bytesTransferred);
        }

        const FilterDescription &getFilterDescription() const
        {
            RCF_ASSERT(0);
            return * (const FilterDescription *) NULL;
        }

        ConnectionOrientedClientTransport &mTransport;
        Filter &mFilter;
        bool mTop;
    };

    void OverlappedAmi::onCompletion(std::size_t numBytes)
    {
        RCF_ASSERT(mThisPtr.get() || !mpTransport);
        OverlappedAmiPtr overlappedAmiPtr = mThisPtr;
        mThisPtr.reset();
        RCF_ASSERT(!mThisPtr);

        Lock lock(mMutex);
        if (mpTransport)
        {
            try
            {
                mpTransport->onCompletion( static_cast<int>(numBytes) );
            }
            catch(const std::exception &e)
            {
                mpTransport->mClientStubCallbackPtr.onError(e);
            }
        }
    }

    void OverlappedAmi::onError(const RCF::Exception & eRcf)
    {
        RCF_ASSERT(mThisPtr);
        OverlappedAmiPtr overlappedAmiPtr = mThisPtr;
        mThisPtr.reset();
        RCF_ASSERT(!mThisPtr);

        Lock lock(mMutex);
        if (mpTransport)
        {
            try
            {
                mpTransport->mClientStubCallbackPtr.onError(eRcf);
            }
            catch(const std::exception &e)
            {
                mpTransport->mClientStubCallbackPtr.onError(e);
            }
        }
    }

    void OverlappedAmi::onTimerExpired(const TimerEntry & timerEntry)
    {
        RCF_UNUSED_VARIABLE(timerEntry);

        Lock lock(mMutex);

        if (mpTransport)
        {
            mpTransport->onTimerExpired(timerEntry);
        }
    }

    void ConnectionOrientedClientTransport::onCompletion(int bytesTransferred)
    {
        switch (mPostState)
        {
        case Connecting:
            onConnectCompleted(0);
            break;

        case Reading:
            onTimedRecvCompleted(bytesTransferred, 0);
            break;

        case Writing:
            onTimedSendCompleted(bytesTransferred, 0);
            break;

        default:
            RCF_ASSERT(0);
        }
    }

    void clearSelfReference(OverlappedAmiPtr & overlappedAmiPtr)
    {
        overlappedAmiPtr.reset();
    }

#ifdef _MSC_VER
#pragma warning( push )
// warning C4355: 'this' : used in base member initializer list
#pragma warning( disable : 4355 ) 
#endif


    ConnectionOrientedClientTransport::ConnectionOrientedClientTransport() :
        mOwn(true),
        mClosed(true),
        mMaxSendSize(1024*1024*10),
        mBytesTransferred(RCF_DEFAULT_INIT),
        mBytesSent(RCF_DEFAULT_INIT),
        mBytesRead(RCF_DEFAULT_INIT),
        mBytesTotal(RCF_DEFAULT_INIT),
        mError(RCF_DEFAULT_INIT),
        mEndTimeMs(RCF_DEFAULT_INIT),

        mAsync(RCF_DEFAULT_INIT),
        mRegisteredForAmi(RCF_DEFAULT_INIT),
        mPreState(Connecting),
        mPostState(Connecting),
        mReadBufferPos(RCF_DEFAULT_INIT),
        mWriteBufferPos(RCF_DEFAULT_INIT),
        mpClientStubReadBuffer(RCF_DEFAULT_INIT),
        mBytesToRead(RCF_DEFAULT_INIT),
        mBytesRequested(RCF_DEFAULT_INIT),
        mByteBuffer(),
        mOverlappedPtr( new OverlappedAmi(this) )
    {
        setTransportFilters( std::vector<FilterPtr>() );
    }

    ConnectionOrientedClientTransport::ConnectionOrientedClientTransport(
        const ConnectionOrientedClientTransport &rhs) :
            I_ClientTransport(rhs),        
            mOwn(true),
            mClosed(true),
            mMaxSendSize(1024*1024*10),
            mBytesTransferred(RCF_DEFAULT_INIT),
            mBytesSent(RCF_DEFAULT_INIT),
            mBytesRead(RCF_DEFAULT_INIT),
            mBytesTotal(RCF_DEFAULT_INIT),
            mError(RCF_DEFAULT_INIT),
            mEndTimeMs(RCF_DEFAULT_INIT),

            mAsync(RCF_DEFAULT_INIT),
            mRegisteredForAmi(RCF_DEFAULT_INIT),
            mPreState(Connecting),
            mPostState(Connecting),
            mReadBufferPos(RCF_DEFAULT_INIT),
            mWriteBufferPos(RCF_DEFAULT_INIT),
            mpClientStubReadBuffer(RCF_DEFAULT_INIT),
            mBytesToRead(RCF_DEFAULT_INIT),
            mBytesRequested(RCF_DEFAULT_INIT),
            mByteBuffer(),
            mOverlappedPtr( new OverlappedAmi(this) )
    {
        setTransportFilters( std::vector<FilterPtr>() );
    }

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    ConnectionOrientedClientTransport::~ConnectionOrientedClientTransport()
    {
        RCF_DTOR_BEGIN
            
            RCF_ASSERT(mClosed);

        RCF_DTOR_END
    }

    void ConnectionOrientedClientTransport::connect(
        I_ClientTransportCallback &clientStub, 
        unsigned int timeoutMs)
    {
        if (!isConnected())
        {
            mClosed = false;

            mPostState = Connecting;

            if (mAsync)
            {
                implConnectAsync(clientStub, timeoutMs);
            }
            else
            {
                implConnect(clientStub, timeoutMs);
            }
        }
    }

    void ConnectionOrientedClientTransport::onConnectCompleted(int err)
    {
        RCF_UNUSED_VARIABLE(err);
        mClientStubCallbackPtr.onConnectCompleted();
    }

    void ConnectionOrientedClientTransport::disconnect(unsigned int timeoutMs)
    {
        RCF_UNUSED_VARIABLE(timeoutMs);

        // close the connection
        close();
    }

    void ConnectionOrientedClientTransport::read(
        const ByteBuffer &byteBuffer_, 
        std::size_t bytesRequested)
    {
        mBytesRequested = bytesRequested;

        if (byteBuffer_.getLength() == 0)
        {
            if (mReadBuffer2Ptr.get() == NULL || !mReadBuffer2Ptr.unique())
            {
                mReadBuffer2Ptr.reset( new std::vector<char>() );
            }
            mReadBuffer2Ptr->resize(bytesRequested);
            mByteBuffer = ByteBuffer(mReadBuffer2Ptr);
        }
        else
        {
            mByteBuffer = byteBuffer_;
        }

        mBytesToRead = RCF_MIN(bytesRequested, mByteBuffer.getLength());
        std::size_t & bytesToRead = mBytesToRead;

        mPostState = Reading;

        if (!mAsync)
        {
            std::size_t bytesRead = implRead(mByteBuffer, bytesToRead);
            RCF_UNUSED_VARIABLE(bytesRead);
        }
        else
        {
            std::size_t bytesRead = implReadAsync(mByteBuffer, bytesToRead);            
            RCF_UNUSED_VARIABLE(bytesRead);
        }
    }

    void ConnectionOrientedClientTransport::onTimedRecvCompleted(int ret, int err)
    {
        switch (ret)
        {
        case -2:
            RCF_THROW(
                Exception(_RcfError_ClientReadTimeout()))
                (mBytesToRead);
            break;

        case -1:
            RCF_THROW(
                Exception(
                _RcfError_ClientReadFail(),
                err,
                RcfSubsystem_Os))
                (mBytesToRead)(err);
            break;

        case  0:
            RCF_THROW(
                Exception(_RcfError_PeerDisconnect()))
                (mBytesToRead);
            break;

        default:
            RCF_ASSERT(
                0 < ret && ret <= static_cast<int>(mBytesRequested))
                (ret)(mBytesRequested);

            ByteBuffer b(mByteBuffer.release(), 0, ret);
            mTransportFilters.empty() ?
                onReadCompleted(b) :
                mTransportFilters.back()->onReadCompleted(b);
        }

    }

    void ConnectionOrientedClientTransport::onTimedSendCompleted(int ret, int err)
    {
        RCF_UNUSED_VARIABLE(err);
        mTransportFilters.empty() ?
            onWriteCompleted(ret) :
            mTransportFilters.back()->onWriteCompleted(ret);
    }

    void ConnectionOrientedClientTransport::write(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        mPostState = Writing;

        if (!mAsync)
        {
            std::size_t bytesWritten = implWrite(byteBuffers);
            RCF_UNUSED_VARIABLE(bytesWritten);
        }
        else
        {
            std::size_t bytesWritten = implWriteAsync(byteBuffers);
            RCF_UNUSED_VARIABLE(bytesWritten);
        }
    }

    // helper to disambiguate std::vector<ByteBuffer>::resize()
    typedef void (std::vector<ByteBuffer>::*PfnResize)(std::vector<ByteBuffer>::size_type);

    void clearByteBuffers( std::vector<ByteBuffer> &byteBuffers)
    {
        byteBuffers.resize(0);
    }

    void ConnectionOrientedClientTransport::onReadCompleted(
        const ByteBuffer &byteBuffer)
    {
        onTransitionCompleted(byteBuffer.getLength());
    }

    void ConnectionOrientedClientTransport::onWriteCompleted(
        std::size_t bytesTransferred)
    {
        onTransitionCompleted(bytesTransferred);
    }


    int ConnectionOrientedClientTransport::send(
        I_ClientTransportCallback &clientStub, 
        const std::vector<ByteBuffer> &data,
        unsigned int totalTimeoutMs)
    {

        RCF3_TRACE("")(lengthByteBuffers(data))(totalTimeoutMs);

        unsigned int startTimeMs = getCurrentTimeMs();
        mEndTimeMs = startTimeMs + totalTimeoutMs;

        mByteBuffers.resize(0);
        std::copy(data.begin(), data.end(), std::back_inserter(mByteBuffers));

        mClientStubCallbackPtr.reset(&clientStub);

        mPreState = Writing;
        mWriteBufferPos = 0;
        transition();
        return 1;
    }

    std::size_t ConnectionOrientedClientTransport::timedSend(const std::vector<ByteBuffer> &data)
    {
        std::size_t bytesRequested = lengthByteBuffers(data);
        std::size_t bytesToWrite = bytesRequested;
        std::size_t bytesWritten = 0;

        using namespace boost::multi_index::detail;
        scope_guard resizeGuard =
            make_guard(clearByteBuffers, boost::ref(mSlicedByteBuffers));
        RCF_UNUSED_VARIABLE(resizeGuard);

        while (true)
        {
            sliceByteBuffers(mSlicedByteBuffers, data, bytesWritten);

            mTransportFilters.empty() ?
                write(mSlicedByteBuffers):
                mTransportFilters.front()->write(mSlicedByteBuffers);

            RCF_ASSERT(
                0 < mBytesTransferred &&
                mBytesTransferred <= lengthByteBuffers(mSlicedByteBuffers))
                (mBytesTransferred)(lengthByteBuffers(mSlicedByteBuffers));

            bytesToWrite -= mBytesTransferred;
            bytesWritten += mBytesTransferred;

            if (
                mClientProgressPtr.get() &&
                (mClientProgressPtr->mTriggerMask & ClientProgress::Event))
            {
                ClientProgress::Action action = ClientProgress::Continue;

                mClientProgressPtr->mProgressCallback(
                    bytesWritten, bytesRequested,
                    ClientProgress::Event,
                    ClientProgress::Send,
                    action);

                RCF_VERIFY(
                    action != ClientProgress::Cancel,
                    Exception(_RcfError_ClientCancel()))
                    (mBytesSent)(mBytesTotal);
            }

            if (bytesToWrite == 0)
            {
                return bytesWritten;
            }
        }
    }

    // return bufferLen
    std::size_t ConnectionOrientedClientTransport::timedReceive(
        ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        std::size_t bytesToRead = bytesRequested;
        std::size_t bytesRead = 0;
        while (true)
        {
            ByteBuffer buffer(byteBuffer, bytesRead, bytesToRead);

            mTransportFilters.empty() ?
                read(buffer, bytesToRead):
                mTransportFilters.front()->read(buffer, bytesToRead);

            RCF_ASSERT(
                0 < mBytesTransferred && mBytesTransferred <= bytesToRead)
                (mBytesTransferred)(bytesRead);

            bytesToRead -= mBytesTransferred;
            bytesRead += mBytesTransferred;

            if (
                mClientProgressPtr.get() &&
                (mClientProgressPtr->mTriggerMask & ClientProgress::Event))
            {
                ClientProgress::Action action = ClientProgress::Continue;

                mClientProgressPtr->mProgressCallback(
                    bytesRead,
                    bytesRequested,
                    ClientProgress::Event,
                    ClientProgress::Receive,
                    action);

                RCF_VERIFY(
                    action != ClientProgress::Cancel,
                    Exception(_RcfError_ClientCancel()))
                    (mBytesRead)(mBytesTotal);
            }

            if (bytesToRead == 0)
            {
                return bytesRead;
            }

        }
    }

    int ConnectionOrientedClientTransport::receive(
        I_ClientTransportCallback &clientStub, 
        ByteBuffer &byteBuffer,
        unsigned int timeoutMs)
    {
        mEndTimeMs = getCurrentTimeMs() + timeoutMs;

        mPreState = Reading;
        mReadBufferPos = 0;
        
        mClientStubCallbackPtr.reset(&clientStub);

        mpClientStubReadBuffer = &byteBuffer;
        transition();

        return 1;
    }

    void ConnectionOrientedClientTransport::setCloseFunctor(
        const CloseFunctor &closeFunctor)
    {
        mCloseFunctor = closeFunctor;
    }

    void ConnectionOrientedClientTransport::setNotifyCloseFunctor(
        const NotifyCloseFunctor &notifyCloseFunctor)
    {
        mNotifyCloseFunctor = notifyCloseFunctor;
    }

    void ConnectionOrientedClientTransport::close()
    {
        std::pair< 
            boost::shared_ptr<Lock>,
            OverlappedAmiPtr> overlappedNew = detachOverlappedPtr();

        OverlappedAmiPtr overlappedPtr = overlappedNew.second;
        RCF_ASSERT(overlappedPtr->mpTransport);
            
        if (!mClosed)
        {
            mClientStubCallbackPtr.reset();

            // First notify anyone who is interested.
            if (mNotifyCloseFunctor)
            {
                mNotifyCloseFunctor();
                mNotifyCloseFunctor = NotifyCloseFunctor();
            }

            // And then figure out how to close the socket.
            if (mCloseFunctor)
            {
                // Actually just a workaround for TcpAsioServerTransport.
                mCloseFunctor();
                mCloseFunctor = CloseFunctor();
            }
            else
            {
                implClose();
            }

            mRegisteredForAmi = false;
            mClosed = true;
        }
    }

    void ConnectionOrientedClientTransport::setTransportFilters(
        const std::vector<FilterPtr> &filters)
    {
        mTransportFilters.clear();
        if (!filters.empty())
        {
            mTransportFilters.push_back(
                FilterPtr(new ClientFilterProxy(*this, *filters.front(), true)));

            std::copy(
                filters.begin(),
                filters.end(),
                std::back_inserter(mTransportFilters));

            mTransportFilters.push_back(
                FilterPtr(new ClientFilterProxy(*this, *filters.back(), false)));

            RCF::connectFilters(mTransportFilters);

        }
    }

    void ConnectionOrientedClientTransport::getTransportFilters(
        std::vector<FilterPtr> &filters)
    {
        // TODO: keep the adapter filters out of mTransportFilters?
        if (mTransportFilters.empty())
        {
            filters.resize(0);
        }
        else
        {
            RCF_ASSERT(mTransportFilters.size() >= 3)(mTransportFilters.size());

            //filters.assign(
            //    ++mTransportFilters.begin(),
            //    --mTransportFilters.end());

            // for borland
            std::vector<FilterPtr>::const_iterator iter0(mTransportFilters.begin());
            std::vector<FilterPtr>::const_iterator iter1(mTransportFilters.end());
            ++iter0;
            --iter1;
            filters.assign(iter0, iter1);

        }
    }

    void ConnectionOrientedClientTransport::setMaxSendSize(std::size_t maxSendSize)
    {
        mMaxSendSize = maxSendSize;
    }

    std::size_t ConnectionOrientedClientTransport::getMaxSendSize()
    {
        return mMaxSendSize;
    }

    void ConnectionOrientedClientTransport::issueRead(
        const ByteBuffer &buffer, 
        std::size_t bytesToRead)
    {
        mTransportFilters.empty() ?
            read(buffer, bytesToRead):
            mTransportFilters.front()->read(buffer, bytesToRead);
    }

    void ConnectionOrientedClientTransport::issueWrite(const std::vector<ByteBuffer> &byteBuffers)
    {
        mTransportFilters.empty() ?
            write(byteBuffers):
            mTransportFilters.front()->write(byteBuffers);
    }

    void ConnectionOrientedClientTransport::transition()
    {
        switch (mPreState)
        {
        case Reading:
            if (mReadBufferPos == 0)
            {
                if (mReadBufferPtr.get() == NULL || !mReadBufferPtr.unique())
                {
                    mReadBufferPtr.reset( new std::vector<char>() );
                }
                mReadBufferPtr->resize(4);
                mReadBuffer = ByteBuffer(mReadBufferPtr);

                issueRead(ByteBuffer(mReadBuffer, mReadBufferPos), 4-mReadBufferPos);
            }
            else if (mReadBufferPos < 4)
            {
                issueRead(ByteBuffer(mReadBuffer, mReadBufferPos), 4-mReadBufferPos);
            }
            else if (mReadBufferPos == 4)
            {
                unsigned int length = *(unsigned int*)mReadBuffer.getPtr();
                networkToMachineOrder(&length, sizeof(length), 1);

                RCF_VERIFY(
                    0 < length && length <= getMaxMessageLength(),
                    Exception(_RcfError_ClientMessageLength()))
                    (length)(getMaxMessageLength());

                mReadBufferPtr->resize(4+length);
                mReadBuffer = ByteBuffer(mReadBufferPtr);

                issueRead(ByteBuffer(mReadBuffer, 4), length);
            }
            else if (mReadBufferPos < mReadBuffer.getLength())
            {
                issueRead(ByteBuffer(mReadBuffer, mReadBufferPos), mReadBuffer.getLength()-mReadBufferPos);

            }
            else if (mReadBufferPos == mReadBuffer.getLength())
            {
                *mpClientStubReadBuffer = ByteBuffer(mReadBuffer, 4);
                mReadBuffer.clear();
                mClientStubCallbackPtr.onReceiveCompleted();
            }
            else
            {
                RCF_ASSERT(0);
            }

            break;

        case Writing:

            if (mWriteBufferPos < lengthByteBuffers(mByteBuffers))
            {
                sliceByteBuffers(mSlicedByteBuffers, mByteBuffers, mWriteBufferPos);

                issueWrite(mSlicedByteBuffers);
            }
            else 
            {
                RCF_ASSERT( mWriteBufferPos == lengthByteBuffers(mByteBuffers));
                mByteBuffers.resize(0);
                mSlicedByteBuffers.resize(0);
                mClientStubCallbackPtr.onSendCompleted();
            }

            break;

        default:

            RCF_ASSERT(0);

        }
    }

    void ConnectionOrientedClientTransport::setAsync(bool async) 
    { 
        mAsync = async; 
    }

    void ConnectionOrientedClientTransport::cancel()
    {
        OverlappedAmiPtr overlappedPtr = getOverlappedPtr();
        Lock lock(overlappedPtr->mMutex);
        if (overlappedPtr->mpTransport)
        {
            RCF_ASSERT(overlappedPtr->mpTransport == this);
            RCF::Exception e(( _RcfError_ClientCancel() ));
            mClientStubCallbackPtr.onError(e);
        }
    }

#ifdef BOOST_WINDOWS

    ConnectionOrientedClientTransport::TimerEntry 
    ConnectionOrientedClientTransport::setTimer(
        boost::uint32_t timeoutMs,
        I_ClientTransportCallback *pClientStub)
    {
        if (pClientStub)
        {
            mClientStubCallbackPtr.reset(pClientStub);
        }

        OverlappedAmiPtr overlappedPtr = getOverlappedPtr();
        mTimerEntry = gAmiThreadPoolPtr->addTimerEntry(overlappedPtr, timeoutMs);
        return mTimerEntry;
    }

    void ConnectionOrientedClientTransport::killTimer(
        const TimerEntry & timerEntry)
    {
        RCF_ASSERT(mTimerEntry == timerEntry);
        gAmiThreadPoolPtr->removeTimerEntry(timerEntry);
    }

    void ConnectionOrientedClientTransport::onTimerExpired(
        const TimerEntry & timerEntry)
    {
        if (timerEntry == mTimerEntry)
        {
            try
            {                
                mClientStubCallbackPtr.onTimerExpired();
            }
            catch(const std::exception &e)
            {
                mClientStubCallbackPtr.onError(e);
            }
        }
    }

#else

    // TODO: implement for non-Windows platforms.

    ConnectionOrientedClientTransport::TimerEntry 
    ConnectionOrientedClientTransport::setTimer(
        boost::uint32_t timeoutMs,
        I_ClientTransportCallback *pClientStub)
    {
        return TimerEntry();
    }

    void ConnectionOrientedClientTransport::killTimer(
        const TimerEntry & timerEntry)
    {
    }

    void ConnectionOrientedClientTransport::onTimerExpired(
        const TimerEntry & timerEntry)
    {
    }

#endif

    void ConnectionOrientedClientTransport::onTransitionCompleted(
        std::size_t bytesTransferred)
    {
        if (mAsync)
        {
            onTransitionCompleted_(bytesTransferred);
        }
        else
        {
            applyRecursionLimiter(
                mRecursionState,
                &ConnectionOrientedClientTransport::onTransitionCompleted_,
                *this,
                bytesTransferred);
        }
    }

    void ConnectionOrientedClientTransport::onTransitionCompleted_(std::size_t bytesTransferred)
    {
        if (mPreState == Reading)
        {
            mReadBufferPos += bytesTransferred;
        }
        else if (mPreState == Writing)
        {
            mWriteBufferPos += bytesTransferred;
        }

        if (
            mClientProgressPtr.get() &&
            (mClientProgressPtr->mTriggerMask & ClientProgress::Event))
        {
            ClientProgress::Action action = ClientProgress::Continue;

            if (mPreState == Reading)
            {
                mClientProgressPtr->mProgressCallback(
                    mReadBufferPos, 
                    mReadBuffer.getLength(),
                    ClientProgress::Event,
                    ClientProgress::Receive,
                    action);

                RCF_VERIFY(
                    action != ClientProgress::Cancel,
                    Exception(_RcfError_ClientCancel()))
                    (mReadBufferPos)(mReadBuffer.getLength());
            }
            else if (mPreState == Writing)
            {
                mClientProgressPtr->mProgressCallback(
                    mWriteBufferPos, 
                    lengthByteBuffers(mByteBuffers),
                    ClientProgress::Event,
                    ClientProgress::Send,
                    action);

                RCF_VERIFY(
                    action != ClientProgress::Cancel,
                    Exception(_RcfError_ClientCancel()))
                    (mWriteBufferPos)(lengthByteBuffers(mByteBuffers));
            }

        }

        transition();
    }

    ClientStubCallbackPtr::ClientStubCallbackPtr() : 
        mpClientStub(RCF_DEFAULT_INIT),
        mCount(RCF_DEFAULT_INIT)
    {}

    void ClientStubCallbackPtr::reset(
        I_ClientTransportCallback *pClientStub)
    {
        mpClientStub = pClientStub;
        mCount = 0;
    }

    void ClientStubCallbackPtr::onConnectCompleted(
        bool alreadyConnected)
    {
        RCF_ASSERT(mpClientStub);
        RCF_ASSERT(mCount == 0);
        ++mCount;
        mpClientStub->onConnectCompleted(alreadyConnected);
    }

    void ClientStubCallbackPtr::onSendCompleted()
    {
        RCF_ASSERT(mpClientStub);
        RCF_ASSERT(mCount == 0);
        ++mCount;
        mpClientStub->onSendCompleted();
    }

    void ClientStubCallbackPtr::onReceiveCompleted()
    {
        RCF_ASSERT(mpClientStub);
        RCF_ASSERT(mCount == 0);
        ++mCount;
        mpClientStub->onReceiveCompleted();
    }

    void ClientStubCallbackPtr::onTimerExpired()
    {
        RCF_ASSERT(mpClientStub);
        ++mCount;
        mpClientStub->onTimerExpired();
    }

    void ClientStubCallbackPtr::onError(const std::exception &e)
    {
        RCF_ASSERT(mpClientStub);
        mpClientStub->onError(e);
    }

    RcfServer * ClientStubCallbackPtr::getAsyncDispatcher()
    {
        return mpClientStub->getAsyncDispatcher();
    }

    OverlappedAmiPtr ConnectionOrientedClientTransport::getOverlappedPtr()
    {
        Lock lock(mOverlappedPtrMutex);
        return mOverlappedPtr;
    }

    std::pair<LockPtr, OverlappedAmiPtr> 
    ConnectionOrientedClientTransport::detachOverlappedPtr()
    {
        Lock lock(mOverlappedPtrMutex);
        {
            Lock lock(mOverlappedPtr->mMutex);
            mOverlappedPtr->mpTransport = NULL;
        }
        
        mOverlappedPtr.reset( new OverlappedAmi(this));
        return std::make_pair( 
            LockPtr(new Lock(mOverlappedPtr->mMutex)),
            mOverlappedPtr);

    }

} // namespace RCF
