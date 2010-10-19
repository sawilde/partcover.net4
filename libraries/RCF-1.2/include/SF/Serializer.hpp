
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_SERIALIZER_HPP
#define INCLUDE_SF_SERIALIZER_HPP

#include <boost/mpl/assert.hpp>
#include <boost/mpl/or.hpp>

#include <RCF/Exception.hpp>
#include <RCF/Export.hpp>
#include <RCF/TypeTraits.hpp>

#include <SF/Archive.hpp>
#include <SF/I_Stream.hpp>
#include <SF/SerializeFundamental.hpp>
#include <SF/SfNew.hpp>
#include <SF/Tools.hpp>


namespace boost {
    namespace serialization {
        template<class Base, class Derived>
        const Base & base_object(const Derived & d);
    }
}

#if defined(_MSC_VER) && _MSC_VER < 1310

#include <boost/type_traits.hpp>
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(long double)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(__int64)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(unsigned __int64)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(std::string)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(std::wstring)

#endif

namespace SF {

    // Generic serializer, subclassed by all other serializers.

    class RCF_EXPORT SerializerBase : boost::noncopyable
    {
    private:
        void                invokeRead(Archive &ar);
        void                invokeWrite(Archive &ar);

        // Following are overridden to provide type-specific operations.
        virtual std::string getTypeName() = 0;
        virtual void        newObject(Archive &ar) = 0;
        virtual bool        isDerived() = 0;
        virtual std::string getDerivedTypeName() = 0;
        virtual void        getSerializerPolymorphic(const std::string &derivedTypeName) = 0;
        virtual void        invokeSerializerPolymorphic(SF::Archive &) = 0;
        virtual void        serializeContents(Archive &ar) = 0;
        virtual void        addToInputContext(IStream *, const UInt32 &) = 0;
        virtual bool        queryInputContext(IStream *, const UInt32 &) = 0;
        virtual void        addToOutputContext(OStream *, UInt32 &) = 0;
        virtual bool        queryOutputContext(OStream *, UInt32 &) = 0;
        virtual void        setFromId() = 0;
        virtual void        setToNull() = 0;
        virtual bool        isNull() = 0;
        virtual bool        isNonAtomic() = 0;

    public:
                            SerializerBase();
        virtual             ~SerializerBase();
        void                invoke(Archive &ar);
    };    

    //---------------------------------------------------------------------
    // Type-specific serializers

    // These pragmas concern Serializer<T>::newObject, but needs to be up here, probably because Serializer<T> is a template
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4675 )  // warning C4675: resolved overload was found by argument-dependent lookup
#pragma warning( disable : 4702 )  // warning C4702: unreachable code
#endif

    class I_SerializerPolymorphic;

    template<typename T>
    class Serializer : public SerializerBase
    {
    public:
        Serializer(T ** ppt);

    private:
        typedef ObjectId IdT;
        T **                        ppt;
        I_SerializerPolymorphic *   pf;
        IdT                         id;

        std::string         getTypeName();
        void                newObject(Archive &ar);
        bool                isDerived();
        std::string         getDerivedTypeName();
        void                getSerializerPolymorphic(const std::string &derivedTypeName);
        void                invokeSerializerPolymorphic(SF::Archive &ar);
        void                serializeContents(Archive &ar);
        void                addToInputContext(SF::IStream *stream, const UInt32 &nid);
        bool                queryInputContext(SF::IStream *stream, const UInt32 &nid);
        void                addToOutputContext(SF::OStream *stream, UInt32 &nid);
        bool                queryOutputContext(SF::OStream *stream, UInt32 &nid);
        void                setFromId();
        void                setToNull();
        bool                isNull();
        bool                isNonAtomic();
    };

