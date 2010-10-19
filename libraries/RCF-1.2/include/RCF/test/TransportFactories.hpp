
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TEST_TRANSPORTFACTORIES_HPP
#define INCLUDE_RCF_TEST_TRANSPORTFACTORIES_HPP

#include <iostream>
#include <typeinfo>
#include <utility>
#include <vector>

#include <sys/stat.h>

#include <boost/config.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/version.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/ThreadLibrary.hpp>

#ifdef RCF_USE_BOOST_ASIO
#include <RCF/TcpAsioServerTransport.hpp>
#include <RCF/Asio.hpp>
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
#include <RCF/UnixLocalClientTransport.hpp>
#include <RCF/UnixLocalServerTransport.hpp>
#endif
#endif

#ifdef BOOST_WINDOWS
#include <RCF/TcpIocpServerTransport.hpp>
#include <RCF/Win32NamedPipeClientTransport.hpp>
#include <RCF/Win32NamedPipeEndpoint.hpp>
#include <RCF/Win32NamedPipeServerTransport.hpp>
#endif

#include <RCF/TcpClientTransport.hpp>
#include <RCF/UdpClientTransport.hpp>
#include <RCF/UdpServerTransport.hpp>

#if defined(_MSC_VER) && _MSC_VER <= 1200
#define for if (0) {} else for
#endif

#include <RCF/ObjectFactoryService.hpp>
template<typename Interface>
inline bool tryCreateRemoteObject(
    RCF::I_RcfClient &rcfClient,
    std::string objectName = "")
{
    try
    {
        rcfClient.getClientStub().createRemoteObject(objectName);
        return true;
    }
    catch (const RCF::Exception &e)
    {
        RCF_TRACE("")(e);
        return false;
    }
}

#include <RCF/util/AutoRun.hpp>

#ifdef RCF_USE_BOOST_THREADS
#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>
#endif

namespace Platform {
    namespace OS {

#if defined(RCF_MULTI_THREADED)

        inline void SleepMs(std::size_t msec)
        {

            std::size_t sec = msec / 1000;
            msec = msec % 1000;

            // Sleep for a whole number of seconds.
            if (sec)
            {
                Platform::OS::Sleep( static_cast<unsigned int>(sec) );
            }

            // Sleep the remainder.

#if defined(RCF_USE_BOOST_THREADS)
            using boost::xtime;
            using boost::xtime_get;
            using boost::TIME_UTC;
            using boost::thread;
#else
            using RCF::RcfBoostThreads::boost::xtime;
            using RCF::RcfBoostThreads::boost::xtime_get;
            using RCF::RcfBoostThreads::boost::TIME_UTC;
            using RCF::RcfBoostThreads::boost::thread;
#endif

            xtime xt = {0};
            // 2007-05-29, Test_ClientTimeout hung on the following line, for unknown reasons
            xtime_get(&xt, TIME_UTC);
            xt.nsec += static_cast<xtime::xtime_nsec_t>(msec*1000000);
            thread::sleep(xt);

        }

#elif defined(RCF_SINGLE_THREADED)

        inline void SleepMs(std::size_t msec)
        {
            assert(0 && "SleepMs() not implemented for RCF_SINGLE_THREADED");
            class X {};
            throw X();
        }

#endif

    }
}

namespace RCF {

    typedef boost::shared_ptr<ClientTransportAutoPtr> ClientTransportAutoPtrPtr;

    typedef std::pair<ServerTransportPtr, ClientTransportAutoPtrPtr> TransportPair;

    class I_TransportFactory
    {
    public:
        virtual ~I_TransportFactory() {}
        virtual TransportPair createTransports() = 0;
        virtual TransportPair createNonListeningTransports() = 0;
        virtual bool isConnectionOriented() = 0;
        virtual bool supportsTransportFilters() = 0;
    };

    typedef boost::shared_ptr<I_TransportFactory> TransportFactoryPtr;

    typedef std::vector<TransportFactoryPtr> TransportFactories;

