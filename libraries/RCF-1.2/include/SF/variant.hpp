
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_VARIANT_HPP
#define INCLUDE_SF_VARIANT_HPP

#include <boost/variant.hpp>

#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/empty.hpp>

namespace SF {

    class Archive;

    class VariantSerializer : public boost::static_visitor<>
    {
    public:
        VariantSerializer(SF::Archive &ar) : mAr(ar)
        {}

        template<typename T>
        void operator()(const T &t) const
        {
            mAr & t;
        }

    private:
        SF::Archive &mAr;
    };

    template<class S>
    struct VariantDeserializer
    {
        struct LoadNull 
        {
            template<class V>
            static void invoke(
                SF::Archive &,
                int,
                V &)
            {}
        };

        struct Load 
        {
            template<class V>
            static void invoke(
                SF::Archive &       ar,
                int                 which,
                V &                 v)
            {
                using namespace boost::mpl;
                if (which == 0)
                {
                    typedef BOOST_DEDUCED_TYPENAME front<S>::type head_type;
                    head_type value;
                    ar & value;
                    v = value;
                }
                else
                {
                    typedef BOOST_DEDUCED_TYPENAME pop_front<S>::type type;
                    VariantDeserializer<type>::load(ar, which - 1, v);
                }
            }
        };

        template<class V>
        static void load(
            SF::Archive &           ar,
            int                     which,
            V &                     v)
        {
            using namespace boost::mpl;

            typedef BOOST_DEDUCED_TYPENAME eval_if<empty<S>,
                boost::mpl::identity<LoadNull>,
                boost::mpl::identity<Load>
            >::type typex;

            typex::invoke(ar, which, v);
        }

    };

    template< BOOST_VARIANT_ENUM_PARAMS(class T) >
    void serialize_vc6(
        SF::Archive &                                       ar, 
        boost::variant< BOOST_VARIANT_ENUM_PARAMS(T) > &    v,
        const unsigned int)
    {
        if (ar.isWrite())
        {
            ar & int(v.which());
            VariantSerializer variantSerializer(ar);
            v.apply_visitor(variantSerializer);
        }
        else
        {
            int which = 0;
            ar & which;
            
            typedef BOOST_DEDUCED_TYPENAME 
                boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>::types types;

            if(which >=  boost::mpl::size<types>::value)
            {
                RCF_THROW(
                    RCF::Exception(
                        RCF::_RcfError_VariantDeserialization(which, boost::mpl::size<types>::value), 
                        "boost::variant mismatch"))
                        (which)(boost::mpl::size<types>::value);
            }

            VariantDeserializer<types>::load(ar, which, v);
        }
    }
} // namespace SF

#endif // ! INCLUDE_SF_VARIANT_HPP
