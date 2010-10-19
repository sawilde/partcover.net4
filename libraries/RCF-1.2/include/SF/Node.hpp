
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_NODE_HPP
#define INCLUDE_SF_NODE_HPP

#include <boost/noncopyable.hpp>

#include <SF/DataPtr.hpp>
#include <SF/PortableTypes.hpp>

#include <RCF/util/DefaultInit.hpp>

namespace SF {

    //****************************************************************************
    // Node class represents a node in the serialized hierarchy of objects
    // (eg XML streams would translate it to an element in a DOM tree)

    class Node : boost::noncopyable
    {
    public:
        Node() :
            type(),
            label(),
            id(RCF_DEFAULT_INIT),
            ref(RCF_DEFAULT_INIT)
        {}

        Node(
            const DataPtr & type, 
            const DataPtr & label,  
            const UInt32    id, 
            const UInt32    nullPtr) :
                type(type),
                label(label),
                id(id),
                ref(nullPtr)
        {}

        DataPtr type;
        DataPtr label;
        UInt32 id;
        UInt32 ref;
    };

} // namespace SF

#endif // ! INCLUDE_SF_NODE_HPP
