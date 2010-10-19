
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/Protocol/Protocol.hpp>
#include <RCF/SerializationDefs.hpp>
#include <RCF/SerializationProtocol.hpp>

namespace RCF {

#ifdef RCF_USE_SF_SERIALIZATION
    const SerializationProtocol DefaultSerializationProtocol = Sp_SfBinary;
#else
    const SerializationProtocol DefaultSerializationProtocol = Sp_BsBinary;
#endif

#ifdef RCF_USE_PROTOBUF
    const MarshalingProtocol DefaultMarshalingProtocol = Mp_Rcf;
#else
    const MarshalingProtocol DefaultMarshalingProtocol = Mp_Rcf;
#endif

} // namespace RCF
