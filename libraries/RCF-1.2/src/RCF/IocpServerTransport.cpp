
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/IocpServerTransport.hpp>

#include <RCF/ClientTransport.hpp>
#include <RCF/CurrentSession.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/ObjectPool.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/ThreadManager.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    // Iocp

    Iocp::Iocp(int nMaxConcurrency)
    {
        m_hIOCP = NULL;
        Create(nMaxConcurrency);
    }

    Iocp::~Iocp()
    {
        RCF_DTOR_BEGIN
            if (m_hIOCP != NULL)
            {
                int ret = CloseHandle(m_hIOCP);
                int err = Platform::OS::BsdSockets::GetLastError();
                RCF_VERIFY(
                    ret,
                    Exception(
                        _RcfError_Socket(),
                        err,
                        RcfSubsystem_Os,
                        "CloseHande() failed"))
                    (m_hIOCP);
            }
        RCF_DTOR_END
    }

    void Iocp::Create(int nMaxConcurrency)
    {
        m_hIOCP = CreateIoCompletionPort(
            INVALID_HANDLE_VALUE,
            NULL,
            0,
            nMaxConcurrency);

        int err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(
            m_hIOCP != NULL,
            Exception(
                _RcfError_Socket(),
                err,
                RcfSubsystem_Os,
                "CreateIoCompletionPort() failed"));

    }

    void Iocp::AssociateDevice(HANDLE hDevice, ULONG_PTR CompKey)
    {
        BOOL fOk =
            (CreateIoCompletionPort(hDevice, m_hIOCP, CompKey, 0) == m_hIOCP);

        int err = Platform::OS::BsdSockets::GetLastError();
        RCF_VERIFY(
            fOk,
            Exception(
                _RcfError_Socket(),
                err,
                RcfSubsystem_Os,
                "CreateIoCompletionPort() failed"))
            (hDevice)(static_cast<__int64>(CompKey));
    }

    void Iocp::AssociateSocket(SOCKET hSocket, ULONG_PTR CompKey)
    {
        AssociateDevice((HANDLE) hSocket, CompKey);
    }

    void Iocp::PostStatus(ULONG_PTR CompKey, DWORD dwNumBytes, OVERLAPPED* po)
    {
        BOOL fOk = PostQueuedCompletionStatus(m_hIOCP, dwNumBytes, CompKey, po);
        RCF_ASSERT(fOk);
    }

    BOOL Iocp::GetStatus(
        ULONG_PTR* pCompKey,
        PDWORD pdwNumBytes,
        OVERLAPPED** ppo,
        DWORD dwMilliseconds)
    {
        return GetQueuedCompletionStatus(
            m_hIOCP,
            pdwNumBytes,
            pCompKey,
            ppo,
            dwMilliseconds);
    }

    // FilterProxy

    class RCF_EXPORT FilterProxy : public RCF::IdentityFilter
    {
    public:
        FilterProxy(
            IocpSessionState &sessionState,
            Filter &filter,
            bool top);

    private:
        void    read(
                    const ByteBuffer &byteBuffer,
                    std::size_t bytesRequested);

        void    write(
                    const std::vector<ByteBuffer> &byteBuffers);

        void    onReadCompleted(
                    const ByteBuffer &byteBuffer);

        void    onWriteCompleted(
                    std::size_t bytesTransferred);

        const FilterDescription &getFilterDescription() const
        {
            RCF_ASSERT(0);
            return * (const FilterDescription *) NULL;
        }

        IocpSessionState &  mSessionState;
        Filter &            mFilter;
        bool                mTop;
    };

    FilterProxy::FilterProxy(
        IocpSessionState &sessionState,
        Filter &filter,
        bool top) :
            mSessionState(sessionState),
            mFilter(filter),
            mTop(top)
    {}

    void FilterProxy::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        mTop ?
            mFilter.read(byteBuffer, bytesRequested) :
            mSessionState.read(byteBuffer, bytesRequested);
    }

    void FilterProxy::write(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        mTop ?
            mFilter.write(byteBuffers) :
            mSessionState.write(byteBuffers);
    }

    void FilterProxy::onReadCompleted(
        const ByteBuffer &byteBuffer)
    {
        std::size_t bytesTransferred = byteBuffer.getLength();
        mTop ?
            mSessionState.onFilteredReadWriteCompleted(bytesTransferred) :
            mFilter.onReadCompleted(byteBuffer);
    }

    void FilterProxy::onWriteCompleted(
        std::size_t bytesTransferred)
    {
        mTop ?
            mSessionState.onFilteredReadWriteCompleted(bytesTransferred) :
            mFilter.onWriteCompleted(bytesTransferred);
    }

    // SessionState

    void resetSessionStatePtr(IocpSessionStatePtr &sessionStatePtr)
    {
        sessionStatePtr.reset();
    }
   
    void IocpSessionState::read(
        const ByteBuffer &byteBuffer,
        std::size_t bufferLen)
    {
        if (bufferLen == 0)
        {
            RCF_ASSERT(byteBuffer.isEmpty());
            if (mReadBufferSecondaryPtr)
            {
                getObjectPool().put(mReadBufferSecondaryPtr);
            }
        }
        else if (byteBuffer.getLength() == 0 && bufferLen > 0)
        {
            std::vector<char> &vec = getUniqueReadBufferSecondary();
            vec.resize(bufferLen);
            mReadByteBufferSecondary = getReadByteBufferSecondary();
        }
        else
        {
            mReadByteBufferSecondary = ByteBuffer(byteBuffer, 0, bufferLen);
        }

        // If doing a zero byte read, check that we're not holding on to any buffers.

        if (bufferLen == 0)
        {
            RCF_ASSERT(!mReadBufferPtr);
            RCF_ASSERT(!mReadBufferSecondaryPtr.get());
        }

        implRead(mReadByteBufferSecondary, bufferLen);
    }

    
    void IocpSessionState::write(
        const std::vector<ByteBuffer> &byteBuffers,
        IocpSessionState * pReflectee)
    {
        implWrite(byteBuffers, pReflectee);        
    }

    IocpSessionState::IocpSessionState(IocpServerTransport &transport) :
        mPreState(Accepting),
        mPostState(Reading),
        mIssueZeroByteRead(RCF_DEFAULT_INIT),
        mReadBufferRemaining(RCF_DEFAULT_INIT),
        mWriteBufferRemaining(RCF_DEFAULT_INIT),
        mError(RCF_DEFAULT_INIT),
        mOwnFd(true),
        mCloseAfterWrite(RCF_DEFAULT_INIT),
        mTransport(transport),
        mReflected(RCF_DEFAULT_INIT),
        mHasBeenClosed(RCF_DEFAULT_INIT),
        mMutex()
    {
    }

    void IocpSessionState::resetState()
    {
        mPreState               = Accepting;
        mPostState              = Reading;
        mReadBufferRemaining    = 0;
        mWriteBufferRemaining   = 0;
        mError                  = 0;
        mOwnFd                  = true;
        mCloseAfterWrite        = false;
        mReflected              = false;
        mHasBeenClosed          = false;

        mTransportFilters.clear();
        mReflectionSessionStateWeakPtr.reset();
        mThisPtr.reset();
    }

    int IocpSessionState::getLastError()
    {
        return mError;
    }

    IocpSessionState::~IocpSessionState()
    {
        RCF_DTOR_BEGIN

            mTransport.unregisterSession(mWeakThisPtr);

            RCF2_TRACE("")(mSessionPtr.get())(mOwnFd);

            // close the reflected session, if appropriate
            if (mReflected)
            {
                IocpSessionStatePtr reflectionSessionStatePtr(
                    mReflectionSessionStateWeakPtr.lock());

                if (reflectionSessionStatePtr)
                {
                    reflectionSessionStatePtr->postClose();
                }
            }

            // Return our buffers to the buffer pool, if appropriate.

            mTransportFilters.clear();

            if (mReadBufferPtr.get() && mReadBufferPtr.unique())
            {
                getObjectPool().put(mReadBufferPtr);
            }

            if (mReadBufferSecondaryPtr.get() && mReadBufferSecondaryPtr.unique())
            {
                getObjectPool().put(mReadBufferSecondaryPtr);
            }
            
        RCF_DTOR_END
    }

    void IocpSessionState::setTransportFilters(
        const std::vector<FilterPtr> &filters)
    {
        mTransportFilters.clear();
        if (!filters.empty())
        {
            mTransportFilters.push_back(
                FilterPtr(new FilterProxy(*this, *filters.front(), true)));

            std::copy(
                filters.begin(),
                filters.end(),
                std::back_inserter(mTransportFilters));

            mTransportFilters.push_back(
                FilterPtr(new FilterProxy(*this, *filters.back(), false)));

            RCF::connectFilters(mTransportFilters);
        }
    }

    void IocpSessionState::getTransportFilters(std::vector<FilterPtr> &filters)
    {
        filters.resize(0);
        if (!mTransportFilters.empty())
        {
            RCF_ASSERT(mTransportFilters.size() >= 3);

            // Older compilers choke on this (vc6, bcb56, gcc295).

            //std::copy(
            //    ++mTransportFilters.begin(), 
            //    --mTransportFilters.end(), 
            //    std::back_inserter(filters));

            for (std::size_t i=1; i<mTransportFilters.size()-1; ++i)
            {
                filters.push_back(mTransportFilters[i]);
            }
        }
    }

    void IocpSessionState::filteredRead(
        ByteBuffer &byteBuffer,
        std::size_t bufferLen)
    {
        mTransportFilters.empty() ?
            read(byteBuffer, bufferLen) :
            mTransportFilters.front()->read(byteBuffer, bufferLen);
    }

    void IocpSessionState::filteredWrite(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        mTransportFilters.empty() ?
            write(byteBuffers) :
            mTransportFilters.front()->write(byteBuffers);
    }

    void IocpSessionState::onReadWriteCompleted(
        std::size_t bytesTransferred)
    {
        // Handle an accept completion.
        if (mPreState == Accepting)
        {
            RCF_ASSERT( bytesTransferred == 0 )(bytesTransferred);
            transition();
            return;
        }
        
        // Handle reflected completions
        if (mReflected)
        {
            reflect(bytesTransferred);
            return;
        }

        // Handle normal read and write completions.
        RCF_ASSERT(mPostState == Reading || mPostState == Writing)(mPostState);
        if (mPostState == Reading)
        {
            RCF2_TRACE("read completed")(bytesTransferred);

            if (mTransportFilters.empty())
            {
                onFilteredReadWriteCompleted(bytesTransferred);
            }
            else
            {
                mReadByteBufferSecondary = ByteBuffer(
                    mReadByteBufferSecondary, 
                    0, 
                    bytesTransferred);

                mTransportFilters.back()->onReadCompleted(
                    mReadByteBufferSecondary);
            }
        }
        else if (mPostState == Writing)
        {
            RCF2_TRACE("write completed")(bytesTransferred);

            mTransportFilters.empty() ?
                onFilteredReadWriteCompleted(bytesTransferred) :
                mTransportFilters.back()->onWriteCompleted(bytesTransferred);
        }
        else
        {
            RCF_ASSERT(0);
        }
    }

    std::vector<char> &IocpSessionState::getReadBuffer()
    {
        if (!mReadBufferPtr)
        {
            getObjectPool().get(mReadBufferPtr);
        }
        return *mReadBufferPtr;
    }

    std::vector<char> &IocpSessionState::getUniqueReadBuffer()
    {
        if (!mReadBufferPtr || !mReadBufferPtr.unique())
        {
            getObjectPool().get(mReadBufferPtr);
        }
        return *mReadBufferPtr;
    }

    ByteBuffer IocpSessionState::getReadByteBuffer()
    {
        return ByteBuffer(
            &(*mReadBufferPtr)[0],
            (*mReadBufferPtr).size(),
            mReadBufferPtr);
    }

    std::vector<char> &IocpSessionState::getReadBufferSecondary()
    {
        if (!mReadBufferSecondaryPtr)
        {
            getObjectPool().get(mReadBufferSecondaryPtr);
        }
        return *mReadBufferSecondaryPtr;
    }

    std::vector<char> &IocpSessionState::getUniqueReadBufferSecondary()
    {
        if (!mReadBufferSecondaryPtr || !mReadBufferSecondaryPtr.unique())
        {
            getObjectPool().get(mReadBufferSecondaryPtr);
        }
        return *mReadBufferSecondaryPtr;
    }

    ByteBuffer IocpSessionState::getReadByteBufferSecondary() const
    {
        return ByteBuffer(
            &(*mReadBufferSecondaryPtr)[0],
            (*mReadBufferSecondaryPtr).size(),
            mReadBufferSecondaryPtr);
    }

    I_ServerTransport & IocpSessionState::getServerTransport()
    {
        return mTransport;
    }

    void IocpSessionState::sendServerError(int error)
    {
        
        std::vector<ByteBuffer> byteBuffers(1);
        encodeServerError(byteBuffers.front(), error);

        mPreState = IocpSessionState::Ready;
        mCloseAfterWrite = true;
        postWrite(byteBuffers);
    }

    void IocpSessionState::implOnMessageLengthError()
    {
        sendServerError(RcfError_ServerMessageLength);
    }

    void IocpSessionState::onAccept()
    {
        implOnAccept();
    }

    void IocpSessionState::transition()
    {
        switch(mPreState)
        {
        case IocpSessionState::Accepting:

            onAccept();            

            break;

        case IocpSessionState::ReadingDataCount:
        {
            RCF_ASSERT(
                0 <= mReadBufferRemaining && mReadBufferRemaining <= 4)
                (mReadBufferRemaining);

            if (mIssueZeroByteRead)
            {
                RCF_ASSERT(!mReadBufferPtr);
                ByteBuffer byteBuffer;
                filteredRead(byteBuffer, 0);
            }
            else
            {
                std::vector<char> &readBuffer = getReadBuffer();
                if (mReadBufferRemaining == 0 && readBuffer.empty())
                {
                    // Zero byte read completed.
                    getUniqueReadBuffer().resize(4);
                    mReadBufferRemaining = 4;
                    transition();
                }
                else if (mReadBufferRemaining == 0)
                {
                    unsigned int packetLength = 0;
                    memcpy( &packetLength, &readBuffer[0], 4);
                    networkToMachineOrder(&packetLength, 4, 1);
                    if (    0 < packetLength 
                        &&  packetLength <= mTransport.getMaxMessageLength())
                    {
                        // TODO: configurable limit on packetLength
                        getReadBuffer().resize(packetLength);
                        mReadBufferRemaining = packetLength;
                        mPreState = IocpSessionState::ReadingData;
                        transition();
                    }
                    else
                    {
                        implOnMessageLengthError();
                    }
                }
                else if (0 < mReadBufferRemaining && mReadBufferRemaining <= 4)
                {
                    RCF_ASSERT(
                        mReadBufferRemaining <= readBuffer.size())
                        (mReadBufferRemaining)(readBuffer.size());

                    std::size_t readIdx = readBuffer.size() - mReadBufferRemaining;
                    char *readPos = & readBuffer[readIdx];

                    mReadByteBuffer = ByteBuffer(
                        readPos,
                        mReadBufferRemaining);

                    filteredRead(mReadByteBuffer, mReadBufferRemaining);
                }
            }

            break;
        }        

        case IocpSessionState::ReadingData:
        {
            if (mReadBufferRemaining == 0)
            {
                mPreState = IocpSessionState::Ready;
                mTransport.getSessionManager().onReadCompleted(mSessionPtr);
            }
            else
            {
                getReadBuffer();

                RCF_ASSERT(
                    mReadBufferRemaining <= mReadBufferPtr->size())
                    (mReadBufferRemaining)(mReadBufferPtr->size());

                std::size_t readIdx = mReadBufferPtr->size() - mReadBufferRemaining;
                char *readPos = & (*mReadBufferPtr)[readIdx];

                mReadByteBuffer = ByteBuffer(
                    readPos,
                    mReadBufferRemaining);

                filteredRead(
                    mReadByteBuffer,
                    mReadBufferRemaining);
            }

            break;
        }
            

        case IocpSessionState::WritingData:
        {
            if (mWriteBufferRemaining == 0)
            {
                if (mCloseAfterWrite)
                {
                    // We want to keep the connection open long enough for the client
                    // to receive the data we sent. Even though the write has completed,
                    // if we immediately close the socket, it is quite likely that the data
                    // won't make it across to the client.

                    implDelayCloseAfterSend();
                }
                else
                {
                    mWriteByteBuffers.resize(0);
                    mSlicedWriteByteBuffers.resize(0);
                    mPreState = IocpSessionState::Ready;
                    mTransport.getSessionManager().onWriteCompleted(mSessionPtr);
                }
            }
            else
            {
                std::size_t writeBufferLen = RCF::lengthByteBuffers(mWriteByteBuffers);

                RCF_ASSERT( mWriteBufferRemaining <= writeBufferLen )
                    (mWriteBufferRemaining)(writeBufferLen);

                std::size_t offset = writeBufferLen - mWriteBufferRemaining;

                mSlicedWriteByteBuffers.resize(0);

                RCF::sliceByteBuffers(
                    mSlicedWriteByteBuffers,
                    mWriteByteBuffers,
                    offset);

                filteredWrite(mSlicedWriteByteBuffers);
            }

            break;
        }

        default:

            RCF_ASSERT(0)(mPreState);
        }

    }

    void IocpSessionState::reflect(
        std::size_t bytesTransferred)
    {
        RCF_ASSERT(
            mPreState == IocpSessionState::ReadingData ||
            mPreState == IocpSessionState::ReadingDataCount ||
            mPreState == IocpSessionState::WritingData)
            (mPreState);

        RCF_ASSERT(mReflected);

        if (bytesTransferred == 0)
        {
            // Previous operation was aborted for some reason (probably because
            // of a thread exiting). Reissue the operation.

            if (mPreState == IocpSessionState::WritingData)
            {
                mPreState = IocpSessionState::ReadingData;
            }
            else
            {
                mPreState = IocpSessionState::WritingData;
            }
        }

        if (mPreState == IocpSessionState::WritingData)
        {
            mPreState = IocpSessionState::ReadingData;
            std::vector<char> & readBuffer = getReadBuffer();
            readBuffer.resize(8*1024);            
            read(ByteBuffer(&readBuffer[0], readBuffer.size()), 8*1024);
        }
        else if (
            mPreState == IocpSessionState::ReadingData ||
            mPreState == IocpSessionState::ReadingDataCount)
        {
            mPreState = IocpSessionState::WritingData;

            IocpSessionStatePtr sessionStatePtr = 
                mReflectionSessionStateWeakPtr.lock();
            
            if (sessionStatePtr)
            {
                RCF_ASSERT(sessionStatePtr->mReflected);
                std::vector<char> & readBuffer = getReadBuffer();

                mWriteByteBuffers.resize(0);

                mWriteByteBuffers.push_back(
                    ByteBuffer(&readBuffer[0], bytesTransferred));

                write(mWriteByteBuffers, sessionStatePtr.get());
            }
        }
    }

    void IocpSessionState::onFilteredReadWriteCompleted(
        std::size_t bytesTransferred)
    {
        {
            if (mIssueZeroByteRead)
            {
                mIssueZeroByteRead = false;
            }

            if (mPreState == IocpSessionState::ReadingData ||
                mPreState == IocpSessionState::ReadingDataCount)
            {
                RCF_ASSERT(
                    bytesTransferred <= mReadBufferRemaining )
                    (bytesTransferred)(mReadBufferRemaining);

                mReadBufferRemaining -= bytesTransferred;
                transition();
            }
            else if (mPreState == IocpSessionState::WritingData)
            {
                RCF_ASSERT(
                    bytesTransferred <= mWriteBufferRemaining)
                    (bytesTransferred)(mWriteBufferRemaining);

                mWriteBufferRemaining -= bytesTransferred;
                transition();
            }
        }
    }

    void IocpSessionState::postWrite(
        std::vector<ByteBuffer> &byteBuffers)
    {
        if (mError)
        {
            return;
        }

        RCF_ASSERT(
            mPreState == IocpSessionState::Ready)
            (mPreState);

        BOOST_STATIC_ASSERT(sizeof(unsigned int) == 4);

        mWriteByteBuffers.resize(0);

        std::copy(
            byteBuffers.begin(),
            byteBuffers.end(),
            std::back_inserter(mWriteByteBuffers));

        byteBuffers.resize(0);

        int messageSize = static_cast<int>(
            RCF::lengthByteBuffers(mWriteByteBuffers));
        
        ByteBuffer &byteBuffer = mWriteByteBuffers.front();

        RCF_ASSERT(
            byteBuffer.getLeftMargin() >= 4)
            (byteBuffer.getLeftMargin());

        byteBuffer.expandIntoLeftMargin(4);
        memcpy(byteBuffer.getPtr(), &messageSize, 4);
        RCF::machineToNetworkOrder(byteBuffer.getPtr(), 4, 1);

        mPreState = IocpSessionState::WritingData;
        mWriteBufferRemaining = RCF::lengthByteBuffers(mWriteByteBuffers);

        transition();
    }

    void IocpSessionState::postRead()
    {
        if (mError)
        {
            return;
        }

        mReadByteBuffer.clear();
        mReadByteBufferSecondary.clear();

        if (mDropReceiveBuffer)
        {
            mReadBufferPtr.reset();
            mDropReceiveBuffer = false;
        }
        else
        {
            if (mReadBufferPtr)
            {
                getObjectPool().put(mReadBufferPtr);
            }
        }

        mPreState = IocpSessionState::ReadingDataCount;
        mReadBufferRemaining = 0;
        mIssueZeroByteRead = true;

        transition();
    }

    void IocpSessionState::notifyClose(
        const IocpSessionStateWeakPtr &sessionStateWeakPtr)
    {
        IocpSessionStatePtr sessionStatePtr(sessionStateWeakPtr.lock());
        if (sessionStatePtr)
        {
            Lock lock(sessionStatePtr->mMutex);
            sessionStatePtr->mHasBeenClosed = true;
        }
    }

    void IocpSessionState::postClose()
    {
        Lock lock(mMutex);
        if (mOwnFd && !mHasBeenClosed)
        {
            implClose();
        }
    }

    void IocpSessionState::closeSession(
        const IocpSessionStateWeakPtr &sessionStateWeakPtr)
    {
        IocpSessionStatePtr sessionStatePtr(sessionStateWeakPtr.lock());
        if (sessionStatePtr)
        {
            sessionStatePtr->postClose();   
        }
    }

    // IocpServerTransport

    IocpServerTransport::IocpServerTransport() :
        mpSessionManager(RCF_DEFAULT_INIT),        
        mMaxSendRecvSize(1024*1024*10),
        mStopFlag(RCF_DEFAULT_INIT),
        mOpen(RCF_DEFAULT_INIT),
        mIocpAutoPtr(RCF_DEFAULT_INIT)
    {}

    IocpServerTransport::~IocpServerTransport()
    {
    }

    void IocpServerTransport::setMaxSendRecvSize(std::size_t maxSendRecvSize)
    {
        mMaxSendRecvSize = maxSendRecvSize;
    }

    std::size_t IocpServerTransport::getMaxSendRecvSize() const
    {
        return mMaxSendRecvSize;
    }

    void IocpServerTransport::registerSession(IocpSessionStateWeakPtr session)
    {
        Lock lock(mSessionsMutex);
        mSessions.insert(session);
    }

    void IocpServerTransport::unregisterSession(IocpSessionStateWeakPtr session)
    {
        Lock lock(mSessionsMutex);
        mSessions.erase(session);
    }

    void IocpServerTransport::open()
    {
        if (!mIocpAutoPtr.get())
        {
            //RCF_ASSERT(mIocpAutoPtr.get() == NULL);

            // create io completion port and associate the listener socket
            // TODO: need to configure this?
            int nMaxConcurrency = 0;
            mIocpAutoPtr.reset(new Iocp());
            mIocpAutoPtr->Create(nMaxConcurrency);

            mSessions.clear();

            implOpen();        
        }
    }

    void IocpServerTransport::close()
    {   
        implClose();
        mIocpAutoPtr.reset();

        if (!mSessions.empty())
        {
            std::vector<IocpSessionStateWeakPtr> sessionStates;

            std::copy(
                mSessions.begin(), 
                mSessions.end(), 
                std::back_inserter(sessionStates));

            for (std::size_t i=0; i<sessionStates.size(); ++i)
            {
                sessionStates[i].lock()->mThisPtr.reset();
            }

            sessionStates.clear();
        }

        RCF_ASSERT(mSessions.empty());
    }

    void IocpSessionState::onCompletion(
        BOOL ret, 
        DWORD dwErr,
        ULONG_PTR completionKey, 
        DWORD dwNumBytes)
    {
        RCF_UNUSED_VARIABLE(completionKey);

        RCF_ASSERT(mThisPtr);
        IocpSessionStatePtr sessionStatePtr = mThisPtr;
        mThisPtr.reset();

        mError = dwErr;

        if (    (   ret == TRUE 
                    && dwNumBytes > 0) 

            ||  (   ret == TRUE 
                    && dwNumBytes == 0 
                    && mPreState == IocpSessionState::Accepting)

            ||  (   ret == TRUE 
                    && dwNumBytes == 0 
                    && mIssueZeroByteRead == true 
                    && implIsConnected())

            ||  (   ret == FALSE 
                    && dwErr == ERROR_OPERATION_ABORTED
                    && dwNumBytes == 0
                    && mPreState != IocpSessionState::Accepting))
        {

            // On pre-Vista Windows, when a thread terminates, any outstanding
            // I/O requests it has will complete with the error 
            // ERROR_OPERATION_ABORTED. In this case, we just reissue the I/O
            // request, on the current thread.

            RcfSessionPtr rcfSessionPtr = 
                boost::static_pointer_cast<RcfSession>(
                mSessionPtr);

            SetCurrentSessionGuard guard(rcfSessionPtr);

            int bytesRead = dwNumBytes;
            onReadWriteCompleted(bytesRead);
        }
        else
        {
            // For any other error than ERROR_OPERATION_ABORTED, we close
            // the connection. Explicitly calling postClose() here allows
            // the session to do a reconnect, if applicable. If we just let
            // the session fall off the stack, it will be closed properly,
            // but it won't be be possible to recycle it.
            postClose();
        }
    }

    void IocpServerTransport::cycle(
        int timeoutMs,
        const volatile bool &stopFlag)
    {

        RCF_UNUSED_VARIABLE(stopFlag);

        // extract a completed io operation from the iocp
        DWORD           dwMilliseconds = timeoutMs < 0 ? INFINITE : timeoutMs;
        DWORD           dwNumBytes = 0;
        ULONG_PTR       completionKey = 0;
        OVERLAPPED *    pOverlapped = 0;
        DWORD           dwErr = 0;

        BOOL ret = mIocpAutoPtr->GetStatus(
            &completionKey,
            &dwNumBytes,
            &pOverlapped,
            dwMilliseconds);

        if (ret == 0)
        {
            dwErr = GetLastError();
        }

        RCF_ASSERT(
            pOverlapped || (!pOverlapped && dwErr == WAIT_TIMEOUT))
            (pOverlapped)(dwErr);

        if (ret == 0 && dwErr == WAIT_TIMEOUT)
        {
            // Just a timeout, nothing to do here.
        }
        else if (pOverlapped)
        {
            IocpOverlapped *pIocpOverlapped =
                static_cast<IocpOverlapped *>(pOverlapped);

            ThreadTouchGuard threadTouchGuard;

            pIocpOverlapped->onCompletion(ret, dwErr, completionKey, dwNumBytes);
        }
    }

    // create a server-aware client transport on the connection associated with
    // this session. fd is owned by the client, not the server session.
    // will only create a client transport the first time it is called,
    // after that an empty auto_ptr is returned.
    ClientTransportAutoPtr IocpServerTransport::createClientTransport(
        SessionPtr sessionPtr)
    {
        IocpSessionState & sessionState =
            dynamic_cast<IocpSessionState &>(sessionPtr->getProactor());

        IocpSessionStatePtr sessionStatePtr = sessionState.shared_from_this();

        ClientTransportAutoPtr clientTransportAutoPtr =
            sessionStatePtr->implCreateClientTransport();

        return clientTransportAutoPtr;
    }

    // create a server session on the connection associated with the client transport
    SessionPtr IocpServerTransport::createServerSession(
        ClientTransportAutoPtr clientTransportAutoPtr,
        StubEntryPtr stubEntryPtr)
    {
        IocpSessionStatePtr sessionStatePtr = implCreateServerSession(*clientTransportAutoPtr);

        sessionStatePtr->mPreState = IocpSessionState::WritingData;
        sessionStatePtr->mWriteBufferRemaining = 0;      

        if (stubEntryPtr)
        {
            RcfSessionPtr rcfSessionPtr = 
                boost::static_pointer_cast<RcfSession>( 
                    sessionStatePtr->mSessionPtr );

            rcfSessionPtr->setDefaultStubEntryPtr(stubEntryPtr);
        }
        
        registerSession(sessionStatePtr->mWeakThisPtr);
        sessionStatePtr->transition();
        return sessionStatePtr->mSessionPtr;
    }

    // start reflecting data between the two given sessions
    bool IocpServerTransport::reflect(
        const SessionPtr &sessionPtr1,
        const SessionPtr &sessionPtr2)
    {
        IocpSessionState & sessionState1 = 
            dynamic_cast<IocpSessionState &>(sessionPtr1->getProactor());

        IocpSessionState & sessionState2 = 
            dynamic_cast<IocpSessionState &>(sessionPtr2->getProactor());

        return
            reflect(
                sessionState1.shared_from_this(),
                sessionState2.shared_from_this());
    }

    bool IocpServerTransport::reflect(
        const IocpSessionStatePtr &sessionStatePtr1,
        const IocpSessionStatePtr &sessionStatePtr2)
    {
        RCF_ASSERT(sessionStatePtr1.get() && sessionStatePtr2.get())
            (sessionStatePtr1.get())(sessionStatePtr2.get());

        sessionStatePtr1->mReflectionSessionStateWeakPtr =
            IocpSessionStateWeakPtr(sessionStatePtr2);

        sessionStatePtr2->mReflectionSessionStateWeakPtr =
            IocpSessionStateWeakPtr(sessionStatePtr1);

        sessionStatePtr1->mReflected = true;
        sessionStatePtr2->mReflected = true;

        return true;
    }

    // check if a server session is still connected
    bool IocpServerTransport::isConnected(
        const SessionPtr &sessionPtr)
    {
        IocpSessionState & sessionState = 
            dynamic_cast<IocpSessionState &>(sessionPtr->getProactor());

        Lock lock(sessionState.mMutex);
        return 
            !sessionState.mHasBeenClosed 
            && sessionState.implIsConnected();
    }

    // create a server-aware client transport to given endpoint
    ClientTransportAutoPtr IocpServerTransport::createClientTransport(
        const I_Endpoint &endpoint)
    {
        return implCreateClientTransport(endpoint);
    }

    void IocpServerTransport::setSessionManager(
        I_SessionManager &sessionManager)
    {
        mpSessionManager = &sessionManager;
    }

    I_SessionManager &IocpServerTransport::getSessionManager()
    {
        RCF_ASSERT(mpSessionManager);
        return *mpSessionManager;
    }

    bool IocpServerTransport::cycleTransportAndServer(
        RcfServer &server,
        int timeoutMs,
        const volatile bool &stopFlag)
    {
        if (!stopFlag && !mStopFlag)
        {
            cycle(timeoutMs/2, stopFlag);
            server.cycleSessions(timeoutMs/2, stopFlag);
        }
        return stopFlag || mStopFlag;
    }

    void IocpServerTransport::onServiceAdded(RcfServer &server)
    {
        setSessionManager(server);

        WriteLock writeLock( getTaskEntriesMutex() );
        getTaskEntries().clear();

        getTaskEntries().push_back(
            TaskEntry(
                boost::bind(
                    &IocpServerTransport::cycleTransportAndServer,
                    this,
                    boost::ref(server),
                    _1,
                    _2),
                StopFunctor(),
                "RCF Iocp Server"));

        mStopFlag = false;
    }

    void IocpServerTransport::onServiceRemoved(RcfServer &)
    {}

    void IocpServerTransport::onServerStart(RcfServer &)
    {
        if (!mOpen)
        {
            open();
            mOpen = true;
        }
    }

    void IocpServerTransport::onServerStop(RcfServer &)
    {
        if (mOpen)
        {
            mOpen = false;
            close();
            mStopFlag = false;
        }
    }

    void IocpServerTransport::onServerOpen(RcfServer &)
    {
        if (!mOpen)
        {
            open();
            mOpen = true;
        }
    }

    void IocpServerTransport::onServerClose(RcfServer &)
    {
        if (mOpen)
        {
            mOpen = false;
            close();            
        }
    }

    void IocpServerTransport::associateSocket(int fd)
    {
        // This assert is in two lines because it wouldn't compile otherwise,
        // in codewarrior release builds.
        bool ok = mIocpAutoPtr.get() && !mStopFlag;
        RCF_ASSERT(ok);

        mIocpAutoPtr->AssociateSocket(fd, fd);
    }
    
} // namespace RCF
