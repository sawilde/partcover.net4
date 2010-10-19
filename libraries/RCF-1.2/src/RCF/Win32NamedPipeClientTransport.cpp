
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/Win32NamedPipeClientTransport.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/Win32NamedPipeEndpoint.hpp>

namespace RCF {

    Win32NamedPipeClientTransport::Win32NamedPipeClientTransport(
        const Win32NamedPipeClientTransport & rhs) :
            ConnectionOrientedClientTransport(rhs),
            mPipeName(rhs.mPipeName),
            mEpPipeName(rhs.mEpPipeName),
            mhPipe(INVALID_HANDLE_VALUE),
            mhEvent(INVALID_HANDLE_VALUE),
            mpSec(RCF_DEFAULT_INIT)
    {
        HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        DWORD dwErr = GetLastError();
        RCF_VERIFY( hEvent, Exception(_RcfError_Pipe(), dwErr));

        mhEvent = hEvent;
        mpSec = rhs.mpSec;
    }

    Win32NamedPipeClientTransport::Win32NamedPipeClientTransport(
        const tstring & pipeName) :
            mEpPipeName(pipeName),
            mhPipe(INVALID_HANDLE_VALUE),
            mhEvent(INVALID_HANDLE_VALUE),
            mpSec(RCF_DEFAULT_INIT)
    {
        if (pipeName.at(0) == RCF_T('\\'))
        {
            mPipeName = pipeName;
        }
        else
        {
            mPipeName = RCF_T("\\\\.\\pipe\\") + pipeName;
        }
        
        HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        DWORD dwErr = GetLastError();
        RCF_VERIFY( hEvent, Exception(_RcfError_Pipe(), dwErr));

        mhEvent = hEvent;
    }

    Win32NamedPipeClientTransport::Win32NamedPipeClientTransport(HANDLE hPipe) :
        mEpPipeName(),
        mhPipe(hPipe),
        mhEvent(),
        mpSec(RCF_DEFAULT_INIT)
    {
        mClosed = false;

        HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        DWORD dwErr = GetLastError();
        RCF_VERIFY( hEvent, Exception(_RcfError_Pipe(), dwErr));

        mhEvent = hEvent;
    }

    Win32NamedPipeClientTransport::~Win32NamedPipeClientTransport()
    {
        RCF_DTOR_BEGIN
            close();

            if (mhEvent != INVALID_HANDLE_VALUE)
            {
                BOOL ok = CloseHandle(mhEvent);
                DWORD dwErr = GetLastError();
                RCF_VERIFY(ok, Exception(_RcfError_Pipe(), dwErr));
            }
        RCF_DTOR_END
    }

    ClientTransportAutoPtr Win32NamedPipeClientTransport::clone() const
    {
        return ClientTransportAutoPtr(new Win32NamedPipeClientTransport(*this));
    }

    HANDLE Win32NamedPipeClientTransport::getNativeHandle() const
    {
        return mhPipe;
    }

    HANDLE Win32NamedPipeClientTransport::releaseHandle()
    {
        HANDLE hPipe = mhPipe;
        mhPipe = INVALID_HANDLE_VALUE;
        return hPipe;
    }

    EndpointPtr Win32NamedPipeClientTransport::getEndpointPtr() const
    {
        return EndpointPtr( new Win32NamedPipeEndpoint(mEpPipeName));
    }

    tstring Win32NamedPipeClientTransport::getPipeName() const
    {
        return mPipeName;
    }

    void Win32NamedPipeClientTransport::setPipeName(const tstring & pipeName)
    {
        mPipeName = pipeName;
    }

    void Win32NamedPipeClientTransport::setSecurityAttributes(
        LPSECURITY_ATTRIBUTES pSec)
    {
        mpSec = pSec;
    }

    void Win32NamedPipeClientTransport::implClose()
    {
        if (mhPipe != INVALID_HANDLE_VALUE)
        {
            BOOL ok = CloseHandle(mhPipe);
            DWORD dwErr = GetLastError();

            mhPipe = INVALID_HANDLE_VALUE;

            RCF_VERIFY(
                ok,
                Exception(
                    _RcfError_Pipe(),
                    dwErr,
                    RcfSubsystem_Os,
                    "CloseHandle() failed"))
                (mhPipe);
        }
    }

    void Win32NamedPipeClientTransport::implConnect(
        I_ClientTransportCallback &clientStub,
        unsigned int timeoutMs)
    {
        unsigned int endTimeMs = getCurrentTimeMs() + timeoutMs;
        HANDLE hPipe = INVALID_HANDLE_VALUE;
        while (hPipe == INVALID_HANDLE_VALUE && generateTimeoutMs(endTimeMs)) 
        { 
            hPipe = CreateFile( 
                mPipeName.c_str(),      // pipe name 
                GENERIC_READ |          // read and write access 
                GENERIC_WRITE, 
                0,                      // no sharing 
                mpSec,                  // default security attributes
                OPEN_EXISTING,          // opens existing pipe 
                FILE_FLAG_OVERLAPPED,   // non-blocking
                NULL);                  // no template file 

            DWORD dwErr = GetLastError();

            if (hPipe == INVALID_HANDLE_VALUE && dwErr == ERROR_PIPE_BUSY) 
            {
                DWORD timeoutMs = 100;
                BOOL ok = WaitNamedPipe(mPipeName.c_str(), timeoutMs);
                DWORD dwErr = GetLastError();
                RCF_UNUSED_VARIABLE(ok);
                RCF_UNUSED_VARIABLE(dwErr);
            }
            else if (hPipe == INVALID_HANDLE_VALUE)
            {
                RCF_THROW(
                    Exception(
                    _RcfError_ClientConnectFail(),
                    dwErr,
                    RcfSubsystem_Os,
                    "CreateFile() failed"));
            }                       
        }
        if (hPipe == INVALID_HANDLE_VALUE)
        {
            RCF_THROW(Exception(_RcfError_ClientConnectTimeout(
                timeoutMs, 
                Win32NamedPipeEndpoint(mPipeName).asString())));
        }
        else
        {
            mhPipe = hPipe;
        }

        clientStub.onConnectCompleted();
    }