#ifdef _MSC_VER
#pragma warning( pop )
#endif
    /*
    template<typename T> struct GetIndirection                         { typedef boost::mpl::int_<0> Level; typedef T Base; };
    template<typename T> struct GetIndirection<T *>                    { typedef boost::mpl::int_<1> Level; typedef T Base; };
    template<typename T> struct GetIndirection<T const *>              { typedef boost::mpl::int_<1> Level; typedef T Base; };
    template<typename T> struct GetIndirection<T * const>              { typedef boost::mpl::int_<1> Level; typedef T Base; };
    template<typename T> struct GetIndirection<T const * const>        { typedef boost::mpl::int_<1> Level; typedef T Base; };
    template<typename T> struct GetIndirection<T **>                   { typedef boost::mpl::int_<2> Level; typedef T Base; };
    template<typename T> struct GetIndirection<T **const>              { typedef boost::mpl::int_<2> Level; typedef T Base; };
    template<typename T> struct GetIndirection<T *const*>              { typedef boost::mpl::int_<2> Level; typedef T Base; };
    template<typename T> struct GetIndirection<T const**>              { typedef boost::mpl::int_<2> Level; typedef T Base; };
    template<typename T> struct GetIndirection<T *const*const>         { typedef boost::mpl::int_<2> Level; typedef T Base; };
    template<typename T> struct GetIndirection<T const**const>         { typedef boost::mpl::int_<2> Level; typedef T Base; };
    template<typename T> struct GetIndirection<T const*const*>         { typedef boost::mpl::int_<2> Level; typedef T Base; };
    template<typename T> struct GetIndirection<T const*const*const>    { typedef boost::mpl::int_<2> Level; typedef T Base; };
    */

    template<typename PPT>
    struct GetIndirection
    {
        typedef typename RCF::RemovePointer<PPT>::type    PT;
        typedef typename RCF::IsPointer<PPT>::type        is_single;
        typedef typename RCF::IsPointer<PT>::type         is_double;

        typedef
        typename boost::mpl::if_<
            is_double,
            boost::mpl::int_<2>,
            typename boost::mpl::if_<
                is_single,
                boost::mpl::int_<1>,
                boost::mpl::int_<0>
            >::type
        >::type Level;

        typedef
        typename RCF::RemoveCv<
            typename RCF::RemovePointer<
                typename RCF::RemoveCv<
                    typename RCF::RemovePointer<
                        typename RCF::RemoveCv<PPT>::type
                    >::type
                >::type
            >::type
        >::type Base;
    };

    template<typename T>
    inline void invokeCustomSerializer(
        T **ppt, 
        Archive &ar, 
        RCF_PFTO_HACK int)
    {
        BOOST_MPL_ASSERT(( boost::mpl::not_< RCF::IsPointer<T> > ));
        Serializer<T>(ppt).invoke(ar);
    }

    template<typename U, typename T>
    inline void invokeSerializer(
        U *, 
        T *, 
        boost::mpl::int_<0> *, 
        const U &u, 
        Archive &ar)
    {
        BOOST_MPL_ASSERT(( boost::mpl::not_< RCF::IsPointer<T> > ));
        T *pt = const_cast<T *>(&u);
        invokeCustomSerializer( (T **) (&pt), ar, 0);
    }

    template<typename U, typename T>
    inline void invokeSerializer(
        U *, 
        T *, 
        boost::mpl::int_<1> *, 
        const U &u, 
        Archive &ar)
    {
        BOOST_MPL_ASSERT(( boost::mpl::not_< RCF::IsPointer<T> > ));
        invokeCustomSerializer( (T **) (&u), ar, 0);
    }

    template<typename U, typename T>
    inline void invokeSerializer(
        U *, 
        T *, 
        boost::mpl::int_<2> *, 
        const U &u, 
        Archive &ar)
    {
        BOOST_MPL_ASSERT(( boost::mpl::not_< RCF::IsPointer<T> > ));
        invokeCustomSerializer( (T**) (u), ar, 0);
    }

    template<typename U>
    inline void invokeSerializer(U u, Archive &ar)
    {
        typedef typename GetIndirection<U>::Level Level;
        typedef typename GetIndirection<U>::Base T;
        BOOST_MPL_ASSERT(( boost::mpl::not_< RCF::IsPointer<T> > ));
        //BOOST_MPL_ASSERT(( boost::mpl::not_< boost::is_const<T> > ));
        invokeSerializer( (U *) 0, (T *) 0, (Level *) 0, u, ar);
    }

    template<typename U>
    inline void invokePtrSerializer(U u, Archive &ar)
    {
        typedef typename GetIndirection<U>::Level Level;
        const int levelOfIndirection = Level::value;
        RCF_ASSERT( levelOfIndirection == 1 || levelOfIndirection == 2);
        ar.setFlag( SF::Archive::POINTER, levelOfIndirection == 2 );
        invokeSerializer(u,ar);
    }

    template<typename T>
    inline void serializeEnum(SF::Archive &ar, T &t)
    {
        int rcfRuntimeVersion = ar.getRuntimeVersion();
        if (rcfRuntimeVersion >= 2)
        {
            ar & SF::Archive::Flag(SF::Archive::NO_BEGIN_END);
        }

        if (ar.isRead())
        {
            boost::int32_t n = 0;
            ar & n;
            t = T(n);
        }
        else /* if (ar.isWrite())) */
        {
            boost::int32_t n = t;
            ar & n;
        }
    }

    template<typename T>
    inline void serializeInternal(Archive &archive, T &t)
    {
        // A compiler error here indicates that the class T has not implemented
        // an internal SF serialization function. Here are a few situations in
        // which this can happen:

        // *  No serialization function was provided at all, in which case one
        //    needs to be written (either internal or external).
        // 
        // *  The intention was to provide an external SF serialization function, 
        //    but the external serialization function definition is incorrect and 
        //    hence not visible to the framework.
        
        // *  The intention was to serialize this class through Boost.Serialization,
        //    in which case RCF_USE_BOOST_SERIALIZATION should be defined and 
        //    RCF_USE_SF_SERIALIZATION should be undefined.
        
        // *  The class is a Protocol Buffers generated class, and the intention 
        //    was to serialize it as such, in which case RCF_USE_PROTOBUF 
        //    needs to be defined.

        t.serialize(archive);
    }

    template<typename T>
    inline void serializeFundamentalOrNot(
        Archive &                           archive, 
        T &                                 t, 
        boost::mpl::true_ *)
    {
        serializeFundamental(archive, t);
    }

    template<typename T>
    inline void serializeFundamentalOrNot(
        Archive &                           archive, 
        T &                                 t, 
        boost::mpl::false_ *)
    {
        serializeInternal(archive, t);
    }

    template<typename T>
    inline void serializeEnumOrNot(
        Archive &                           archive, 
        T &                                 t, 
        boost::mpl::true_ *)
    {
        serializeEnum(archive, t);
    }

    template<typename T>
    inline void serializeEnumOrNot(
        Archive &                           archive, 
        T &                                 t, 
        boost::mpl::false_ *)
    {
        typedef typename RCF::IsFundamental<T>::type type;
        serializeFundamentalOrNot(archive, t, (type *) NULL);
    }

    template<typename T>
    inline void serialize(
        Archive &                           archive, 
        T &                                 t)
    {
        typedef typename boost::is_enum<T>::type type;
        serializeEnumOrNot(archive, t, (type *) NULL);
    }

    template<typename T>
    inline void serialize_vc6(
        Archive &                           archive, 
        T &                                 t,
        const unsigned RCF_PFTO_HACK int)
    {
        serialize(archive, t);
    }

    template<typename T>
    inline void preserialize(
        Archive &                           archive, 
        T *&                                pt,
        const unsigned RCF_PFTO_HACK int)
    {
        BOOST_MPL_ASSERT(( boost::mpl::not_< RCF::IsPointer<T> > ));
        typedef typename RCF::RemoveCv<T>::type U;
        serialize_vc6(archive, (U &) *pt, static_cast<const unsigned int>(0) );
    }

    template<typename T> 
    Archive & operator&(
        Archive &                           archive, 
        const T &                           t)
    {
        invokePtrSerializer(&t, archive);
        return archive;
    }

}

