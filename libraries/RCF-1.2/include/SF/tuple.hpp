
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_TUPLE_HPP
#define INCLUDE_SF_TUPLE_HPP

#include <boost/tuple/tuple.hpp>

namespace SF {

    class Archive;

    template<
        typename T0>
    void serialize(SF::Archive &ar, boost::tuples::tuple<T0> &t)
    {
        ar 
            & t.template get<0>();
    }

    template<
        typename T0, 
        typename T1>
    void serialize(SF::Archive &ar, boost::tuples::tuple<T0,T1> &t)
    {
        ar 
            & t.template get<0>() 
            & t.template get<1>();
    }

    template<
        typename T0, 
        typename T1, 
        typename T2>
    void serialize(SF::Archive &ar, boost::tuples::tuple<T0,T1,T2> &t)
    {
        ar 
            & t.template get<0>() 
            & t.template get<1>() 
            & t.template get<2>();
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3>
    void serialize(SF::Archive &ar, boost::tuples::tuple<T0,T1,T2,T3> &t)
    {
        ar 
            & t.template get<0>() 
            & t.template get<1>() 
            & t.template get<2>() 
            & t.template get<3>();
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3, 
        typename T4>
    void serialize(SF::Archive &ar, boost::tuples::tuple<T0,T1,T2,T3,T4> &t)
    {
        ar 
            & t.template get<0>() 
            & t.template get<1>() 
            & t.template get<2>() 
            & t.template get<3>() 
            & t.template get<4>();
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3, 
        typename T4, 
        typename T5>
    void serialize(SF::Archive &ar, boost::tuples::tuple<T0,T1,T2,T3,T4,T5> &t)
    {
        ar 
            & t.template get<0>() 
            & t.template get<1>() 
            & t.template get<2>() 
            & t.template get<3>() 
            & t.template get<4>() 
            & t.template get<5>();
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3, 
        typename T4, 
        typename T5, 
        typename T6>
    void serialize(SF::Archive &ar, boost::tuples::tuple<T0,T1,T2,T3,T4,T5,T6> &t)
    {
        ar 
            & t.template get<0>() 
            & t.template get<1>() 
            & t.template get<2>() 
            & t.template get<3>() 
            & t.template get<4>() 
            & t.template get<5>() 
            & t.template get<6>();
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3, 
        typename T4, 
        typename T5, 
        typename T6, 
        typename T7>
    void serialize(SF::Archive &ar, boost::tuples::tuple<T0,T1,T2,T3,T4,T5,T6,T7> &t)
    {
        ar 
            & t.template get<0>() 
            & t.template get<1>() 
            & t.template get<2>() 
            & t.template get<3>() 
            & t.template get<4>() 
            & t.template get<5>() 
            & t.template get<6>() 
            & t.template get<7>();
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3, 
        typename T4, 
        typename T5, 
        typename T6, 
        typename T7, 
        typename T8>
    void serialize(SF::Archive &ar, boost::tuples::tuple<T0,T1,T2,T3,T4,T5,T6,T7,T8> &t)
    {
        ar 
            & t.template get<0>() 
            & t.template get<1>() 
            & t.template get<2>() 
            & t.template get<3>() 
            & t.template get<4>() 
            & t.template get<5>() 
            & t.template get<6>() 
            & t.template get<7>() 
            & t.template get<8>();
    }

    template<
        typename T0, 
        typename T1, 
        typename T2, 
        typename T3, 
        typename T4, 
        typename T5, 
        typename T6, 
        typename T7, 
        typename T8, 
        typename T9>
    void serialize(SF::Archive &ar, boost::tuples::tuple<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> &t)
    {
        ar 
            & t.template get<0>() 
            & t.template get<1>() 
            & t.template get<2>() 
            & t.template get<3>() 
            & t.template get<4>() 
            & t.template get<5>() 
            & t.template get<6>() 
            & t.template get<7>() 
            & t.template get<8>() 
            & t.template get<9>();
    }

}

#endif // ! INCLUDE_SF_TUPLE_HPP
