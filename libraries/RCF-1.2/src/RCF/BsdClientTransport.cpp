
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/BsdClientTransport.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/Exception.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/TimedBsdSockets.hpp>

#ifdef BOOST_WINDOWS
#include <RCF/IocpServerTransport.hpp>
#endif

namespace RCF {

    BsdClientTransport::BsdClientTransport() :
        mFd(-1)
    {}

    BsdClientTransport::BsdClientTransport(int fd) :
        mFd(fd)
    {
        mClosed = false;
    }

    BsdClientTransport::BsdClientTransport(const BsdClientTransport & rhs) :
        ConnectionOrientedClientTransport(rhs),
        mFd(-1)
    {}

    BsdClientTransport::~BsdClientTransport()
    {
        RCF_DTOR_BEGIN
            close();
        RCF_DTOR_END
    }

#ifdef BOOST_WINDOWS

    // return -2 for timeout, -1 for error, 0 for ready
    int pollSocketWithProgressMwfmo(
        const ClientProgressPtr &clientProgressPtr,
        ClientProgress::Activity activity,
        unsigned int endTimeMs,
        int fd,
        int &err,
        bool bRead)
    {
        RCF_UNUSED_VARIABLE(err);
        RCF_UNUSED_VARIABLE(activity);

        ClientStub & clientStub = *getCurrentClientStubPtr();

        int uiMessageFilter = clientProgressPtr->mUiMessageFilter;

        HANDLE readEvent = WSACreateEvent();
        using namespace boost::multi_index::detail;
        scope_guard WSACloseEventGuard = make_guard(WSACloseEvent, readEvent);
        RCF_UNUSED_VARIABLE(WSACloseEventGuard);

        int nRet = WSAEventSelect(fd, readEvent, bRead ? FD_READ : FD_WRITE);
        RCF_ASSERT(nRet == 0)(nRet);
        HANDLE handles[] = { readEvent };

        while (true)
        {
            unsigned int timeoutMs = generateTimeoutMs(endTimeMs);
            timeoutMs = clientStub.generatePollingTimeout(timeoutMs);

            if (timeoutMs == 0)
            {
                return -2;
            }

            DWORD dwRet = MsgWaitForMultipleObjects(
                1, 
                handles, 
                0, 
                timeoutMs, 
                uiMessageFilter);

            if (dwRet == WAIT_TIMEOUT)
            {
                clientStub.onPollingTimeout();

                if (generateTimeoutMs(endTimeMs) != 0)
                {
                    continue;
                }
            }
            else if (dwRet == WAIT_OBJECT_0)
            {
                // File descriptor is ready to be read.
                return 0;
            }
            else if (dwRet == WAIT_OBJECT_0 + 1)
            {
                clientStub.onUiMessage();                
            }
        }
    }

#endif

    PollingFunctor::PollingFunctor(
        const ClientProgressPtr &clientProgressPtr,
        ClientProgress::Activity activity,
        unsigned int endTimeMs) :
            mClientProgressPtr(clientProgressPtr),
            mActivity(activity),
            mEndTimeMs(endTimeMs)
    {}

#ifdef BOOST_WINDOWS

    // On Windows, the user may have requested callbacks on UI messages, in 
    // which case we'll need to use MsgWaitForMultipleObjects() rather than
    // plain old select().

    int PollingFunctor::operator()(int fd, int &err, bool bRead)
    {
        if (
            mClientProgressPtr.get() &&
            mClientProgressPtr->mTriggerMask & ClientProgress::UiMessage)
        {
            return pollSocketWithProgressMwfmo(
                mClientProgressPtr,
                mActivity,
                mEndTimeMs,
                fd,
                err,
                bRead);
        }
        else
        {
            return pollSocket(
                mEndTimeMs,
                fd,
                err,
                bRead);
        }
    }

#else

    int PollingFunctor::operator()(int fd, int &err, bool bRead)
    {
        return pollSocket(
            mEndTimeMs,
            fd,
            err,
            bRead);
    }

#endif

    std::size_t BsdClientTransport::implRead(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        std::size_t bytesToRead = RCF_MIN(bytesRequested, byteBuffer.getLength());

        PollingFunctor pollingFunctor(
            mClientProgressPtr,
            ClientProgress::Receive,
            mEndTimeMs);

        RCF3_TRACE("calling recv()")(byteBuffer.getLength())(bytesToRead);

        int err = 0;
        int ret = RCF::timedRecv(
            *this,
            pollingFunctor,
            err,
            mFd,
            byteBuffer,
            bytesToRead,
            0);

        switch (ret)
        {
        case -2:

            RCF_THROW(
                Exception(_RcfError_ClientReadTimeout()))
                (bytesToRead);

            break;

        case -1:
            
            RCF_THROW(
                Exception(
                    _RcfError_ClientReadFail(),
                    err,
                    RcfSubsystem_Os))
                (bytesToRead)(err);

            break;

        case  0:
            
            RCF_THROW(
                Exception(_RcfError_PeerDisconnect()))
                (bytesToRead);

            break;

        default:
            
            RCF_ASSERT(
                0 < ret && ret <= static_cast<int>(bytesRequested))
                (ret)(bytesRequested);
        }

        return ret;
    }

#ifdef BOOST_WINDOWS

