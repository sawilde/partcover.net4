
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#define __STDC_CONSTANT_MACROS

#include <boost/config.hpp>
#include <boost/version.hpp>

// Problems with BSer. Include valarray early so it doesn't get trampled by min/max macro definitions.
#if defined(_MSC_VER) && _MSC_VER == 1310 && defined(RCF_USE_BOOST_SERIALIZATION)
#include <valarray>
#endif

// Problems with BSer. Suppress some static warnings.
#if defined(_MSC_VER) && defined(RCF_USE_BOOST_SERIALIZATION) && BOOST_VERSION >= 104100
#pragma warning( push )
#pragma warning( disable : 4308 )  // warning C4308: negative integral constant converted to unsigned type
#endif

#ifndef RCF_CPP_WHICH_SECTION
#define RCF_CPP_WHICH_SECTION 0
#endif

#if RCF_CPP_WHICH_SECTION == 0 || RCF_CPP_WHICH_SECTION == 1

#include "AmiThreadPool.cpp"
#include "AsyncFilter.cpp"
#include "BsdClientTransport.cpp"
#include "ByteBuffer.cpp"
#include "ByteOrdering.cpp"
#include "CheckRtti.cpp"
#include "ClientStub.cpp"
#include "ClientTransport.cpp"
#include "ConnectionOrientedClientTransport.cpp"
#include "CurrentSerializationProtocol.cpp"
#include "CurrentSession.cpp"
#include "Endpoint.cpp"
#include "EndpointBrokerService.cpp"
#include "EndpointServerService.cpp"
#include "Exception.cpp"
#include "FilterService.cpp"

#endif // RCF_CPP_WHICH_SECTION == 1

#if RCF_CPP_WHICH_SECTION == 0 || RCF_CPP_WHICH_SECTION == 2

#include "InitDeinit.cpp"
#include "IpAddress.cpp"
#include "IpClientTransport.cpp"
#include "IpServerTransport.cpp"
#include "Marshal.cpp"
#include "MethodInvocation.cpp"
#include "MethodInvocationProto.cpp"
#include "MulticastClientTransport.cpp"
#include "ObjectFactoryService.cpp"
#include "ObjectPool.cpp"
#include "PerformanceData.cpp"
#include "PingBackService.cpp"
#include "PublishingService.cpp"
#include "Random.cpp"
#include "RcfClient.cpp"
#include "RcfServer.cpp"
#include "RcfSession.cpp"
#include "SerializationProtocol.cpp"
#include "ServerInterfaces.cpp"
#include "ServerStub.cpp"
#include "ServerTask.cpp"
#include "ServerTransport.cpp"
#include "Service.cpp"
#include "SessionObjectFactoryService.cpp"
#include "SessionTimeoutService.cpp"
#include "StubEntry.cpp"
#include "StubFactory.cpp"
#include "SubscriptionService.cpp"
#include "TcpClientTransport.cpp"
#include "TcpEndpoint.cpp"

#endif // RCF_CPP_WHICH_SECTION == 2

#if RCF_CPP_WHICH_SECTION == 0 || RCF_CPP_WHICH_SECTION == 3

#include "ThreadLibrary.cpp"
#include "ThreadLocalData.cpp"
#include "ThreadLocalCache.cpp"
#include "ThreadManager.cpp"
#include "TimedBsdSockets.cpp"
#include "Token.cpp"
#include "Tools.cpp"
#include "UdpClientTransport.cpp"
#include "UdpEndpoint.cpp"
#include "UdpServerTransport.cpp"
#include "UsingBsdSockets.cpp"
#include "Version.cpp"

#include "Protocol/Protocol.cpp"

#include "util/Trace.cpp"
#include "util/Platform.cpp"

#ifdef RCF_USE_BOOST_ASIO
#include "AsioServerTransport.cpp"
#include "TcpAsioServerTransport.cpp"
#endif

#ifdef RCF_USE_BOOST_ASIO
#include <RCF/Asio.hpp>
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
#include "UnixLocalServerTransport.cpp"
#include "UnixLocalClientTransport.cpp"
#include "UnixLocalEndpoint.cpp"
#endif
#endif

#ifdef BOOST_WINDOWS
#include "NamedPipeEndpoint.cpp"
#elif defined(RCF_USE_BOOST_ASIO)
#include <RCF/Asio.hpp>
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
#include "NamedPipeEndpoint.cpp"
#endif
#endif

#ifdef BOOST_WINDOWS
#include "IocpServerTransport.cpp"
#include "TcpIocpServerTransport.cpp"
#include "Schannel.cpp"
#include "SspiFilter.cpp"
#include "Win32NamedPipeClientTransport.cpp"
#include "Win32NamedPipeEndpoint.cpp"
#include "Win32NamedPipeServerTransport.cpp"
#endif

#ifdef RCF_USE_OPENSSL
#include "OpenSslEncryptionFilter.cpp"
#include "UsingOpenSsl.cpp"
#endif

#ifdef RCF_USE_ZLIB
#include "ZlibCompressionFilter.cpp"
#endif

#ifdef RCF_USE_SF_SERIALIZATION
#include "../SF/SF.cpp"
#else
#include "../SF/Encoding.cpp"
#endif

#ifndef RCF_USE_BOOST_THREADS
#include "RcfBoostThreads/RcfBoostThreads.cpp"
#endif

#ifdef RCF_USE_BOOST_FILESYSTEM
#include "FileTransferService.cpp"
#endif

#ifdef RCF_USE_PROTOBUF
#include <RCF\protobuf\RcfMessages.pb.cc>
#endif

#endif // RCF_CPP_WHICH_SECTION == 3

#if defined(_MSC_VER) && defined(RCF_USE_BOOST_SERIALIZATION) && BOOST_VERSION >= 104100
#pragma warning( pop )
#endif
