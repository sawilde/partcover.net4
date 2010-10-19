
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TEST_PRINTTESTHEADER
#define INCLUDE_RCF_TEST_PRINTTESTHEADER

#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include <RCF/util/AutoBuild.hpp>
#include <boost/version.hpp>

#ifdef RCF_USE_BOOST_ASIO
#include <boost/asio/version.hpp>
#include <RCF/Asio.hpp>
#endif

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )  // warning C4996: '' was declared deprecated
#endif

inline void printTestHeader(const char *file)
{
    std::cout << "\n*********************\n";
    std::cout << file << std::endl;
    time_t now = time(NULL);
    std::cout << "Time now: " << std::string(ctime(&now));
    std::cout << "Defines:" << std::endl;

    std::cout << "BOOST_VERSION: " << BOOST_VERSION << std::endl;

#ifdef RCF_USE_BOOST_ASIO
    std::cout << "BOOST_ASIO_VERSION: " << BOOST_ASIO_VERSION << std::endl;
#endif

#ifdef RCF_USE_BOOST_ASIO
#if defined(BOOST_ASIO_HAS_IOCP)
    std::cout << "BOOST_ASIO_HAS_IOCP" << std::endl;
#elif defined(BOOST_ASIO_HAS_EPOLL)
    std::cout << "BOOST_ASIO_HAS_EPOLL" << std::endl;
#elif defined(BOOST_ASIO_HAS_KQUEUE)
    std::cout << "BOOST_ASIO_HAS_KQUEUE" << std::endl;
#elif defined(BOOST_ASIO_HAS_DEV_POLL)
    std::cout << "BOOST_ASIO_HAS_DEV_POLL" << std::endl;
#else
    std::cout << "Boost.Asio - using select()" << std::endl;
#endif
#endif

#ifdef RCF_MULTI_THREADED
    std::cout << "RCF_MULTI_THREADED" << std::endl;
#endif

#ifdef RCF_SINGLE_THREADED
    std::cout << "RCF_SINGLE_THREADED" << std::endl;
#endif

#ifdef RCF_USE_BOOST_THREADS
    std::cout << "RCF_USE_BOOST_THREADS" << std::endl;
#endif

#ifdef RCF_USE_BOOST_ASIO
    std::cout << "RCF_USE_BOOST_ASIO" << std::endl;
#endif

#ifdef RCF_USE_SF_SERIALIZATION
    std::cout << "RCF_USE_SF_SERIALIZATION" << std::endl;
#endif

#ifdef RCF_USE_BOOST_SERIALIZATION
    std::cout << "RCF_USE_BOOST_SERIALIZATION" << std::endl;
#endif

#ifdef RCF_USE_BOOST_XML_SERIALIZATION
    std::cout << "RCF_USE_BOOST_XML_SERIALIZATION" << std::endl;
#endif

#ifdef RCF_USE_ZLIB
    std::cout << "RCF_USE_ZLIB" << std::endl;
#endif

#ifdef RCF_USE_OPENSSL
    std::cout << "RCF_USE_OPENSSL" << std::endl;
#endif

#ifdef BOOST_SP_ENABLE_DEBUG_HOOKS
    std::cout << "BOOST_SP_ENABLE_DEBUG_HOOKS" << std::endl;
#endif

    std::cout << "RCF_TEMP_DIR: " << RCF_TEMP_DIR << std::endl;
    
    std::cout << "*********************\n\n";
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif


#endif // ! INCLUDE_RCF_TEST_PRINTTESTHEADER