#include <SF/Registry.hpp>
#include <SF/SerializePolymorphic.hpp>
#include <SF/Stream.hpp>

namespace SF {

    template<typename T>
    Serializer<T>::Serializer(T **ppt) :
        ppt(ppt),
        pf(RCF_DEFAULT_INIT),
        id()
    {}

    template<typename T>
    std::string Serializer<T>::getTypeName()
    {
        return SF::Registry::getSingleton().getTypeName( (T *) 0);
    }

    template<typename T>
    void Serializer<T>::newObject(Archive &ar)
    {
        *ppt = sfNew((T *) NULL, (T **) NULL, ar);
    }

    template<typename T>
    bool Serializer<T>::isDerived()
    {
        if (*ppt && typeid(T) != typeid(**ppt))
        {
            if (!SF::Registry::getSingleton().isTypeRegistered( typeid(**ppt) ))
            {
                RCF_THROW(RCF::Exception(RCF::_SfError_TypeRegistration(typeid(T).name())))
                    (typeid(T))(typeid(**ppt));
            }
            return true;
        }
        return false;
    }

    template<typename T>
    std::string Serializer<T>::getDerivedTypeName()
    {
        return SF::Registry::getSingleton().getTypeName( typeid(**ppt) );
    }

    template<typename T>
    void Serializer<T>::getSerializerPolymorphic(
        const std::string &derivedTypeName)
    {
        pf = & SF::Registry::getSingleton().getSerializerPolymorphic( 
            (T *) 0, 
            derivedTypeName);
    }

