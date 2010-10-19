
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

// NOTE: when using the build system to supply definitions for OUTPUT_DIR_UNQUOTED
// and OPENSSL_CERTIFICATE_DIR_UNQUOTED, the paths should not contain any symbols which
// might be replaced by the preprocessor. Eg if OUTPUT_DIR_UNQUOTED is
//
// \\A\\B\\C\\_MSC_VER\\D\\E
//
// then the resulting OUTPUT_DIR will be "\\A\\B\\C\\0x1310\\D\\E", or something like that.

#ifndef INCLUDE_UTIL_AUTOBUILD_HPP
#define INCLUDE_UTIL_AUTOBUILD_HPP

#include <boost/config.hpp>

#define UTIL_AUTOBUILD_HELPER_MACRO_QUOTE(x) UTIL_AUTOBUILD_HELPER_MACRO_QUOTE_(x)
#define UTIL_AUTOBUILD_HELPER_MACRO_QUOTE_(x) #x

#ifdef BOOST_WINDOWS
#define RCF_PATH_SEPARATOR "\\"
#else
#define RCF_PATH_SEPARATOR "/"
#endif

#ifdef TEMP_DIR_UNQUOTED
#define RCF_TEMP_DIR UTIL_AUTOBUILD_HELPER_MACRO_QUOTE( TEMP_DIR_UNQUOTED ) RCF_PATH_SEPARATOR
#else
#define RCF_TEMP_DIR ""
#endif

#endif //! INCLUDE_UTIL_AUTOBUILD_HPP
