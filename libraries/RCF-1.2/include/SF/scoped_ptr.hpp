
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_SCOPED_PTR_HPP
#define INCLUDE_SF_SCOPED_PTR_HPP

//#include <boost/scoped_ptr.hpp>
namespace boost {
    template<class T>
    class scoped_ptr;
}

#include <SF/SerializeSmartPtr.hpp>

namespace SF {

    // boost::scoped_ptr
    SF_SERIALIZE_SIMPLE_SMARTPTR( boost::scoped_ptr );

}

#endif // ! INCLUDE_SF_SCOPED_PTR_HPP
