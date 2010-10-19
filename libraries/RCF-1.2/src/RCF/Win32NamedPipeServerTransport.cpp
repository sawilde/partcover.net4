
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/Win32NamedPipeServerTransport.hpp>

#include <RCF/CurrentSession.hpp>
#include <RCF/Win32NamedPipeEndpoint.hpp>
#include <RCF/Win32NamedPipeClientTransport.hpp>

namespace RCF {

    void resetSessionStatePtr(IocpSessionStatePtr &sessionStatePtr);

    // Win32NamedPipeSessionState

    Win32NamedPipeSessionState::Win32NamedPipeSessionState(
        Win32NamedPipeServerTransport & transport,
        HANDLE hPipe) : 
            IocpSessionState(transport),
            mTransport(transport),
            mhPipe(hPipe)
    {
        mSessionPtr = mTransport.mpSessionManager->createSession();
        mSessionPtr->setProactor(*this);        
    }

    Win32NamedPipeSessionState::~Win32NamedPipeSessionState()
    {
        RCF_DTOR_BEGIN

            mEnableReconnect = false;
            postClose();

            // TODO: do this sort of stuff on a separate thread.
            if (!mTransport.mPipeName.empty() && mTransport.mOpen)
            {
                bool needAnotherAccept = false;
                {
                    Lock lock(mTransport.mQueuedAcceptsMutex);
                    --mTransport.mQueuedAccepts;
                    if (mTransport.mQueuedAccepts <= 0)
                    {
                        needAnotherAccept = true;
                    }
                }
                if (needAnotherAccept)
                {
                    mTransport.createSessionState()->accept();
                }
            }

        RCF_DTOR_END
    }

    const I_RemoteAddress & Win32NamedPipeSessionState::getRemoteAddress()
    {
        return mRemoteAddress;
    }

    void Win32NamedPipeSessionState::accept()
    {
        mTransport.registerSession(mWeakThisPtr);

        mThisPtr = mWeakThisPtr.lock();
        mError = 0;

        BOOL ok = ConnectNamedPipe(mhPipe, this);
        DWORD dwErr = GetLastError();

        if (!ok && dwErr != ERROR_IO_PENDING && dwErr != ERROR_PIPE_CONNECTED)
        {
            mError = dwErr;
        }

        // ConnectNamedPipe() can complete either synchronously or
        // asynchronously. We need to cater for both possibilities.

        if ( !ok && dwErr == ERROR_IO_PENDING )
        {
            mError = 0;
            Lock lock(mTransport.mQueuedAcceptsMutex);
            ++mTransport.mQueuedAccepts;
        }
        else if (!ok && dwErr == ERROR_PIPE_CONNECTED)
        {
            mError = 0;
            Lock lock(mTransport.mQueuedAcceptsMutex);
            ++mTransport.mQueuedAccepts;
            mThisPtr.reset();
            onAccept();
        }
        else
        {
            // MSDN says ConectNamedPipe will always return 0, in overlapped mode.
            RCF_ASSERT(!ok);

            mThisPtr.reset();
        }

    }

    void Win32NamedPipeSessionState::implOnAccept()
    {
        // Initiate another accept, if appropriate.
        if (!mTransport.mPipeName.empty())
        {
            bool needAnotherAccept = false;
            {
                Lock lock(mTransport.mQueuedAcceptsMutex);
                --mTransport.mQueuedAccepts;
                if (mTransport.mQueuedAccepts <= 0)
                {
                    needAnotherAccept = true;
                }
            }
            if (needAnotherAccept)
            {
                mTransport.createSessionState()->accept();
            }
        }

        // simulate a completed write to kick things off
        mPreState = IocpSessionState::WritingData;
        mWriteBufferRemaining = 0;
        transition();
    }

    void Win32NamedPipeSessionState::implRead(
        const ByteBuffer &byteBuffer,
        std::size_t bufferLen)
    {
        mPostState = Reading;

        mThisPtr = mWeakThisPtr.lock();

        bool readOk = false;

        {
            Lock lock(mMutex);

            if (!mHasBeenClosed)
            {
                mError = 0;
                DWORD dwBytesRead = 0;

                BOOL ok = ReadFile(
                    mhPipe, 
                    byteBuffer.getPtr(), 
                    static_cast<DWORD>(bufferLen),
                    &dwBytesRead, 
                    this);

                DWORD dwErr = 0;

                if (!ok)
                {
                    dwErr = GetLastError();

                    if (    dwErr != ERROR_IO_PENDING 
                        &&  dwErr != WSA_IO_PENDING 
                        &&  dwErr != ERROR_MORE_DATA)
                    {
                        mError = dwErr;
                    }
                }

                if (    dwBytesRead
                    ||  (ok && dwBytesRead == 0 && bufferLen == 0)
                    ||  (!ok && mError == 0))
                {
                    readOk = true;
                }
            }
        }

        if (!readOk)
        {
            mThisPtr.reset();
            if (mEnableReconnect && mOwnFd)
            {
                reconnect();
            }
        }

    }

