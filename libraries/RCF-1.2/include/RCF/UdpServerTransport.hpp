
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_UDPSERVERTRANSPORT_HPP
#define INCLUDE_RCF_UDPSERVERTRANSPORT_HPP

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/IpAddress.hpp>
#include <RCF/Service.hpp>
#include <RCF/ServerTransport.hpp>
#include <RCF/IpServerTransport.hpp>
#include <RCF/ThreadLibrary.hpp>

namespace RCF {
   
    class UdpServerTransport;
    class UdpSessionState;

    typedef boost::shared_ptr<UdpServerTransport>   UdpServerTransportPtr;
    typedef boost::shared_ptr<UdpSessionState>      UdpSessionStatePtr;

    class UdpSessionState : public I_Proactor
    {
    public:

        UdpSessionState(UdpServerTransport & transport);

        int                        getNativeHandle() const;

        typedef UdpSessionStatePtr SessionStatePtr;

    private:

        boost::shared_ptr< std::vector<char> >      mReadVecPtr;
        boost::shared_ptr< std::vector<char> >      mWriteVecPtr;
        IpAddress                                   remoteAddress;
        UdpServerTransport &                        mTransport;
        SessionPtr                                  mSessionPtr;

        friend class UdpServerTransport;

    private:

        // I_Proactor
        const I_RemoteAddress & getRemoteAddress() const;
        I_ServerTransport &     getServerTransport();
        const I_RemoteAddress & getRemoteAddress();

        void                    setTransportFilters(
                                    const std::vector<FilterPtr> &filters);

        void                    getTransportFilters(
                                    std::vector<FilterPtr> &filters);

        ByteBuffer              getReadByteBuffer();
        void                    postRead();
        
        void                    postWrite(
                                    std::vector<ByteBuffer> &byteBuffers);

        void                    postClose();        
    };

    typedef UdpSessionState UdpProactor;

    class RCF_EXPORT UdpServerTransport :
        public I_ServerTransport,
        public I_IpServerTransport,
        public I_Service,
        boost::noncopyable
    {
    public:

        UdpServerTransport(int port = 0);

        UdpServerTransport(
            const std::string &        networkInterface, 
            int                        port = 0, 
            const std::string &        multicastIp = "");

        ServerTransportPtr 
                    clone();

        I_SessionManager &
                    getSessionManager();

        void        setSessionManager(I_SessionManager &sessionManager);
        void        setPort(int port);
        int         getPort() const;
        void        open();
        void        close();
        void        cycle(int timeoutMs, const volatile bool &stopFlag);

        bool        cycleTransportAndServer(
                        RcfServer &server,
                        int timeoutMs,
                        const volatile bool &stopFlag);

        UdpServerTransport & enableSharedAddressBinding();

        // I_Service implementation
    private:
        void        onServiceAdded(RcfServer &server);
        void        onServiceRemoved(RcfServer &server);
        void        onServerOpen(RcfServer &server);
        void        onServerClose(RcfServer &server);
        void        onServerStart(RcfServer &server);
        void        onServerStop(RcfServer &server);

    private:

        typedef UdpSessionState SessionState;
        typedef UdpSessionStatePtr SessionStatePtr;

        I_SessionManager *  mpSessionManager;
        int                 mPort;
        int                 mFd;
        volatile bool       mStopFlag;
        unsigned int        mPollingDelayMs;
        std::string         mMulticastIp;
        bool                mEnableSharedAddressBinding;

        friend class UdpSessionState;

    };

} // namespace RCF

#endif // ! INCLUDE_RCF_UDPSERVERTRANSPORT_HPP
