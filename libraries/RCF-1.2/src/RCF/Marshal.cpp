
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/Marshal.hpp>

#include <algorithm>

#include <boost/function.hpp>

#include <RCF/ClientProgress.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/ThreadLocalCache.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    bool serializeOverride(SerializationProtocolOut &out, ByteBuffer & u)
    {
        int rcfRuntimeVersion = out.getRuntimeVersion();

        if (rcfRuntimeVersion <= 3)
        {
            // Legacy behavior - no metadata for ByteBuffer.
            int len = static_cast<int>(u.getLength());
            serialize(out, len);
            out.insert(u);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool serializeOverride(SerializationProtocolOut &out, ByteBuffer * pu)
    {
        RCF_ASSERT(pu);
        return serializeOverride(out, *pu);
    }

    bool deserializeOverride(SerializationProtocolIn &in, ByteBuffer & u)
    {
        int rcfRuntimeVersion = in.getRuntimeVersion();

        if (rcfRuntimeVersion <= 3)
        {
            // Legacy behavior - no metadata for ByteBuffer.
            int len = 0;
            deserialize(in, len);
            in.extractSlice(u, len);
            return true;
        }
        else
        {
            return false;
        }
    }

    Mutex * gpCandidatesMutex = NULL;
    Candidates * gpCandidates = NULL;

    Mutex & gCandidatesMutex()
    {
        return *gpCandidatesMutex; 
    }

    Candidates & gCandidates()
    {
        return *gpCandidates;
    }

    RCF_ON_INIT_DEINIT_NAMED( 
        gpCandidatesMutex = new Mutex; gpCandidates = new Candidates; ,
        delete gpCandidatesMutex; gpCandidatesMutex = NULL; delete gpCandidates; gpCandidates = NULL,
        InitDeinitCandidates)

    void ClientStub::enrol(I_Future *pFuture)
    {
        mFutures.push_back(pFuture);
        pFuture->setClientStub(this);
    }

    void ClientStub::init( 
        const std::string &subInterface, 
        int fnId, 
        RCF::RemoteCallSemantics rcs)
    {        
        mRequest.init(
            getTargetToken(),
            getTargetName(),
            subInterface,
            fnId,
            getSerializationProtocol(),
            mMarshalingProtocol,
            (rcs == RCF::Oneway),
            false,
            getRcfRuntimeVersion(),
            false,
            mPingBackIntervalMs,
            mArchiveVersion);

        mOut.reset(
            getSerializationProtocol(),
            32,
            mRequest.encodeRequestHeader(),
            mRcfRuntimeVersion,
            mArchiveVersion);

        bool asyncParameters = false;
        {
            ::RCF::CurrentClientStubSentry sentry(shared_from_this());
            mpParameters->write(mOut);

            mFutures.clear();
            asyncParameters = mpParameters->enrolFutures(this);
        }
        if (asyncParameters)
        {
            setAsync(true);
        }
    }

    void ClientStub::connect()
    {
        CurrentClientStubSentry sentry(shared_from_this());
        instantiateTransport();
        if (    !mConnected 
            ||    (    mConnected 
                &&    mAutoReconnect 
                &&    mRcs == Twoway 
                &&    !mTransport->isConnected()))
        {
            mTransport->disconnect(mConnectTimeoutMs);

            if (mAsync)
            {
                mAsyncTimerReason = Connect;
                mAsyncTimerEntry = mTransport->setTimer(mConnectTimeoutMs, this);
            }

            mTransport->connect(*this, mConnectTimeoutMs);            
        }
        else
        {
            onConnectCompleted(true);
        }
    }

    void ClientStub::connectAsync(boost::function0<void> onCompletion)
    {
        setAsync(true);
        instantiateTransport();
        mTransport->setAsync(mAsync);
        setAsyncCallback(onCompletion);
        connect();
    }

    void ClientStub::waitAsync(
        boost::function0<void> onCompletion, 
        boost::uint32_t timeoutMs)
    {
        setAsync(true);
        instantiateTransport();
        mTransport->setAsync(mAsync);
        setAsyncCallback(onCompletion);

        RCF_ASSERT(mAsyncTimerReason == None);
        mAsyncTimerReason = Wait;
        mAsyncTimerEntry = mTransport->setTimer(timeoutMs, this);
    }

    void ClientStub::onConnectCompleted(bool alreadyConnected)
    {
        if (!alreadyConnected)
        {
            if (mAsync)
            {
                if (mAsyncTimerEntry != TimerEntry())
                {
                    mTransport->killTimer(mAsyncTimerEntry);
                    mAsyncTimerEntry = TimerEntry();
                }
                mAsyncTimerReason = None;
            }

            std::vector<FilterPtr> filters;
            mTransport->getTransportFilters(filters);

            std::for_each(
                filters.begin(),
                filters.end(),
                boost::bind(&Filter::reset, _1));

            mTransport->setTransportFilters(std::vector<FilterPtr>());
            if (!filters.empty())
            {
                requestTransportFilters(filters);
            }
            mConnected = true;

            if (
                mClientProgressPtr.get() &&
                (mClientProgressPtr->mTriggerMask & ClientProgress::Event))
            {
                ClientProgress::Action action = ClientProgress::Continue;

                mClientProgressPtr->mProgressCallback(
                    0,
                    0,
                    ClientProgress::Event,
                    ClientProgress::Connect,
                    action);

                RCF_VERIFY(
                    action != ClientProgress::Cancel,
                    Exception(_RcfError_ClientCancel()));
            }
        }

        if (!mEncodedByteBuffers.empty())
        {
            unsigned int timeoutMs = generateTimeoutMs(mEndTimeMs);

            if (mAsync)
            {
                mAsyncTimerReason = Write;
                mAsyncTimerEntry = mTransport->setTimer(timeoutMs, this);
            }

            // Prepend the length field.
            BOOST_STATIC_ASSERT(sizeof(unsigned int) == 4);

            unsigned int messageLength = static_cast<unsigned int>(
                lengthByteBuffers(mEncodedByteBuffers));

            ByteBuffer & byteBuffer = mEncodedByteBuffers.front();
            byteBuffer.expandIntoLeftMargin(4);
            memcpy(byteBuffer.getPtr(), &messageLength, 4);
            RCF::machineToNetworkOrder(byteBuffer.getPtr(), 4, 1);

            if (mBatchMode)
            {
                RCF_ASSERT(mRcs == Oneway);
                RCF_ASSERT(!mAsync);
                RCF_ASSERT(mBatchBufferPtr);

                // Accumulate in the batch buffer.
                std::size_t appendLen = lengthByteBuffers(mEncodedByteBuffers);
                std::size_t currentSize = mBatchBufferPtr->size();

                bool usingTempBuffer = false;

                // If this message will cause us to exceed the limit, then flush first.
                if (    mBatchMaxMessageLength 
                    &&  currentSize + appendLen > mBatchMaxMessageLength)
                {
                    mBatchBufferTemp.resize(appendLen);
                    copyByteBuffers(mEncodedByteBuffers, & mBatchBufferTemp[0] );
                    usingTempBuffer = true;

                    flushBatch(timeoutMs);
                    currentSize = mBatchBufferPtr->size();
                }
                
                mBatchBufferPtr->resize( currentSize + appendLen);

                if (usingTempBuffer)
                {
                    memcpy(
                        & (*mBatchBufferPtr)[currentSize], 
                        &mBatchBufferTemp[0], 
                        mBatchBufferTemp.size());
                }
                else
                {
                    copyByteBuffers(
                        mEncodedByteBuffers, 
                        & (*mBatchBufferPtr)[currentSize] );
                }

                ++mBatchMessageCount;
            }
            else
            {
                int err = getTransport().send(*this, mEncodedByteBuffers, timeoutMs);
                RCF_UNUSED_VARIABLE(err);
            }
        }
        else
        {
            if (mAsyncCallback)
            {
                boost::function0<void> cb = mAsyncCallback;
                mAsyncCallback = boost::function0<void>();

                cb();
            }
        }        
    }

    void ClientStub::send()
    {
        CurrentClientStubSentry sentry(shared_from_this());

        unsigned int totalTimeoutMs = getRemoteCallTimeoutMs();
        unsigned int startTimeMs = getCurrentTimeMs();
        mEndTimeMs = startTimeMs + totalTimeoutMs;            

        ThreadLocalCached< std::vector<ByteBuffer> > tlcByteBuffers;
        std::vector<ByteBuffer> &byteBuffers = tlcByteBuffers.get();

        mOut.extractByteBuffers(byteBuffers);
        int protocol = mOut.getSerializationProtocol();
        RCF_UNUSED_VARIABLE(protocol);

        mEncodedByteBuffers.resize(0);

        mRequest.encodeRequest(
            byteBuffers,
            mEncodedByteBuffers,
            getMessageFilters());

        instantiateTransport();

        mTransport->setAsync(mAsync);

        WithProgressCallback *pWithCallbackProgress =
            dynamic_cast<WithProgressCallback *>(&getTransport());

        if (pWithCallbackProgress)
        {
            pWithCallbackProgress->setClientProgressPtr(
                getClientProgressPtr());
        }

        // TODO: make sure timeouts behave as expected, esp. when connect() is doing round trip filter setup calls
        connect();

    }

    void ClientStub::onSendCompleted()
    {
        mEncodedByteBuffers.resize(0);
        if (mRcs != RCF::Oneway)
        {
            receive();
        }
    }

    void ClientStub::receive()
    {
        if (mPingBackIntervalMs && mRcfRuntimeVersion >= 5)
        {
            mPingBackCheckIntervalMs = 3 * mPingBackIntervalMs;

            mNextPingBackCheckMs = 
                Platform::OS::getCurrentTimeMs() + mPingBackCheckIntervalMs;

            // So we avoid the special value 0.
            mNextPingBackCheckMs |= 1;
        }

        if (mAsync)
        {
            mAsyncTimerReason = Read;
        }

        unsigned int timeoutMs = generateTimeoutMs(mEndTimeMs);
        mEncodedByteBuffer.clear();
        int err = getTransport().receive(*this, mEncodedByteBuffer, timeoutMs);
        RCF_UNUSED_VARIABLE(err);
    }

    void ClientStub::onException(const Exception & e)
    {
        if (mAsync)
        {
            onError(e);
        }
        else
        {
            e.throwSelf();
        }
    }

    void ClientStub::onReceiveCompleted()
    {
        if (mAsync)
        {
            if (mAsyncTimerEntry != TimerEntry())
            {
                mTransport->killTimer(mAsyncTimerEntry);
                mAsyncTimerEntry = TimerEntry();
            }            
            mAsyncTimerReason = None;
        }

        ByteBuffer unfilteredByteBuffer;

        MethodInvocationResponse response;

        mRequest.decodeResponse(
            mEncodedByteBuffer,
            unfilteredByteBuffer,
            response,
            getMessageFilters());

        mEncodedByteBuffer.clear();

        mIn.reset(
            unfilteredByteBuffer,
            mOut.getSerializationProtocol(),
            mRcfRuntimeVersion,
            mArchiveVersion);

        if (response.isException())
        {
            std::auto_ptr<RemoteException> remoteExceptionAutoPtr(
                response.getExceptionPtr());

            if (!remoteExceptionAutoPtr.get())
            {
                deserialize(mIn, remoteExceptionAutoPtr);
            }

            onException(*remoteExceptionAutoPtr);
        }
        else if (response.isError())
        {
            int err = response.getError();
            if (err == RcfError_VersionMismatch)
            {
                int version = response.getArg0();

                RCF_VERIFY(
                    version < getRcfRuntimeVersion(),
                    Exception(_RcfError_Encoding()));

                if (getAutoVersioning() && getTries() == 0)
                {
                    setRcfRuntimeVersion(version);
                    setTries(1);

                    init(mRequest.getSubInterface(), mRequest.getFnId(), mRcs);
                    send();
                }
                else
                {
                    onException(VersioningException(version));
                }
            }
            else if (err == RcfError_PingBack)
            {
                // A ping back message carries a parameter specifying
                // the ping back interval in ms. The client can use that
                // to make informed decisions about whether the connection
                // has died or not.

                mPingBackIntervalMs = response.getArg0();

                // Record a timestamp and go back to receiving.

                ++mPingBackCount;
                mPingBackTimeStamp = Platform::OS::getCurrentTimeMs();

                receive();
            }
            else
            {
                onException(RemoteException( Error(response.getError()) ));
            }
        }
        else
        {
            RCF::CurrentClientStubSentry sentry(shared_from_this());
            mpParameters->read(mIn);
            mIn.clearByteBuffer();

            if (mAsync)
            {
                Lock lock(*mSignalledMutex);
                mSignalled = true;
                mSignalledCondition->notify_all();
            }

            if (mAsyncCallback)
            {
                boost::function0<void> cb = mAsyncCallback;
                mAsyncCallback = boost::function0<void>();

                cb();
            }
        }
    }

    bool ClientStub::ready()
    {
        Lock lock(*mSignalledMutex);
        return mSignalled;
    }

    void ClientStub::wait(boost::uint32_t timeoutMs)
    {
        Lock lock(*mSignalledMutex);
        if (!mSignalled)
        {
            if (timeoutMs)
            {
                mSignalledCondition->timed_wait(lock, timeoutMs);
            }
            else
            {
                mSignalledCondition->wait(lock);
            }            
        }
    }

    void ClientStub::cancel()
    {
        mTransport->cancel();
    }

    void ClientStub::setSubRcfClientPtr(RcfClientPtr clientStubPtr)
    {
        Lock lock(mSubRcfClientMutex);
        mSubRcfClientPtr = clientStubPtr;
    }

    RcfClientPtr ClientStub::getSubRcfClientPtr()
    {
        Lock lock(mSubRcfClientMutex);
        return mSubRcfClientPtr;
    }

    void ClientStub::call( 
        RCF::RemoteCallSemantics rcs)
    {
        mRetry = false;
        mRcs = rcs;
        mPingBackTimeStamp = 0;
        mPingBackCount = 0;

        RCF_ASSERT(mAsyncTimerReason == None);

        // Set the progress timer timeouts.
        mTimerIntervalMs = 0;
        mNextTimerCallbackMs = 0;

        if (    mClientProgressPtr.get()
            &&  mClientProgressPtr->mTriggerMask & ClientProgress::Timer)
        {            
            mTimerIntervalMs = mClientProgressPtr->mTimerIntervalMs;

            mNextTimerCallbackMs = 
                Platform::OS::getCurrentTimeMs() + mTimerIntervalMs;

            // So we avoid the special value 0.
            mNextTimerCallbackMs |= 1;
        }

        // We don't set ping back timeouts until we are about to receive.
        mPingBackCheckIntervalMs = 0;
        mNextPingBackCheckMs = 0;

        mSignalled = false;
        
        send();
    }

    void ClientStub::setAsync(bool async)
    {
        mAsync = async;

        if (mAsync && !mSignalledMutex)
        {
            mSignalledMutex.reset( new Mutex() );
            mSignalledCondition.reset( new Condition() );
        }
    }

    bool ClientStub::getAsync()
    {
        return mAsync;
    }

    void ClientStub::setAsyncCallback(boost::function0<void> callback)
    {
        mAsyncCallback = callback;
    }

    std::auto_ptr<Exception> ClientStub::getAsyncException()
    {
        Lock lock(*mSignalledMutex);
        return mAsyncException;
    }

    void ClientStub::setAsyncException(std::auto_ptr<Exception> asyncException)
    {
        Lock lock(*mSignalledMutex);
        mAsyncException = asyncException;
    }

    bool ClientStub::hasAsyncException()
    {
        Lock lock(*mSignalledMutex);
        return mAsyncException.get() != NULL;
    }

    typedef boost::shared_ptr< ClientTransportAutoPtr > ClientTransportAutoPtrPtr;

    void vc6_helper(
        boost::function1<void, ClientTransportAutoPtr> func,
        ClientTransportAutoPtrPtr clientTransportAutoPtrPtr)
    {
        func(*clientTransportAutoPtrPtr);
    }

    void convertRcfSessionToRcfClient(
        boost::function1<void, ClientTransportAutoPtr> func,
        RemoteCallSemantics rcs)
    {
        RcfSession & rcfSession = getCurrentRcfSession();

        I_ServerTransportEx & serverTransport =
            dynamic_cast<I_ServerTransportEx &>(
                rcfSession.getProactor().getServerTransport());

        ClientTransportAutoPtrPtr clientTransportAutoPtrPtr(
            new ClientTransportAutoPtr(
                serverTransport.createClientTransport(rcfSession.shared_from_this())));

        rcfSession.addOnWriteCompletedCallback(
            boost::bind(
                vc6_helper,
                func,
                clientTransportAutoPtrPtr) );

        bool closeSession = (rcs == RCF::Twoway);

        rcfSession.setCloseSessionAfterWrite(closeSession);
    }

    RcfSessionPtr convertRcfClientToRcfSession(
        ClientStub & clientStub, 
        RcfServer & server)
    {
        I_ServerTransportEx &serverTransportEx =
            dynamic_cast<RCF::I_ServerTransportEx &>(server.getServerTransport());

        SessionPtr sessionPtr =
            serverTransportEx.createServerSession(
                clientStub.releaseTransport(),
                StubEntryPtr());

        ClientTransportAutoPtr apClientTransport(
            serverTransportEx.createClientTransport(sessionPtr) );

        clientStub.setTransport(apClientTransport);

        return boost::static_pointer_cast<RcfSession>(sessionPtr);
    }

} // namespace RCF