    static TransportFactories &getTransportFactories()
    {
        static TransportFactories transportFactories;
        return transportFactories;
    }

    static TransportFactories &getIpTransportFactories()
    {
        static TransportFactories ipTransportFactories;
        return ipTransportFactories;
    }


    inline void writeTransportTypes(
        std::ostream &os,
        I_ServerTransport &serverTransport,
        I_ClientTransport &clientTransport)
    {
        os
            << "Server transport: " << typeid(serverTransport).name() << ", "
            << "Client transport: " << typeid(clientTransport).name()
            << std::endl;
    }

    //**************************************************
    // transport factories

#ifdef BOOST_WINDOWS

    class TcpIocpTransportFactory : public I_TransportFactory
    {
    public:
        TransportPair createTransports()
        {
            typedef boost::shared_ptr<TcpIocpServerTransport> TcpIocpServerTransportPtr;
            TcpIocpServerTransportPtr tcpServerTransportPtr(
                new TcpIocpServerTransport(0));

            tcpServerTransportPtr->open();
            int port = tcpServerTransportPtr->getPort();

            ClientTransportAutoPtrPtr clientTransportAutoPtrPtr(
                new ClientTransportAutoPtr(
                    new TcpClientTransport("127.0.0.1", port)));

            return std::make_pair(
                ServerTransportPtr(tcpServerTransportPtr), 
                clientTransportAutoPtrPtr);

        }
        TransportPair createNonListeningTransports()
        {
            return std::make_pair(
                ServerTransportPtr( new TcpIocpServerTransport(0) ),
                ClientTransportAutoPtrPtr());
        }
        bool isConnectionOriented()
        {
            return true;
        }
        bool supportsTransportFilters()
        {
            return true;
        }
    };

    AUTO_RUN(
        getTransportFactories().push_back(
            TransportFactoryPtr( new TcpIocpTransportFactory())))

    AUTO_RUN(
        getIpTransportFactories().push_back(
            TransportFactoryPtr( new TcpIocpTransportFactory())))

    class Win32NamedPipeTransportFactory : public I_TransportFactory
    {
    public:
        TransportPair createTransports()
        {
            typedef boost::shared_ptr<Win32NamedPipeServerTransport> Win32NamedPipeServerTransportPtr;
            Win32NamedPipeServerTransportPtr serverTransportPtr(
                new Win32NamedPipeServerTransport(RCF_T("")));

            tstring pipeName = serverTransportPtr->getPipeName();

            ClientTransportAutoPtrPtr clientTransportAutoPtrPtr(
                new ClientTransportAutoPtr(
                    new Win32NamedPipeClientTransport(pipeName)));

            return std::make_pair(
                ServerTransportPtr(serverTransportPtr), 
                clientTransportAutoPtrPtr);

        }

        TransportPair createNonListeningTransports()
        {
            return std::make_pair(
                ServerTransportPtr( new Win32NamedPipeServerTransport( RCF_T("")) ),
                ClientTransportAutoPtrPtr());

        }

        bool isConnectionOriented()
        {
            return true;
        }

        bool supportsTransportFilters()
        {
            return true;
        }
    };

    AUTO_RUN(
        getTransportFactories().push_back(
            TransportFactoryPtr( new Win32NamedPipeTransportFactory())))

#endif

#ifdef RCF_USE_BOOST_ASIO

    class TcpAsioTransportFactory : public I_TransportFactory
    {
    public:
        TransportPair createTransports()
        {
            typedef boost::shared_ptr<TcpAsioServerTransport> TcpAsioServerTransportPtr;
            TcpAsioServerTransportPtr tcpServerTransportPtr(
                new TcpAsioServerTransport(0));

            tcpServerTransportPtr->open();
            int port = tcpServerTransportPtr->getPort();

            ClientTransportAutoPtrPtr clientTransportAutoPtrPtr(
                new ClientTransportAutoPtr(
                    new TcpClientTransport("127.0.0.1", port)));

            return std::make_pair(
                ServerTransportPtr(tcpServerTransportPtr), 
                clientTransportAutoPtrPtr);
        }