    template<typename T>
    void Serializer<T>::invokeSerializerPolymorphic(SF::Archive &ar)
    {
        RCF_ASSERT(pf);
        void **ppvb = (void **) (ppt); // not even reinterpret_cast wants to touch this
        pf->invoke(ppvb, ar);
    }

    template<typename T>
    void Serializer<T>::serializeContents(Archive &ar)
    {
        preserialize(ar, *ppt, 0);
    }

    template<typename T>
    void Serializer<T>::addToInputContext(SF::IStream *stream, const UInt32 &nid)
    {
        ContextRead &ctx = stream->getContext();
        ctx.add(nid, IdT( (void *) (*ppt), &typeid(T)));
    }

    template<typename T>
    bool Serializer<T>::queryInputContext(SF::IStream *stream, const UInt32 &nid)
    {
        ContextRead &ctx = stream->getContext();
        return ctx.query(nid, id);
    }

    template<typename T>
    void Serializer<T>::addToOutputContext(SF::OStream *stream, UInt32 &nid)
    {
        ContextWrite &ctx = stream->getContext();
        ctx.add( IdT( (void *) *ppt, &typeid(T)), nid);
    }

    template<typename T>
    bool Serializer<T>::queryOutputContext(SF::OStream *stream, UInt32 &nid)
    {
        ContextWrite &ctx = stream->getContext();
        return ctx.query( IdT( (void *) *ppt, &typeid(T)), nid);
    }

    template<typename T>
    void Serializer<T>::setFromId()
    {
        *ppt = reinterpret_cast<T *>(id.first);
    }

    template<typename T>
    void Serializer<T>::setToNull()
    {
        *ppt = NULL;
    }

    template<typename T>
    bool Serializer<T>::isNull()
    {
        return *ppt == NULL;
    }

    template<typename T>
    bool Serializer<T>::isNonAtomic()
    {
        bool isFundamental = RCF::IsFundamental<T>::value;
        return !isFundamental;
    }

} // namespace SF

#endif // ! INCLUDE_SF_SERIALIZER_HPP
