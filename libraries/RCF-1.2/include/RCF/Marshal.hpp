
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_MARSHAL_HPP
#define INCLUDE_RCF_MARSHAL_HPP

#include <RCF/ClientStub.hpp>
#include <RCF/CurrentSerializationProtocol.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/Tools.hpp>
#include <RCF/TypeTraits.hpp>
#include <RCF/Version.hpp>

#include <boost/mpl/and.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/not.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#ifdef RCF_USE_SF_SERIALIZATION
#include <SF/memory.hpp>
#endif

#ifdef RCF_USE_BOOST_SERIALIZATION
#include <RCF/BsAutoPtr.hpp>
#endif

#include <SF/Tools.hpp>

namespace RCF {

    // The following macro hacks are necessitated by Boost.Serialization, which 
    // , apparently on principle, refuses to serialize pointers to (1) fundamental 
    // types, and (2) std::string, while happily serializing pointers to everything
    // else...

#define RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION(type)                \
    inline void serializeImpl(                                          \
        SerializationProtocolOut &out,                                  \
        const type *pt,                                                 \
        long int)                                                       \
    {                                                                   \
        serializeImpl(out, *pt, 0);                                     \
    }                                                                   \
                                                                        \
    inline void serializeImpl(                                          \
        SerializationProtocolOut &out,                                  \
        type *const pt,                                                 \
        long int)                                                       \
    {                                                                   \
        serializeImpl(out, *pt, 0);                                     \
    }                                                                   \
                                                                        \
    inline void deserializeImpl(                                        \
        SerializationProtocolIn &in,                                    \
        type *&pt,                                                      \
        long int)                                                       \
    {                                                                   \
        RCF_ASSERT(pt==NULL);                                           \
        pt = new type();                                                \
        deserializeImpl(in, *pt, 0);                                    \
    }

    SF_FOR_EACH_FUNDAMENTAL_TYPE( RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION )

#define RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION_T3(type)             \
    template<typename T1, typename T2, typename T3>                     \
    inline void serializeImpl(                                          \
        SerializationProtocolOut &out,                                  \
        const type<T1,T2,T3> *pt,                                       \
        int)                                                            \
    {                                                                   \
        serializeImpl(out, *pt, 0);                                     \
    }                                                                   \
                                                                        \
    template<typename T1, typename T2, typename T3>                     \
    inline void serializeImpl(                                          \
        SerializationProtocolOut &out,                                  \
        type<T1,T2,T3> *const pt,                                       \
        int)                                                            \
    {                                                                   \
        serializeImpl(out, *pt, 0);                                     \
    }                                                                   \
                                                                        \
    template<typename T1, typename T2, typename T3>                     \
    inline void deserializeImpl(                                        \
        SerializationProtocolIn &in,                                    \
        type<T1,T2,T3> *&pt,                                            \
        int)                                                            \
    {                                                                   \
        RCF_ASSERT(pt==NULL);                                           \
        pt = new type<T1,T2,T3>();                                      \
        deserializeImpl(in, *pt, 0);                                    \
    }

#ifdef RCF_USE_BOOST_SERIALIZATION

#if defined(__MWERKS__) || (defined(_MSC_VER) && _MSC_VER < 1310)
    // ambiguity issues with CW
    RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION(std::string)
    RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION(std::wstring)
#else
    RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION_T3(std::basic_string)
#endif

#endif

#undef RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION

#undef RCF_DEFINE_PRIMITIVE_POINTER_SERIALIZATION_T3

    // Boost.Serialization handles smart pointers very clumsily, so we do those ourselves

#define RefCountSmartPtr boost::shared_ptr

    template<typename T>
    inline void serializeImpl(
        SerializationProtocolOut &out,
        const RefCountSmartPtr<T> *spt,
        int)
    {
        serialize(out, *spt);
    }

    template<typename T>
    inline void serializeImpl(
        SerializationProtocolOut &out,
        RefCountSmartPtr<T> *const spt,
        int)
    {
        serialize(out, *spt);
    }

    template<typename T>
    inline void deserializeImpl(
        SerializationProtocolIn &in,
        RefCountSmartPtr<T> *&spt,
        int)
    {
        spt = new RefCountSmartPtr<T>();
        deserialize(in, *spt);
    }

    template<typename T>
    inline void serializeImpl(
        SerializationProtocolOut &out,
        const RefCountSmartPtr<T> &spt,
        int)
    {
        serialize(out, spt.get());
    }