    std::size_t BsdClientTransport::implReadAsync(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        WSABUF wsabuf = {0};
        wsabuf.buf = byteBuffer.getPtr();
        wsabuf.len = static_cast<u_long>(bytesRequested);

        OverlappedAmiPtr overlappedPtr = getOverlappedPtr();

        RCF_ASSERT(!overlappedPtr->mThisPtr);
        overlappedPtr->mThisPtr = overlappedPtr;
        RCF_ASSERT(overlappedPtr->mThisPtr);

        using namespace boost::multi_index::detail;
        scope_guard clearSelfReferenceGuard =
            make_guard(clearSelfReference, boost::ref(overlappedPtr->mThisPtr));

        DWORD dwReceived = 0;
        DWORD dwFlags = 0;
        int ret = WSARecv(
            mFd, 
            &wsabuf, 
            1, 
            &dwReceived, 
            &dwFlags, 
            overlappedPtr.get(), 
            NULL);
        int err = WSAGetLastError();

        RCF_ASSERT(ret == -1 || ret == 0);
        if (err == S_OK || err == WSA_IO_PENDING)
        {
            clearSelfReferenceGuard.dismiss();
        }
        else
        {
            RCF::Exception e(_RcfError_Socket(), err);
            mClientStubCallbackPtr.onError(e);
        }

        // TODO: error handling. Sync or async?
        // ...

        return 0;

    }

#else

    std::size_t BsdClientTransport::implReadAsync(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        RCF_ASSERT(0);
        return 0;
    }

#endif


    std::size_t BsdClientTransport::implWrite(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        PollingFunctor pollingFunctor(
            mClientProgressPtr,
            ClientProgress::Send,
            mEndTimeMs);

        int err = 0;

        RCF3_TRACE("calling send()")(lengthByteBuffers(byteBuffers));

        int ret = RCF::timedSend(
            pollingFunctor,
            err,
            mFd,
            byteBuffers,
            getMaxSendSize(),
            0);

        switch (ret)
        {
        case -2:

            RCF_THROW(
                Exception(_RcfError_ClientWriteTimeout()));

            break;

        case -1:
            
            RCF_THROW(
                Exception(
                    _RcfError_ClientWriteFail(),
                    err,
                    RcfSubsystem_Os))
                (err);

            break;

        case 0:
            
            RCF_THROW(
                Exception(_RcfError_PeerDisconnect()));
            
            break;

        default:
            
            RCF_ASSERT(
                0 < ret && ret <= static_cast<int>(lengthByteBuffers(byteBuffers)))
                (ret)(lengthByteBuffers(byteBuffers));            

            onTimedSendCompleted(ret, 0);
        }

        return ret;
    }

#ifdef BOOST_WINDOWS

    std::size_t BsdClientTransport::implWriteAsync(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        // Some WSA magic

        if (!mRegisteredForAmi)
        {
            RcfServer * preferred = mClientStubCallbackPtr.getAsyncDispatcher();
            if (preferred)
            {
                I_ServerTransport & transport = preferred->getServerTransport();
                IocpServerTransport & iocpTransport = dynamic_cast<IocpServerTransport &>(transport);
                iocpTransport.associateSocket(mFd);
            }
            else
            {
                gAmiThreadPoolPtr->mIocp.AssociateSocket(mFd, mFd);
            }

            mRegisteredForAmi = true;
        }

        std::vector<WSABUF> wsabufs;
        for (std::size_t i=0; i<byteBuffers.size(); ++i)
        {
            WSABUF wsabuf = {0};
            wsabuf.buf = byteBuffers[i].getPtr();
            wsabuf.len = static_cast<u_long>(byteBuffers[i].getLength());
            wsabufs.push_back(wsabuf);
        }

        OverlappedAmiPtr overlappedPtr = getOverlappedPtr();

        RCF_ASSERT(!overlappedPtr->mThisPtr);
        overlappedPtr->mThisPtr = overlappedPtr;
        RCF_ASSERT(overlappedPtr->mThisPtr);

        using namespace boost::multi_index::detail;
        scope_guard clearSelfReferenceGuard =
            make_guard(clearSelfReference, boost::ref(overlappedPtr->mThisPtr));

        DWORD dwSent = 0;
        DWORD dwFlags = 0;
        int ret = WSASend(
            mFd,
            &wsabufs[0],
            static_cast<DWORD>(wsabufs.size()),
            &dwSent,
            dwFlags,
            overlappedPtr.get(),
            NULL);
        int err = WSAGetLastError();

        RCF_ASSERT(ret == -1 || ret == 0);
        if (err == S_OK || err == WSA_IO_PENDING)
        {
            clearSelfReferenceGuard.dismiss();
        }
        else
        {
            RCF::Exception e(_RcfError_Socket(), err);
            mClientStubCallbackPtr.onError(e);
        }

        // TODO: error handling. Sync or async?
        // ...

        return 0;
    }

#else

    std::size_t BsdClientTransport::implWriteAsync(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        RCF_ASSERT(0);
        return 0;
    }

#endif

    bool BsdClientTransport::isConnected()
    {
        return isFdConnected(mFd);
    }

    int BsdClientTransport::releaseFd()
    {
        // We've got problems if there is a close functor involved.
        RCF_ASSERT( !mNotifyCloseFunctor );

        int myFd = mFd;
        mFd = -1;
        return myFd;
    }

    int BsdClientTransport::getFd() const
    {
        return mFd;
    }

    int    BsdClientTransport::getNativeHandle() const
    {
        return mFd;
    }
    
} // namespace RCF
