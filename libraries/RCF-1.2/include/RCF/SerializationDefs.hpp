
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_SERIALIZATIONDEFS_HPP
#define INCLUDE_RCF_SERIALIZATIONDEFS_HPP

// NB: Any code that uses the RCF_USE_SF_SERIALIZATION/RCF_USE_BOOST_SERIALIZATION macros,
// needs to include this file first.

#if !defined(RCF_USE_SF_SERIALIZATION) && !defined(RCF_USE_BOOST_SERIALIZATION) && !defined(RCF_USE_BOOST_XML_SERIALIZATION)
#define RCF_USE_SF_SERIALIZATION
#endif

#endif // ! INCLUDE_RCF_SERIALIZATIONDEFS_HPP
