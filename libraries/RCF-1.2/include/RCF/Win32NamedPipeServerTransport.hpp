
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_WIN32NAMEDPIPESERVERTRANSPORT_HPP
#define INCLUDE_RCF_WIN32NAMEDPIPESERVERTRANSPORT_HPP

#include <RCF/Export.hpp>
#include <RCF/IocpServerTransport.hpp>
#include <RCF/ServerTransport.hpp>
#include <RCF/Service.hpp>

#include <RCF/util/Tchar.hpp>
#include "tchar.h"

namespace RCF {

    class Win32NamedPipeSessionState;
    class Win32NamedPipeImpersonator;

    // Win32NamedPipeServerTransport

    class RCF_EXPORT Win32NamedPipeServerTransport : 
        public IocpServerTransport
    {
    public:
        Win32NamedPipeServerTransport(const tstring & pipeName);
        ~Win32NamedPipeServerTransport();

        ServerTransportPtr      clone();

        void                    setSecurityAttributes(LPSECURITY_ATTRIBUTES pSec);

        tstring                 getPipeName();

    private:

        IocpSessionStatePtr     implCreateServerSession(I_ClientTransport & clientTransport);
        ClientTransportAutoPtr  implCreateClientTransport(const I_Endpoint &endpoint);
        void                    implOpen();
        void                    implClose();

        typedef Win32NamedPipeSessionState          SessionState;
        typedef boost::shared_ptr<SessionState>     SessionStatePtr;

        SessionStatePtr         createSessionState();

        tstring                                     mPipeName;
        HANDLE                                      mPipeNameLock;
        LPSECURITY_ATTRIBUTES                       mpSec;

        Mutex                                       mQueuedAcceptsMutex;
        boost::int32_t                              mQueuedAccepts;
        
        friend class Win32NamedPipeSessionState;
    };

    // Win32NamedPipeSessionState

    class RCF_EXPORT Win32NamedPipeSessionState : public IocpSessionState
    {
    public:
        Win32NamedPipeSessionState(
            Win32NamedPipeServerTransport &serverTransport, 
            HANDLE hPipe);

        ~Win32NamedPipeSessionState();

        const I_RemoteAddress & getRemoteAddress();
        HANDLE                  getNativeHandle();

    private:
        void                    implOnAccept();
        
        void                    implRead(
                                    const ByteBuffer &byteBuffer, 
                                    std::size_t bufferLen);

        void                    implWrite(
                                    const std::vector<ByteBuffer> &byteBuffers, 
                                    IocpSessionState * pReflectee = NULL);

        ClientTransportAutoPtr  implCreateClientTransport();

        void                    implDelayCloseAfterSend();
        void                    implClose();
        bool                    implIsConnected();

        virtual
        void                    implOnMessageLengthError();

        void                    accept();
        void                    reconnect();

        Win32NamedPipeServerTransport & mTransport;
        HANDLE                          mhPipe;
        NoRemoteAddress                 mRemoteAddress;
        tstring                         mRemotePipeName;


        friend class Win32NamedPipeServerTransport;
        friend class Win32NamedPipeImpersonator;

        typedef Win32NamedPipeSessionState          SessionState;
        typedef boost::shared_ptr<SessionState>     SessionStatePtr;
    };

    class RCF_EXPORT Win32NamedPipeImpersonator
    {
    public:
        Win32NamedPipeImpersonator();
        ~Win32NamedPipeImpersonator();
        void impersonate();
        void revertToSelf() const;
    };

    typedef Win32NamedPipeSessionState Win32NamedPipeProactor;

} // namespace RCF


#endif // ! INCLUDE_RCF_WIN32NAMEDPIPESERVERTRANSPORT_HPP