    template<typename T>
    inline void deserializeImpl(
        SerializationProtocolIn &in,
        RefCountSmartPtr<T> &spt,
        int)
    {
        T *pt = NULL;
        deserialize(in, pt);

        RefCountSmartPtr<T> *pspt =
            in.mPointerContext.get( (RefCountSmartPtr<T> **) NULL, pt);

        if (pspt == NULL)
        {
            spt = RefCountSmartPtr<T>(pt);

            in.mPointerContext.set(
                &spt,
                pt);
        }
        else
        {
            spt = *pspt;
        }
    }

#undef RefCountSmartPtr

#ifdef RCF_USE_BOOST_SERIALIZATION
} // namespace RCF

namespace boost { namespace serialization {

    template<class Archive>
    void save(Archive & ar, const RCF::ByteBuffer &byteBuffer, unsigned int)
    {

        RCF::SerializationProtocolIn *pIn = NULL;
        RCF::SerializationProtocolOut *pOut = NULL;

        RCF::ClientStubPtr clientStubPtr = RCF::getCurrentClientStubPtr();
        RCF::RcfSessionPtr rcfSessionPtr = RCF::getCurrentRcfSessionPtr();
        if (clientStubPtr)
        {
            pIn = &clientStubPtr->getSpIn();
            pOut = &clientStubPtr->getSpOut();
        }
        else if (rcfSessionPtr)
        {
            pIn = &rcfSessionPtr->getSpIn();
            pOut = &rcfSessionPtr->getSpOut();
        }

        boost::uint32_t len = byteBuffer.getLength();
        ar & len;
        pOut->insert(byteBuffer);
    }

    template<class Archive>
    void load(Archive &ar, RCF::ByteBuffer &byteBuffer, unsigned int)
    {

        RCF::SerializationProtocolIn *pIn = NULL;
        RCF::SerializationProtocolOut *pOut = NULL;

        RCF::ClientStubPtr clientStubPtr = RCF::getCurrentClientStubPtr();
        RCF::RcfSessionPtr rcfSessionPtr = RCF::getCurrentRcfSessionPtr();
        if (clientStubPtr)
        {
            pIn = &clientStubPtr->getSpIn();
            pOut = &clientStubPtr->getSpOut();
        }
        else if (rcfSessionPtr)
        {
            pIn = &rcfSessionPtr->getSpIn();
            pOut = &rcfSessionPtr->getSpOut();
        }

        boost::uint32_t len = 0;
        ar & len;
        pIn->extractSlice(byteBuffer, len);
    }

}} // namespace boost namespace serialization

    BOOST_SERIALIZATION_SPLIT_FREE(RCF::ByteBuffer);

namespace RCF {
#endif // RCF_USE_BOOST_SERIALIZATION

    // serializeOverride() and deserializeOverride() are used to implement
    // a backwards compatibility workaround, for ByteBuffer interoperability 
    // with RCF runtime version <= 3.

    template<typename U>
    bool serializeOverride(SerializationProtocolOut &, U &)
    {
        return false;
    }

#if defined(__BORLANDC__) && __BORLANDC__ <= 0x560

#else

    template<typename U>
    bool serializeOverride(SerializationProtocolOut &, U *)
    {
        return false;
    }

#endif

    template<typename U>
    bool deserializeOverride(SerializationProtocolIn &, U &)
    {
        return false;
    }

    RCF_EXPORT bool serializeOverride(SerializationProtocolOut &out, ByteBuffer & u);

    RCF_EXPORT bool serializeOverride(SerializationProtocolOut &out, ByteBuffer * pu);

    RCF_EXPORT bool deserializeOverride(SerializationProtocolIn &in, ByteBuffer & u);