    void Win32NamedPipeSessionState::implWrite(
        const std::vector<ByteBuffer> &byteBuffers,
        IocpSessionState * pReflectee)
    {
        Win32NamedPipeSessionState * pXReflectee = 
            static_cast<Win32NamedPipeSessionState *>(pReflectee);

        mPostState = Writing;

        const ByteBuffer & byteBuffer = byteBuffers.front();

        mThisPtr = mWeakThisPtr.lock();

        HANDLE & hPipe = pXReflectee ? pXReflectee->mhPipe : mhPipe ;

        bool writeOk = false;

        {
            Lock lock(mMutex);

            if (!mHasBeenClosed)
            {
                mError = 0;
                DWORD dwBytesWritten = 0;

                BOOL ok = WriteFile(
                    hPipe,
                    byteBuffer.getPtr(),
                    static_cast<DWORD>(byteBuffer.getLength()),
                    &dwBytesWritten,
                    this);

                DWORD dwErr = 0;

                if (!ok)
                {
                    dwErr = GetLastError();

                    if (    dwErr != ERROR_IO_PENDING 
                        &&  dwErr != WSA_IO_PENDING 
                        &&  dwErr != ERROR_MORE_DATA)
                    {
                        mError = dwErr;
                    }
                }

                if (    dwBytesWritten
                    ||  (!ok &&  mError == 0))
                {
                    writeOk = true;
                }
            }
        }

        if (!writeOk)
        {
            mThisPtr.reset();
            if (mEnableReconnect && mOwnFd)
            {
                reconnect();
            }
        }
    }

    void Win32NamedPipeSessionState::implDelayCloseAfterSend()
    {
        // TODO
        // ...
    }

    void Win32NamedPipeSessionState::reconnect()
    {
        RCF_ASSERT(mEnableReconnect && mOwnFd);

        BOOL ok = DisconnectNamedPipe(mhPipe);
        DWORD dwErr = GetLastError();

        RCF_UNUSED_VARIABLE(ok);
        RCF_UNUSED_VARIABLE(dwErr);
        
        mSessionPtr.reset();
        mSessionPtr = mTransport.mpSessionManager->createSession();
        mSessionPtr->setProactor(*this);

        resetState();

        accept();
    }

    void Win32NamedPipeSessionState::implClose()
    {
        RCF_ASSERT(mOwnFd);

        if (mEnableReconnect)
        {
            reconnect();
        }
        else
        {
            BOOL ok = CloseHandle(mhPipe);
            DWORD dwErr = GetLastError();

            RCF_ASSERT(ok)(dwErr);

            mHasBeenClosed = true;
        }
    }

    HANDLE Win32NamedPipeSessionState::getNativeHandle()
    {
        return mhPipe;
    }

    // TODO
    bool Win32NamedPipeSessionState::implIsConnected()
    {
        return true;
    }

    void Win32NamedPipeSessionState::implOnMessageLengthError()
    {
        // Do a hard close on the connection. 

        // It would be  nice to send an informative message back 
        // to the client, as the TCP transports do, but it seems 
        // rather difficult to do so reliably. If we write 
        // something to the pipe, and then call FlushFileBuffers(), 
        // we could potentially block indefinitely.

        if (mEnableReconnect && mOwnFd)
        {
            reconnect();
        }
    }

    ClientTransportAutoPtr Win32NamedPipeSessionState::implCreateClientTransport()
    {
        HANDLE hPipe = INVALID_HANDLE_VALUE;
        {
            Lock lock(mMutex);
            if (mOwnFd && !mHasBeenClosed)
            {
                mOwnFd = false;
                mEnableReconnect = false;
                hPipe = mhPipe;
            }
        }

        std::auto_ptr<Win32NamedPipeClientTransport> w32PipeClientTransport(
            new Win32NamedPipeClientTransport(hPipe));

        w32PipeClientTransport->setNotifyCloseFunctor( boost::bind(
            &IocpSessionState::notifyClose, 
            IocpSessionStateWeakPtr(shared_from_this())));

        w32PipeClientTransport->setPipeName(mRemotePipeName);

        return ClientTransportAutoPtr(w32PipeClientTransport.release());
    }

    // Win32NamedPipeServerTransport

    Win32NamedPipeServerTransport::Win32NamedPipeServerTransport(
        const tstring & pipeName) :
            IocpServerTransport(),
            mpSec(RCF_DEFAULT_INIT),
            mQueuedAccepts(RCF_DEFAULT_INIT),
            mPipeNameLock(INVALID_HANDLE_VALUE)
    {
        if (pipeName.empty())
        {
            // Generate a pipe name dynamically, that won't conflict with any
            // other test executables that may be running concurrently.

            std::pair<tstring, HANDLE> pipeAndHandle = generateNewPipeName();
            mPipeName = pipeAndHandle.first + RCF_T("_0");
            mPipeNameLock = pipeAndHandle.second;
        }
        else
        {
            if (pipeName.at(0) == RCF_T('\\'))
            {
                mPipeName = pipeName;
            }
            else
            {
                mPipeName = RCF_T("\\\\.\\pipe\\") + pipeName;
            }
        }
    }

