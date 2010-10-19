
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_SHARED_PTR_HPP
#define INCLUDE_SF_SHARED_PTR_HPP

//#include <boost/shared_ptr.hpp>
namespace boost {
    template<class T>
    class shared_ptr;
}

#include <SF/SerializeSmartPtr.hpp>

namespace SF {

    // serialize boost::shared_ptr
    SF_SERIALIZE_REFCOUNTED_SMARTPTR( boost::shared_ptr );

} // namespace SF

#endif // ! INCLUDE_SF_SHARED_PTR_HPP
