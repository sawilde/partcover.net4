
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_LIST_HPP
#define INCLUDE_SF_LIST_HPP

#include <list>

#include <SF/SerializeStl.hpp>

namespace SF {

    // std::list
    template<typename T, typename A>
    inline void serialize_vc6(SF::Archive &ar, std::list<T,A> &t, const unsigned int)
    {
        serializeStlContainer<PushBackSemantics>(ar, t);
    }

} // namespace SF

#endif // ! INCLUDE_SF_LIST_HPP
