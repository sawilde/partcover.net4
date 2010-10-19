
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_ANY_HPP
#define INCLUDE_SF_ANY_HPP

#include <boost/any.hpp>

#include <RCF/Export.hpp>

namespace SF {

    class Archive;

    RCF_EXPORT void serialize(SF::Archive &ar, boost::any &a);

} // namespace SF

#endif // ! INCLUDE_SF_ANY_HPP
