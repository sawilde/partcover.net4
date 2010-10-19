
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_GETINTERFACENAME_HPP
#define INCLUDE_RCF_GETINTERFACENAME_HPP

#include <string>

namespace RCF {

    /// Returns the runtime name of the given RCF interface.
    template<typename Interface>
    inline std::string getInterfaceName(Interface * = 0)
    {
        return Interface::getInterfaceName();
    }

} // namespace RCF

#endif // ! INCLUDE_RCF_GETINTERFACENAME_HPP
