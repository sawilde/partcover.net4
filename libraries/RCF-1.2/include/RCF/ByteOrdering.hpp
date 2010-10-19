
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_BYTEORDERING_HPP
#define INCLUDE_RCF_BYTEORDERING_HPP

#include <RCF/Export.hpp>

namespace RCF {

    RCF_EXPORT void machineToNetworkOrder(void *buffer, int width, int count);
    RCF_EXPORT void networkToMachineOrder(void *buffer, int width, int count);

} // namespace RCF

#endif // ! INCLUDE_RCF_BYTEORDERING_HPP
