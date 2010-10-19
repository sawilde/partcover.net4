
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_BSAUTOPTR_HPP
#define INCLUDE_RCF_BSAUTOPTR_HPP

#include <memory>

#include <boost/serialization/split_free.hpp>
#include <boost/serialization/nvp.hpp>

// Boost.Serialization code for std::auto_ptr.

namespace boost {
    namespace serialization {

        template<typename Archive, typename T>
        void serialize(Archive &ar, std::auto_ptr<T> &apt, const unsigned int version)
        {
            split_free(ar, apt, version);
        }

        template<typename Archive, typename T>
        void save(Archive &ar, const std::auto_ptr<T> &apt, const unsigned int)
        {
            T *pt = apt.get();
            ar & boost::serialization::make_nvp("Dummy", pt);
        }

        template<typename Archive, typename T>
        void load(Archive &ar, std::auto_ptr<T> &apt, const unsigned int)
        {
            T *pt = NULL;
            ar & boost::serialization::make_nvp("Dummy", pt);
            apt.reset(pt);
        }

    }
}

#endif // ! INCLUDE_RCF_BSAUTOPTR_HPP
