
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_PORTABLETYPES_HPP
#define INCLUDE_SF_PORTABLETYPES_HPP

#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>

namespace SF {

    typedef char                                Byte8;
    typedef boost::uint32_t                     UInt32;

    BOOST_STATIC_ASSERT( sizeof(Byte8) == 1 );
    BOOST_STATIC_ASSERT( sizeof(UInt32) == 4 );

} // namespace SF

#endif // ! INCLUDE_SF_PORTABLETYPES_HPP