    Win32NamedPipeServerTransport::~Win32NamedPipeServerTransport()
    {
        if (mPipeNameLock != INVALID_HANDLE_VALUE)
        {
            CloseHandle(mPipeNameLock);
            mPipeNameLock = INVALID_HANDLE_VALUE;
        }
    }

    ServerTransportPtr Win32NamedPipeServerTransport::clone()
    {
        return ServerTransportPtr( new Win32NamedPipeServerTransport(mPipeName) );
    }

    void Win32NamedPipeServerTransport::setSecurityAttributes(LPSECURITY_ATTRIBUTES pSec)
    {
        mpSec = pSec;
    }

    tstring    Win32NamedPipeServerTransport::getPipeName()
    {
        return mPipeName;
    }

    Win32NamedPipeServerTransport::SessionStatePtr 
    Win32NamedPipeServerTransport::createSessionState()
    {
        const std::size_t       MaxPipeInstances    = PIPE_UNLIMITED_INSTANCES;
        const DWORD             OutBufferSize       = 4096;
        const DWORD             InBufferSize        = 4096;
        const DWORD             DefaultTimeoutMs    = 0;

        HANDLE hPipe = CreateNamedPipe( 
            mPipeName.c_str(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            MaxPipeInstances,
            OutBufferSize,
            InBufferSize,
            DefaultTimeoutMs,
            mpSec);

        DWORD dwErr = GetLastError();

        RCF_VERIFY(hPipe != INVALID_HANDLE_VALUE, Exception(_RcfError_Pipe(), dwErr));

        mIocpAutoPtr->AssociateDevice(hPipe, 0);
        
        SessionStatePtr sessionStatePtr(new SessionState(*this, hPipe));

        sessionStatePtr->mWeakThisPtr = sessionStatePtr;

        return sessionStatePtr;
    }

    void Win32NamedPipeServerTransport::implOpen()
    {
        mQueuedAccepts = 0;

        if (!mPipeName.empty())
        {
            createSessionState()->accept();
        }        
    }

    void Win32NamedPipeServerTransport::implClose()
    {
        Lock lock(mSessionsMutex);
        std::set<IocpSessionStateWeakPtr>::iterator iter;
        for (iter = mSessions.begin(); iter != mSessions.end(); ++iter)
        {
            IocpSessionStatePtr iocpSessionStatePtr((*iter).lock());
            if (iocpSessionStatePtr)
            {
                SessionStatePtr sessionStatePtr = 
                    boost::static_pointer_cast<SessionState>(
                        iocpSessionStatePtr);

                sessionStatePtr->mEnableReconnect = false;
            }
        }
    }

    IocpSessionStatePtr Win32NamedPipeServerTransport::implCreateServerSession(
        I_ClientTransport & clientTransport)
    {
        Win32NamedPipeClientTransport & w32PipeClientTransport =
            dynamic_cast<Win32NamedPipeClientTransport &>(clientTransport);

        HANDLE hPipe = w32PipeClientTransport.releaseHandle();
        RCF_ASSERT(hPipe);

        SessionStatePtr sessionStatePtr( new Win32NamedPipeSessionState(*this, hPipe));

        mIocpAutoPtr->AssociateDevice(hPipe, 0);

        sessionStatePtr->mEnableReconnect = false;

        sessionStatePtr->mWeakThisPtr = sessionStatePtr;

        sessionStatePtr->mRemotePipeName = w32PipeClientTransport.getPipeName();

        return IocpSessionStatePtr(sessionStatePtr);
    }

    ClientTransportAutoPtr Win32NamedPipeServerTransport::implCreateClientTransport(
        const I_Endpoint &endpoint)
    {
        const Win32NamedPipeEndpoint & w32PipeEndpoint =
            dynamic_cast<const Win32NamedPipeEndpoint &>(endpoint);

        return w32PipeEndpoint.createClientTransport();
    }

    Win32NamedPipeImpersonator::Win32NamedPipeImpersonator()
    {
        impersonate();
    }

    Win32NamedPipeImpersonator::~Win32NamedPipeImpersonator()
    {
        RCF_DTOR_BEGIN
            revertToSelf();
        RCF_DTOR_END
    }

    void Win32NamedPipeImpersonator::impersonate()
    {
        Win32NamedPipeSessionState & sessionState = 
            dynamic_cast<Win32NamedPipeSessionState &>(
                RCF::getCurrentSessionPtr()->getProactor());


        BOOL ok = ImpersonateNamedPipeClient(sessionState.mhPipe);
        DWORD dwErr = GetLastError();
        RCF_VERIFY(ok, RCF::Exception(_RcfError_Pipe(), dwErr));
    }

    void Win32NamedPipeImpersonator::revertToSelf() const
    {
        BOOL ok = RevertToSelf();
        DWORD dwErr = GetLastError();
        RCF_VERIFY(ok, RCF::Exception(_RcfError_Pipe(), dwErr));
    }

} // namespace RCF