    void Win32NamedPipeClientTransport::implConnectAsync(
        I_ClientTransportCallback &clientStub,
        unsigned int timeoutMs)
    {
        // TODO
        // ...

        RCF_UNUSED_VARIABLE(clientStub);
        RCF_UNUSED_VARIABLE(timeoutMs);

        RCF_ASSERT(0);

    }

    bool Win32NamedPipeClientTransport::isConnected()
    {
        bool connected = 
            mhPipe != INVALID_HANDLE_VALUE &&
            PeekNamedPipe(mhPipe, NULL,0, NULL, NULL, NULL) == TRUE;

        if (mhPipe != INVALID_HANDLE_VALUE && !connected)
        {
            // Will probably be ERROR_PIPE_NOT_CONNECTED.
            DWORD dwErr = GetLastError();
            RCF_UNUSED_VARIABLE(dwErr);
        }

        return connected;
    }

    std::size_t Win32NamedPipeClientTransport::implRead(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        std::size_t bytesToRead = RCF_MIN(bytesRequested, byteBuffer.getLength());

        BOOL ok = ResetEvent(mhEvent);
        DWORD dwErr = GetLastError();
        RCF_VERIFY(ok, Exception(_RcfError_Pipe(), dwErr));

        OVERLAPPED overlapped = {0};
        overlapped.hEvent = mhEvent;

        DWORD dwRead = 0;
        DWORD dwBytesToRead = static_cast<DWORD>(bytesToRead);

        ok = ReadFile(
            mhPipe, 
            byteBuffer.getPtr(), 
            dwBytesToRead, 
            &dwRead, 
            &overlapped);
        
        dwErr = GetLastError();

        if (!ok)
        {
            RCF_VERIFY( 
                dwErr == ERROR_IO_PENDING ||
                dwErr == WSA_IO_PENDING ||
                dwErr == ERROR_MORE_DATA,
                Exception(_RcfError_ClientReadFail(), dwErr));
        }

        ClientStub & clientStub = *getCurrentClientStubPtr();

        DWORD dwRet = WAIT_TIMEOUT;
        while (dwRet == WAIT_TIMEOUT)
        {
            boost::uint32_t timeoutMs = generateTimeoutMs(mEndTimeMs);
            timeoutMs = clientStub.generatePollingTimeout(timeoutMs);

            dwRet = WaitForSingleObject(overlapped.hEvent, timeoutMs);
            dwErr = GetLastError();

            RCF_VERIFY( 
                dwRet == WAIT_OBJECT_0 || dwRet == WAIT_TIMEOUT, 
                Exception(_RcfError_Pipe(), dwErr));

            RCF_VERIFY(
                generateTimeoutMs(mEndTimeMs),
                Exception(_RcfError_ClientReadTimeout()))
                (mEndTimeMs)(bytesToRead);

            if (dwRet == WAIT_TIMEOUT)
            {
                clientStub.onPollingTimeout();
            }
        }
        RCF_ASSERT(dwRet == WAIT_OBJECT_0);

        dwRead = 0;
        ok = GetOverlappedResult(mhPipe, &overlapped, &dwRead, FALSE);
        dwErr = GetLastError();
        RCF_VERIFY(ok && dwRead > 0, Exception(_RcfError_Pipe(), dwErr));

        onTimedRecvCompleted(dwRead, 0);

        return dwRead;
    }

    std::size_t Win32NamedPipeClientTransport::implReadAsync(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        // TODO
        // ...

        RCF_UNUSED_VARIABLE(byteBuffer);
        RCF_UNUSED_VARIABLE(bytesRequested);

        RCF_ASSERT(0);

        return 0;
    }

    std::size_t Win32NamedPipeClientTransport::implWrite(
        const std::vector<ByteBuffer> &byteBuffers)
    {

        // Not using overlapped I/O here because it interferes with the
        // server session that might be coupled to this transport.

        const ByteBuffer & byteBuffer = byteBuffers.front();

        DWORD count = 0;
        DWORD dwBytesToWrite = static_cast<DWORD>(byteBuffer.getLength());

        BOOL ok = WriteFile( 
            mhPipe,
            byteBuffer.getPtr(),
            dwBytesToWrite,
            &count,
            NULL);

        DWORD dwErr = GetLastError();

        RCF_VERIFY(ok, Exception(_RcfError_ClientWriteFail(), dwErr));

        RCF_ASSERT(count <= dwBytesToWrite)(count)(dwBytesToWrite)(ok)(dwErr);

        onTimedSendCompleted( RCF_MIN(count, dwBytesToWrite), 0);

        return count;
    }

    std::size_t Win32NamedPipeClientTransport::implWriteAsync(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        // TODO
        // ...

        RCF_UNUSED_VARIABLE(byteBuffers);

        RCF_ASSERT(0);

        return 0;
    }

} // namespace RCF