    template<typename T>
    class Local_Value
    {
    public:

        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsReference<T> > ));

        Local_Value() 
        { 
            mT = T(); 
        }

        T &get() 
        { 
            return mT; 
        }

        void set(bool assign, const T &t) 
        { 
            if (assign) 
            {
                mT = t; 
            }
        }

        void set(const T &t) 
        { 
            mT = t; 
        }
        
        void read(SerializationProtocolIn &in) 
        { 
            if (in.getRemainingArchiveLength() != 0)
            {
                if (!deserializeOverride(in, mT))
                {
                    deserialize(in, mT);
                }
            }            
        }

        void write(SerializationProtocolOut &out) 
        { 
            if (!serializeOverride(out, mT))
            {
                serialize(out, mT);
            }
        }

    private:
        T mT;
    };

    template<typename CRefT>
    class Local_CRef
    {
    public:

        typedef typename RemoveReference<CRefT>::type CT;
        typedef typename RemoveCv<CT>::type T;
        BOOST_MPL_ASSERT(( IsReference<CRefT> ));

#ifndef __BORLANDC__
        BOOST_MPL_ASSERT(( IsConst<CT> ));
#endif

        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        Local_CRef() : mpT() 
        {}

        T &get() 
        { 
            return *mpT; 
        }
        
        void set(bool assign, const T &t) 
        { 
            if (assign) 
            {
                *mpT = t; 
            }
        }

        void set(const T &t) 
        { 
            *mpT = t; 
        }

        void read(SerializationProtocolIn &in) 
        { 
            if (in.getRemainingArchiveLength() != 0)
            {
                T *pt = NULL;
                deserialize(in, pt);

                RCF_VERIFY(
                    pt,
                    RCF::Exception(_RcfError_DeserializationNullPointer()))
                    (typeid(T));

                boost::shared_ptr<T> *ppt =
                    in.mPointerContext.get( (boost::shared_ptr<T> **) NULL, pt);

                if (ppt == NULL)
                {
                    mpT = boost::shared_ptr<T>(pt);

                    in.mPointerContext.set(
                        &mpT,
                        pt);
                }
                else
                {
                    mpT = *ppt;
                }
            }
            else
            {
                mpT.reset( new T() );
            }
        }
        void write(SerializationProtocolOut &out) 
        { 
            serialize(out, mpT.get());
        }

    private:
        boost::shared_ptr<T> mpT;
    };

    template<typename RefT>
    class Local_Ref
    {
    public:

        typedef typename RemoveReference<RefT>::type T;
        typedef typename RemoveCv<T>::type U;
        BOOST_MPL_ASSERT(( IsReference<RefT> ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        Local_Ref() : mpT() 
        {}

        T &get() 
        { 
            return *mpT; 
        }
        
        void set(bool assign, const T &t) 
        { 
            if (assign) 
            {
                *mpT = t;
            }
        }
        
        void set(const T &t) 
        { 
            *mpT = t; 
        }

        void read(SerializationProtocolIn &in) 
        { 
            if (in.getRemainingArchiveLength() != 0)
            {
                T *pt = NULL;
                deserialize(in, pt);

                RCF_VERIFY(
                    pt,
                    RCF::Exception(_RcfError_DeserializationNullPointer()))
                    (typeid(T));

                boost::shared_ptr<T> *ppt =
                    in.mPointerContext.get( (boost::shared_ptr<T> **) NULL, pt);

                if (ppt == NULL)
                {
                    mpT = boost::shared_ptr<T>(pt);

                    in.mPointerContext.set(
                        &mpT,
                        pt);

                }
                else
                {
                    mpT = *ppt;
                }
            }
            else
            {
                mpT.reset( new T() );
            }
        }

        void write(SerializationProtocolOut &out) 
        { 
            RCF_ASSERT( mpT.get() );

            if (!serializeOverride(out, mpT.get()))
            {
                // B.Ser. issues - because the client side proxy for a T& has to 
                // deserialize itself as a value, here we have to serialize as a 
                // value.

                serialize(out, *mpT);
            }
        }

    private:
        boost::shared_ptr<T> mpT;
    };

    template<typename PtrT>
    class Local_Ptr
    {
    public:

        typedef typename RemovePointer<PtrT>::type T;
        typedef typename RemoveCv<T>::type U;
        BOOST_MPL_ASSERT(( IsPointer<PtrT> ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        Local_Ptr() : mpT() 
        {}

        T *get() 
        { 
            return mpT.get(); 
        }
        
        void set(bool assign, const T &t) 
        { 
            if (assign) 
            {
                *mpT = t;
            }
        }
        
        void set(const T &t) 
        { 
            *mpT = t; 
        }

        void read(SerializationProtocolIn &in) 
        { 
            if (in.getRemainingArchiveLength() != 0)
            {
                T *pt = NULL;
                deserialize(in, pt);

                boost::shared_ptr<T> *ppt =
                    in.mPointerContext.get( (boost::shared_ptr<T> **) NULL, pt);

                if (ppt == NULL)
                {
                    mpT = boost::shared_ptr<T>(pt);

                    in.mPointerContext.set(
                        &mpT,
                        pt);

                }
                else
                {
                    mpT = *ppt;
                }
            }
        }

        void write(SerializationProtocolOut &out) 
        { 
            serialize(out, mpT.get());
        }

    private:
        boost::shared_ptr<T> mpT;
    };

    template<typename T>
    class Proxy_Value
    {
    public:

        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsReference<T> > ));

        // We use const_cast here, in case T's copy constructor is non-const.
        // E.g. if T is a std::auto_ptr.
        Proxy_Value(const T &t) : mT( const_cast<T &>(t) ) 
        {
        }

        const T &get() 
        { 
            return mT; 
        }

        void read(SerializationProtocolIn &in) 
        {
            RCF_UNUSED_VARIABLE(in);
        }
        
        void write(SerializationProtocolOut &out) 
        { 
            if (!serializeOverride(out, mT))
            {
                serialize(out, mT);
            }
        }

    private:
        T mT;
    };

    template<typename PtrT>
    class Proxy_Ptr
    {
    public:

        typedef typename RemovePointer<PtrT>::type T;

#ifndef __BORLANDC__
        BOOST_MPL_ASSERT(( IsPointer<PtrT> ));
#endif

        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        // We use const_cast here, in case T's copy constructor is non-const.
        // E.g. if T is a std::auto_ptr.
        //Proxy_Ptr(const T &t) : mT( const_cast<T &>(t) ) 
        Proxy_Ptr(T * pt) : mpT(pt) 
        {
        }

        T *& get() 
        { 
            return mpT; 
        }

        void read(SerializationProtocolIn &in) 
        {
            RCF_UNUSED_VARIABLE(in);
        }
        
        void write(SerializationProtocolOut &out) 
        { 
            serialize(out, mpT);
        }

    private:
        T * mpT;
    };

    template<typename CRefT>
    class Proxy_CRef
    {
    public:

        typedef typename RemoveReference<CRefT>::type CT;
        typedef typename RemoveCv<CT>::type T;
        BOOST_MPL_ASSERT(( IsReference<CRefT> ));

#ifndef __BORLANDC__
        BOOST_MPL_ASSERT(( IsConst<CT> ));
#endif

        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        Proxy_CRef(const T &t) : mT(t) 
        {}

        const T &get() 
        { 
            return mT; 
        }

        void read(SerializationProtocolIn &in) 
        { 
            RCF_UNUSED_VARIABLE(in);
        }

        void write(SerializationProtocolOut &out) 
        { 
            serialize(out, &mT);
        }

    private:
        const T &mT;
    };

    template<typename RefT>
    class Proxy_Ref
    {
    public:

        typedef typename RemoveReference<RefT>::type T;
        BOOST_MPL_ASSERT(( IsReference<RefT> ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsConst<RefT> > ));
        BOOST_MPL_ASSERT(( boost::mpl::not_< IsPointer<T> > ));

        Proxy_Ref(T &t) : mT(t) 
        {}

        T &get() 
        { 
            return mT; 
        }

        void read(SerializationProtocolIn &in) 
        { 
            if (in.getRemainingArchiveLength() != 0)
            {
                if (!deserializeOverride(in, mT))
                {
                    deserialize(in, mT);
                }
            }            
        }

        void write(SerializationProtocolOut &out) 
        { 
            serialize(out, &mT);
        }

    private:
        T &mT;
    };

    template<typename T>
    struct IsConstReference
    {
        typedef typename
            boost::mpl::and_<
                IsReference<T>,
                IsConst< typename RemoveReference<T>::type >
            >::type type;

        enum { value = type::value };
    };

#ifdef __BORLANDC__

    // Local

    template<typename T>
    struct Local
    {
        typedef Local_Value<T> type;
    };

    template<typename T>
    struct Local<T*>
    {
        typedef Local_Ptr<T *> type;
    };

    template<typename T>
    struct Local<T &>
    {
        typedef Local_Ref<T &> type;
    };

    template<typename T>
    struct Local<const T &>
    {
        typedef Local_CRef<const T &> type;
    };

    // Proxy

    template<typename T>
    struct Proxy
    {
        typedef Proxy_Value<T> type;
    };

    template<typename T>
    struct Proxy<T*>
    {
        typedef Proxy_Ptr<T *> type;
    };

    template<typename T>
    struct Proxy<T &>
    {
        typedef Proxy_Ref<T &> type;
    };

    template<typename T>
    struct Proxy<const T &>
    {
        typedef Proxy_CRef<const T &> type;
    };

    // ReferenceTo

    template<typename T>
    struct ReferenceTo
    {
        typedef const T & type;
    };

    template<typename T>
    struct ReferenceTo<T &>
    {
        typedef T & type;
    };

    template<typename T>
    struct ReferenceTo<const T &>
    {
        typedef const T & type;
    };

#else

    template<typename T>
    struct Local
    {
        typedef typename
        boost::mpl::if_<
            IsPointer<T>,
            Local_Ptr<T>,
            typename boost::mpl::if_<
                IsConstReference<T>,
                Local_CRef<T>,
                typename boost::mpl::if_<
                    IsReference<T>,
                    Local_Ref<T>,
                    typename boost::mpl::if_<
                        boost::is_enum<T>,
                        //Local_Enum<T>,
                        Local_Value<T>,
                        Local_Value<T>
                    >::type
                >::type
            >::type
        >::type type;
    };

    template<typename T>
    struct Proxy
    {
        typedef typename
        boost::mpl::if_<
            IsPointer<T>,
            Proxy_Ptr<T>,
            typename boost::mpl::if_<
                IsConstReference<T>,
                Proxy_CRef<T>,
                typename boost::mpl::if_<
                    IsReference<T>,
                    Proxy_Ref<T>,
                    typename boost::mpl::if_<
                        boost::is_enum<T>,
                        //Proxy_Enum<T>,
                        Proxy_Value<T>,
                        Proxy_Value<T>
                    >::type
                >::type
            >::type
        >::type type;
    };


    // ReferenceTo:
    // For generic T, return const T &.
    // For T &, return T &.
    // For const T &, return const T &

    template<typename T>
    struct ReferenceTo
    {
        typedef typename
        boost::mpl::if_<
            IsReference<T>,
            T,
            typename boost::mpl::if_<
                RCF::IsConst<T>,
                typename boost::add_reference<T>::type,
                typename boost::add_reference<
                    typename boost::add_const<T>::type
                >::type
            >::type
        >::type type;
    };

#endif

    class I_Parameters
    {
    public:
        virtual ~I_Parameters() {}
        virtual void read(SerializationProtocolIn &in) = 0;
        virtual void write(SerializationProtocolOut &out) = 0;
        virtual bool enrolFutures(RCF::ClientStub *pClientStub) = 0;
    };

    template<typename T>
    struct IsInParameter
    {
        typedef typename boost::mpl::not_< boost::is_same<T,Void> >::type type;
        enum { value = type::value };
    };

    template<typename T>
    struct IsOutParameter
    {
        typedef typename
        boost::mpl::and_<
            IsReference<T>,
            boost::mpl::not_< 
                boost::is_const< 
                    typename RemoveReference<T>::type
                > 
            > 
        >::type type;
        enum { value = type::value };
    };

    template<typename T>
    struct IsReturnValue
    {
        typedef typename boost::mpl::not_< boost::is_same<T, Void> >::type type;
        enum { value = type::value };
    };

    typedef std::map<const void *, I_Future *> Candidates;
    RCF_EXPORT Mutex & gCandidatesMutex();
    RCF_EXPORT Candidates & gCandidates();

    template<
        typename R, 
        typename A1,
        typename A2,
        typename A3,
        typename A4,
        typename A5,
        typename A6,
        typename A7,
        typename A8>
    class ClientParameters : public I_Parameters
    {
    public:

        typedef typename ReferenceTo<A1>::type A1Ref;
        typedef typename ReferenceTo<A2>::type A2Ref;
        typedef typename ReferenceTo<A3>::type A3Ref;
        typedef typename ReferenceTo<A4>::type A4Ref;
        typedef typename ReferenceTo<A5>::type A5Ref;
        typedef typename ReferenceTo<A6>::type A6Ref;
        typedef typename ReferenceTo<A7>::type A7Ref;
        typedef typename ReferenceTo<A8>::type A8Ref;

        ClientParameters( 
            A1Ref a1, A2Ref a2, A3Ref a3, A4Ref a4, A5Ref a5, A6Ref a6, A7Ref a7, A8Ref a8) : 
                a1(a1), a2(a2), a3(a3), a4(a4), a5(a5), a6(a6), a7(a7), a8(a8)
        {
        }

        void read(SerializationProtocolIn &in)
        {
            if (IsReturnValue<R>::value)    r.read(in);
            if (IsOutParameter<A1>::value)    a1.read(in);
            if (IsOutParameter<A2>::value)    a2.read(in);
            if (IsOutParameter<A3>::value)    a3.read(in);
            if (IsOutParameter<A4>::value)    a4.read(in);
            if (IsOutParameter<A5>::value)    a5.read(in);
            if (IsOutParameter<A6>::value)    a6.read(in);
            if (IsOutParameter<A7>::value)    a7.read(in);
            if (IsOutParameter<A8>::value)    a8.read(in);
        }

        void write(SerializationProtocolOut &out)
        {
            if (IsInParameter<A1>::value)    a1.write(out);
            if (IsInParameter<A2>::value)    a2.write(out);
            if (IsInParameter<A3>::value)    a3.write(out);
            if (IsInParameter<A4>::value)    a4.write(out);
            if (IsInParameter<A5>::value)    a5.write(out);
            if (IsInParameter<A6>::value)    a6.write(out);
            if (IsInParameter<A7>::value)    a7.write(out);
            if (IsInParameter<A8>::value)    a8.write(out);
        }

        bool enrolFutures(RCF::ClientStub *pClientStub)
        {
            bool enrolled = false;

            const void * pva[] = {
                &a1.get(),
                &a2.get(),
                &a3.get(),
                &a4.get(),
                &a5.get(),
                &a6.get(),
                &a7.get(),
                &a8.get() };

            for (std::size_t i=0; i<sizeof(pva)/sizeof(pva[0]); ++i)
            {
                const void *pv = pva[i];
                I_Future * pFuture = NULL;

                {
                    Lock lock(gCandidatesMutex());

                    std::map<const void *,I_Future *>::iterator iter = 
                        gCandidates().find(pv);

                    if ( iter != gCandidates().end() ) 
                    {
                        pFuture = iter->second;
                        gCandidates().erase(iter);
                    }
                }

                if (pFuture)
                {
                    pClientStub->enrol( pFuture );
                    enrolled = true;
                }
            }

            return enrolled;
        }

        typename Local<R>::type         r;
        typename Proxy<A1>::type        a1;
        typename Proxy<A2>::type        a2;
        typename Proxy<A3>::type        a3;
        typename Proxy<A4>::type        a4;
        typename Proxy<A5>::type        a5;
        typename Proxy<A6>::type        a6;
        typename Proxy<A7>::type        a7;
        typename Proxy<A8>::type        a8;
    };

    template<
        typename R, 
        typename A1,
        typename A2,
        typename A3,
        typename A4,
        typename A5,
        typename A6,
        typename A7,
        typename A8>
    class AllocateClientParameters
    {
    public:

        typedef typename ReferenceTo<A1>::type A1Ref;
        typedef typename ReferenceTo<A2>::type A2Ref;
        typedef typename ReferenceTo<A3>::type A3Ref;
        typedef typename ReferenceTo<A4>::type A4Ref;
        typedef typename ReferenceTo<A5>::type A5Ref;
        typedef typename ReferenceTo<A6>::type A6Ref;
        typedef typename ReferenceTo<A7>::type A7Ref;
        typedef typename ReferenceTo<A8>::type A8Ref;

        typedef ClientParameters<R, A1, A2, A3, A4, A5, A6, A7, A8> ParametersT;

        // TODO: unnecessary copy of a* here, if A* is not a reference
        ParametersT &operator()(
            ClientStub &clientStub, 
            A1Ref a1, 
            A2Ref a2, 
            A3Ref a3, 
            A4Ref a4, 
            A5Ref a5, 
            A6Ref a6, 
            A7Ref a7, 
            A8Ref a8) const
        {
            clientStub.clearParameters();

            clientStub.mParametersVec.resize(sizeof(ParametersT));
            
            clientStub.mpParameters = new 
                ( &clientStub.mParametersVec[0] ) 
                ParametersT(a1,a2,a3,a4,a5,a6,a7,a8);

            return static_cast<ParametersT &>(*clientStub.mpParameters);
        }
    };

    template<
        typename R, 
        typename A1 = Void, 
        typename A2 = Void, 
        typename A3 = Void,
        typename A4 = Void,
        typename A5 = Void, 
        typename A6 = Void, 
        typename A7 = Void, 
        typename A8 = Void>
    class ServerParameters : public I_Parameters
    {
    public:

        ServerParameters(RcfSession &session)
        {
            read(session.mIn);
        }

        void read(SerializationProtocolIn &in)
        {
            if (IsInParameter<A1>::value) a1.read(in);
            if (IsInParameter<A2>::value) a2.read(in);
            if (IsInParameter<A3>::value) a3.read(in);
            if (IsInParameter<A4>::value) a4.read(in);
            if (IsInParameter<A5>::value) a5.read(in);
            if (IsInParameter<A6>::value) a6.read(in);
            if (IsInParameter<A7>::value) a7.read(in);
            if (IsInParameter<A8>::value) a8.read(in);
        }

        void write(SerializationProtocolOut &out)
        {
            if (IsReturnValue<R>::value) r.write(out);
            if (IsOutParameter<A1>::value) a1.write(out);
            if (IsOutParameter<A2>::value) a2.write(out);
            if (IsOutParameter<A3>::value) a3.write(out);
            if (IsOutParameter<A4>::value) a4.write(out);
            if (IsOutParameter<A5>::value) a5.write(out);
            if (IsOutParameter<A6>::value) a6.write(out);
            if (IsOutParameter<A7>::value) a7.write(out);
            if (IsOutParameter<A8>::value) a8.write(out);
        }

        // TODO: we shouldn't need this here
        bool enrolFutures(RCF::ClientStub *)
        {
            RCF_ASSERT(0);
            return false;
        }

        typename Local<R>::type         r;
        typename Local<A1>::type        a1;
        typename Local<A2>::type        a2;
        typename Local<A3>::type        a3;
        typename Local<A4>::type        a4;
        typename Local<A5>::type        a5;
        typename Local<A6>::type        a6;
        typename Local<A7>::type        a7;
        typename Local<A8>::type        a8;
    };

    typedef boost::shared_ptr<I_Parameters> ParametersPtr;

    template<
        typename R, 
        typename A1 = Void, 
        typename A2 = Void, 
        typename A3 = Void, 
        typename A4 = Void, 
        typename A5 = Void, 
        typename A6 = Void, 
        typename A7 = Void, 
        typename A8 = Void>
    class AllocateServerParameters
    {
    public:
        typedef ServerParameters<R, A1, A2, A3, A4, A5, A6, A7, A8> ParametersT;
        ParametersT &operator()(RcfSession &session) const
        {
            session.clearParameters();

            session.mParametersVec.resize(sizeof(ParametersT));

            session.mpParameters = new 
                ( &session.mParametersVec[0] ) 
                ParametersT(session);

            return static_cast<ParametersT &>(*session.mpParameters);
        }
    };

    class I_Future
    {
    public:
        virtual ~I_Future() {}
        virtual void setClientStub(ClientStub *pClientStub) = 0;
    };

    template<typename T>
    class FutureImpl;

    template<typename T>
    class Future
    {
    public:
        Future() : mStatePtr(new State())
        {}

        Future(T *pt) : mStatePtr( new State(pt))
        {}

        Future(T *pt, ClientStub *pClientStub) : mStatePtr( new State(pt))
        {
            pClientStub->enrol(mStatePtr.get());
        }

        Future(const T &t) : mStatePtr( new State(t))
        {}

        operator T&() 
        { 
            return mStatePtr->operator T&();
        }

        T& operator*()
        {
            return mStatePtr->operator T&();
        }

        Future &operator=(const Future &rhs)
        {
            mStatePtr = rhs.mStatePtr;
            return *this;
        }
    
        Future &operator=(const FutureImpl<T> &rhs)
        {
            rhs.assignTo(*this);
            return *this;
        }

        Future(const FutureImpl<T> &rhs) : mStatePtr( new State())
        {
            rhs.assignTo(*this);
        }

        bool ready()
        {
            return mStatePtr->ready();
        }

        void wait(boost::uint32_t timeoutMs = 0)
        {
            mStatePtr->wait(timeoutMs);
        }

        void cancel()
        {
            mStatePtr->cancel();
        }

        void clear()
        {
            mStatePtr->clear();
        }

        ClientStub & getClientStub()
        {
            return mStatePtr->getClientStub();
        }

    private:

#if defined(_MSC_VER) && _MSC_VER < 1310

    public:
#else

        template<typename U>
        friend class FutureImpl;
#endif

        class State : public I_Future, boost::noncopyable
        {
        public:
            State() : 
                mpt(RCF_DEFAULT_INIT), 
                mtPtr( new T() ), 
                mpClientStub(RCF_DEFAULT_INIT)
            {}

            State(T *pt) : 
                mpt(pt), 
                mpClientStub(RCF_DEFAULT_INIT)
            {}

            State(const T &t) : 
                mpt(RCF_DEFAULT_INIT), 
                mtPtr( new T(t) ), 
                mpClientStub(RCF_DEFAULT_INIT)
            {}

            ~State()
            {
                RCF_DTOR_BEGIN
                unregisterFromCandidates();                            
                RCF_DTOR_END
            }

            operator T&()
            {
                // If a call has been made, check that it has completed, and
                // that there was no exception.

                if (mpClientStub)
                {
                    RCF_ASSERT(mpClientStub->ready());
                    RCF_ASSERT(!mpClientStub->hasAsyncException());
                }

                T *pt = mpt ? mpt : mtPtr.get();
                {
                    Lock lock(gCandidatesMutex());
                    gCandidates()[pt] = this;
                }
                
                return *pt;
            }

            void setClientStub(ClientStub *pClientStub)
            {
                mpClientStub = pClientStub;
            }

            void setClientStub(ClientStub *pClientStub, T * pt)
            {
                unregisterFromCandidates();

                mpClientStub = pClientStub;
                mpt = pt;
                mtPtr.reset();
            }

        private:

            T *                     mpt;
            boost::scoped_ptr<T>    mtPtr;
            RCF::ClientStub *       mpClientStub;

        public:

            bool ready()
            {
                return mpClientStub->ready();
            }

            void wait(boost::uint32_t timeoutMs = 0)
            {
                mpClientStub->wait(timeoutMs);
            }

            void cancel()
            {
                mpClientStub->cancel();                
            }

            ClientStub & getClientStub()
            {
                return *mpClientStub;
            }

            void unregisterFromCandidates()
            {
                T *pt = mpt ? mpt : mtPtr.get();
                Lock lock(gCandidatesMutex());
                Candidates::iterator iter = gCandidates().find(pt);
                if (iter != gCandidates().end())
                {
                    gCandidates().erase(iter);
                }
            }

        };

        boost::shared_ptr<State> mStatePtr;
    };

    template<typename T>
    class FutureImpl
    {
    public:
        FutureImpl(
            T &t, 
            ClientStub &clientStub, 
            const std::string &subInterface,
            int fnId,
            RemoteCallSemantics rcs) :
                mpT(&t),
                mpClientStub(&clientStub),
                mSubInterface(subInterface),
                mFnId(fnId),
                mRcs(rcs),
                mOwn(true)
        {
            // TODO: put this in the initializer list instead?
            clientStub.init(subInterface, fnId,rcs);
        }

        FutureImpl(const FutureImpl &rhs) :
            mpT(rhs.mpT),
            mpClientStub(rhs.mpClientStub),
            mSubInterface(rhs.mSubInterface),
            mFnId(rhs.mFnId),
            mRcs(rhs.mRcs),
            mOwn(rhs.mOwn)
        {
            rhs.mOwn = false;
        }

        FutureImpl &operator=(const FutureImpl &rhs)
        {
            mpT = rhs.mpT;
            mpClientStub = rhs.mpClientStub;
            mSubInterface = rhs.mSubInterface;
            mFnId = rhs.mFnId;
            mRcs = rhs.mRcs;

            mOwn = rhs.mOwn;
            rhs.mOwn = false;
            return *this;
        }

        // Conversion to T kicks off a sync call
        operator T() const
        {
            mOwn = false;

            call();

            T t = *mpT;
            return t;
        }

        T get()
        {
            return operator T();
        }

        // assignment to Future<> kicks off async call
        void assignTo(Future<T> &future) const
        {
            mOwn = false;
            mpClientStub->setAsync(true);
            future.mStatePtr->setClientStub(mpClientStub, mpT);
            call();
        }

        ~FutureImpl()
        {
            if(mOwn)
            {
                call();
            }
        }

    private:

        void call() const
        {

            // TODO
            bool async = mpClientStub->getAsync();

            mpClientStub->setTries(0);

            if (async)
            {
                callAsync();
            }
            else
            {
                callSync();
            }
        }

        void callSync() const
        {
            // ClientStub::onConnectCompleted() uses the contents of mEncodedByteBuffers
            // to determine what stage the current call is in. So mEncodedByteBuffers
            // needs to be cleared after a remote call, even if an exception is thrown.

            // Error handling code here will generally also need to be present in 
            // ClientStub::onError().

            try
            {
                mpClientStub->call(mRcs);
            }
            catch(const RCF::RemoteException &)
            {
                mpClientStub->mEncodedByteBuffers.resize(0);
                throw; 
            }
            catch(const RCF::Exception &)
            {
                mpClientStub->mEncodedByteBuffers.resize(0);
                mpClientStub->disconnect();
                throw;
            }
            catch(...)
            {
                mpClientStub->mEncodedByteBuffers.resize(0);
                mpClientStub->disconnect();
                throw;
            }
        }

        void callAsync() const
        {
            try
            {
                mpClientStub->call(mRcs);
            }
            catch(const RCF::Exception & e)
            {
                std::auto_ptr<RCF::Exception> ape( e.clone() );
                mpClientStub->setAsyncException(ape);

                if (mpClientStub->mAsyncCallback)
                {
                    boost::function0<void> cb = mpClientStub->mAsyncCallback;
                    mpClientStub->mAsyncCallback = boost::function0<void>();

                    cb();
                }
            }
        }

        T *                     mpT;
        ClientStub *            mpClientStub;
        std::string             mSubInterface;
        int                     mFnId;
        RemoteCallSemantics     mRcs;

        mutable bool            mOwn;
    };

    template<typename T, typename U>
    bool operator==(const FutureImpl<T> & fi, const U & u)
    {
        return fi.operator T() == u;
    }

    template<typename T, typename U>
    bool operator==(const U & u, const FutureImpl<T> & fi)
    {
        return u == fi.operator T();
    }

    template<typename T>
    std::ostream & operator<<(std::ostream & os, const FutureImpl<T> & fi)
    {
        return os << fi.operator T();
    }

    // Bidirectional connections - converting between RcfClient and RcfSession.

    RCF_EXPORT void convertRcfSessionToRcfClient(
        boost::function1<void, ClientTransportAutoPtr> func,
        RemoteCallSemantics rcs = RCF::Twoway);

    RCF_EXPORT RcfSessionPtr convertRcfClientToRcfSession(
        ClientStub & clientStub, 
        RcfServer & server);

    template<typename RcfClientT>
    inline RcfSessionPtr convertRcfClientToRcfSession(
        RcfClientT & client, 
        RcfServer & server)
    {
        return convertRcfClientToRcfSession(
            client.getClientStub(),
            server);
    }

} // namespace RCF

#endif // ! INCLUDE_RCF_MARSHAL_HPP
