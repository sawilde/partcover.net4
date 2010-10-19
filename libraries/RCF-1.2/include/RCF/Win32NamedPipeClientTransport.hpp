
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_WIN32NAMEDPIPECLIENTTRANSPORT_HPP
#define INCLUDE_RCF_WIN32NAMEDPIPECLIENTTRANSPORT_HPP

#include <RCF/ConnectionOrientedClientTransport.hpp>

#include <RCF/util/Tchar.hpp>

namespace RCF {

    class RCF_EXPORT Win32NamedPipeClientTransport :
        public ConnectionOrientedClientTransport
    {
    public:
        Win32NamedPipeClientTransport(
            const Win32NamedPipeClientTransport & rhs);

        Win32NamedPipeClientTransport(
            const tstring & pipeName);

        Win32NamedPipeClientTransport(HANDLE hPipe);

        ~Win32NamedPipeClientTransport();

        ClientTransportAutoPtr clone() const;

        HANDLE getNativeHandle() const;

        HANDLE releaseHandle();

        void setDisconnectBeforeClosing(bool disconnectBeforeClosing);

        tstring getPipeName() const;
        void setPipeName(const tstring & pipeName);

        void setSecurityAttributes(LPSECURITY_ATTRIBUTES pSec);

    private:

        std::size_t implRead(
            const ByteBuffer &byteBuffer,
            std::size_t bytesRequested);

        std::size_t implReadAsync(
            const ByteBuffer &byteBuffer,
            std::size_t bytesRequested);

        std::size_t implWrite(
            const std::vector<ByteBuffer> &byteBuffers);

        std::size_t implWriteAsync(
            const std::vector<ByteBuffer> &byteBuffers);

        void implClose();

        void implConnect(
            I_ClientTransportCallback &clientStub,
            unsigned int timeoutMs);

        void implConnectAsync(
            I_ClientTransportCallback &clientStub,
            unsigned int timeoutMs);

        // I_ClientTransport
        EndpointPtr             getEndpointPtr() const;
        bool                    isConnected();

    private:

        tstring                 mEpPipeName;
        tstring                 mPipeName;
        HANDLE                  mhPipe;
        HANDLE                  mhEvent;

        LPSECURITY_ATTRIBUTES   mpSec;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_WIN32NAMEDPIPECLIENTTRANSPORT_HPP
