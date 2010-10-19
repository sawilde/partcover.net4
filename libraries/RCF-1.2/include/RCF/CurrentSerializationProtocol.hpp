
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_CURRENTSERIALIZATIONPROTOCOL_HPP
#define INCLUDE_RCF_CURRENTSERIALIZATIONPROTOCOL_HPP

#include <cstddef>

#include <RCF/Export.hpp>

namespace RCF {

    class SerializationProtocolIn;
    class SerializationProtocolOut;

    RCF_EXPORT SerializationProtocolIn *getCurrentSerializationProtocolIn();
    RCF_EXPORT SerializationProtocolOut *getCurrentSerializationProtocolOut();

} // namespace RCF

#endif // ! INCLUDE_RCF_CURRENTSERIALIZATIONPROTOCOL_HPP