        TransportPair createNonListeningTransports()
        {
            return std::make_pair(
                ServerTransportPtr( new TcpAsioServerTransport(0) ),
                ClientTransportAutoPtrPtr());
        }

        bool isConnectionOriented()
        {
            return true;
        }

        bool supportsTransportFilters()
        {
            return true;
        }

    };

    AUTO_RUN(
        getTransportFactories().push_back(
            TransportFactoryPtr( new TcpAsioTransportFactory())))

    AUTO_RUN(
        getIpTransportFactories().push_back(
            TransportFactoryPtr( new TcpAsioTransportFactory())))

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS

    class UnixLocalTransportFactory : public I_TransportFactory
    {
    public:

        UnixLocalTransportFactory() : mIndex(0)
        {
        }

    private:

        TransportPair createTransports()
        {
            std::string pipeName = generateNewPipeName();
            return std::make_pair(
                ServerTransportPtr( new UnixLocalServerTransport(pipeName) ),
                ClientTransportAutoPtrPtr(
                    new ClientTransportAutoPtr(
                        new UnixLocalClientTransport(pipeName))));
        }

        TransportPair createNonListeningTransports()
        {
            return std::make_pair(
                ServerTransportPtr( new UnixLocalServerTransport("") ),
                ClientTransportAutoPtrPtr());
        }

        bool isConnectionOriented()
        {
            return true;
        }

        bool supportsTransportFilters()
        {
            return true;
        }

    private:

        bool fileExists(const std::string & path)
        {
            struct stat stFileInfo = {};
            int ret = stat(path.c_str(), &stFileInfo);
            return ret == 0;
        }

        std::string generateNewPipeName()
        {
            std::string tempDir = RCF_TEMP_DIR;

            std::string candidate;

            while (candidate.empty() || fileExists(candidate))
            {
                std::ostringstream os;
                os 
                    << tempDir 
                    << "/TestPipe_" 
                    << ++mIndex;

                candidate = os.str();
            }

            return candidate;
        }

        int mIndex;

    };

    AUTO_RUN(
        getTransportFactories().push_back(
            TransportFactoryPtr( new UnixLocalTransportFactory())))

#endif // BOOST_ASIO_HAS_LOCAL_SOCKETS

#endif

    class UdpTransportFactory : public I_TransportFactory
    {
    public:
        TransportPair createTransports()
        {
            typedef boost::shared_ptr<UdpServerTransport> UdpServerTransportPtr;
            UdpServerTransportPtr udpServerTransportPtr(
                new UdpServerTransport(0));

            udpServerTransportPtr->open();
            int port = udpServerTransportPtr->getPort();

            ClientTransportAutoPtrPtr clientTransportAutoPtrPtr(
                new ClientTransportAutoPtr(
                    new UdpClientTransport("127.0.0.1", port)));

            return std::make_pair(
                ServerTransportPtr(udpServerTransportPtr), 
                clientTransportAutoPtrPtr);
        }

        TransportPair createNonListeningTransports()
        {
            return std::make_pair(
                ServerTransportPtr( new UdpServerTransport(0) ),
                ClientTransportAutoPtrPtr());
        }

        bool isConnectionOriented()
        {
            return false;
        }

        bool supportsTransportFilters()
        {
            return false;
        }
    };

#ifndef RCF_TEST_NO_UDP
    AUTO_RUN(
        getTransportFactories().push_back(
            TransportFactoryPtr( new UdpTransportFactory())))

    AUTO_RUN(
        getIpTransportFactories().push_back(
            TransportFactoryPtr( new UdpTransportFactory())))
#endif

#if defined(RCF_USE_BOOST_ASIO)

    typedef TcpAsioTransportFactory TcpTransportFactory;

#elif defined(BOOST_WINDOWS)

    typedef TcpIocpTransportFactory TcpTransportFactory;

#endif
    

} // namespace RCF

#endif // ! INCLUDE_RCF_TEST_TRANSPORTFACTORIES_HPP
