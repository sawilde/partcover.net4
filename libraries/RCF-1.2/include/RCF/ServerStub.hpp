
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_SERVERSTUB_HPP
#define INCLUDE_RCF_SERVERSTUB_HPP

#include <map>
#include <memory>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/GetInterfaceName.hpp>
#include <RCF/RcfClient.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/Token.hpp>

// NB: occurrences of "interface" in this file have been replaced with "interface_", due to obscure bugs with Visual C++

namespace RCF {

    class                                   I_RcfClient;
    typedef boost::shared_ptr<I_RcfClient>  RcfClientPtr;

    template<typename T>
    class I_Deref
    {
    public:
        virtual ~I_Deref() {}
        virtual T &deref() = 0;
    };

    template<typename T>
    class DerefPtr
    {
    public:
        typedef boost::shared_ptr< I_Deref<T> > type;
    };

    template<typename T>
    class DerefObj : public I_Deref<T>
    {
    public:
        DerefObj(T &t) :
            mT(t)
        {}

        T &deref()
        {
            return mT;
        }

    private:
        T &mT;
    };

    template<typename T>
    class DerefSharedPtr : public I_Deref<T>
    {
    public:
        DerefSharedPtr(boost::shared_ptr<T> tPtr) :
            mTPtr(tPtr)
        {}

        T &deref()
        {
            return *mTPtr;
        }

    private:
        boost::shared_ptr<T> mTPtr;
    };

    template<typename T>
    class DerefWeakPtr : public I_Deref<T>
    {
    public:
        DerefWeakPtr(boost::weak_ptr<T> tWeakPtr) :
            mTWeakPtr(tWeakPtr)
        {}

        T &deref()
        {
            boost::shared_ptr<T> tPtr(mTWeakPtr);
            if (tPtr.get())
            {
                return *tPtr;
            }
            RCF_THROW(Exception(_RcfError_ServerStubExpired()));
        }

    private:
        boost::weak_ptr<T> mTWeakPtr;
    };

    template<typename T>
    class DerefAutoPtr : public I_Deref<T>
    {
    public:
        DerefAutoPtr(const std::auto_ptr<T> &tAutoPtr) :
            mTAutoPtr( const_cast<std::auto_ptr<T> &>(tAutoPtr))
        {}

        T &deref()
        {
            return *mTAutoPtr;
        }

    private:
        std::auto_ptr<T> mTAutoPtr;
    };

    typedef boost::function2<
        void,
        int,
        RcfSession &> InvokeFunctor;

    typedef std::map<std::string,  InvokeFunctor> InvokeFunctorMap;

    class StubAccess
    {
    public:

        template<typename InterfaceT, typename IdT, typename ImplementationT>
        void invoke(
            InterfaceT &                interface_,
            const IdT &                 id,
            RcfSession &                session,
            ImplementationT &           t)
        {
            interface_.invoke(id, session, t);
        }

        template<typename InterfaceT, typename DerefPtrT>
        void registerInvokeFunctors(
            InterfaceT &                interface_,
            InvokeFunctorMap &          invokeFunctorMap,
            DerefPtrT                   derefPtr)
        {
            interface_.registerInvokeFunctors(invokeFunctorMap, derefPtr);
        }

        template<typename InheritT, typename InterfaceT, typename DerefPtrT>
        void registerParentInvokeFunctors(
            InheritT *,
            InterfaceT &                interface_,
            InvokeFunctorMap &          invokeFunctorMap,
            DerefPtrT                   derefPtr,
            boost::mpl::false_ *)
        {
            typedef typename GetInterface<InheritT>::type ParentInterfaceT;
            interface_.ParentInterfaceT::registerInvokeFunctors(
                invokeFunctorMap,
                derefPtr);
        }

        template<typename InheritT, typename InterfaceT, typename DerefPtrT>
        void registerParentInvokeFunctors(
            InheritT *,
            InterfaceT &,
            InvokeFunctorMap &,
            DerefPtrT,
            boost::mpl::true_*)
        {}

        template<typename InheritT, typename InterfaceT, typename DerefPtrT>
        void registerParentInvokeFunctors(
            InheritT *                  i,
            InterfaceT &                interface_,
            InvokeFunctorMap &          invokeFunctorMap,
            DerefPtrT                   derefPtr)
        {

            typedef BOOST_DEDUCED_TYPENAME boost::is_same<
                InheritT,
                BOOST_DEDUCED_TYPENAME GetInterface<InheritT>::type >::type type;

            registerParentInvokeFunctors(
                i,
                interface_,
                invokeFunctorMap,
                derefPtr,
                (type *) NULL);
        }

        template<typename InheritT, typename InterfaceT>
        void setClientStubPtr(
            InheritT *,
            InterfaceT &interface_,
            boost::mpl::false_ *)
        {
            typedef typename InheritT::RcfClientT ParentInterfaceT;
            interface_.ParentInterfaceT::setClientStubPtr(
                interface_.mClientStubPtr);
        }

        template<typename InheritT, typename InterfaceT>
        void setClientStubPtr(
            InheritT *,
            InterfaceT &,
            boost::mpl::true_ *)
        {}

        template<typename InheritT, typename InterfaceT>
        void setClientStubPtr(
            InheritT *i,
            InterfaceT &interface_)
        {

            typedef BOOST_DEDUCED_TYPENAME boost::is_same<
                InheritT,
                BOOST_DEDUCED_TYPENAME GetInterface<InheritT>::type >::type type;

            setClientStubPtr(i, interface_, (type *) NULL);
        }

        template<typename Archive, typename RcfClientT>
        void serialize(
            Archive &                           ar, 
            RcfClientT &                        rcfClient)
        {
            if (ar.isWrite())
            {
                rcfClient.mClientStubPtr ?
                    ar & true & rcfClient.getClientStub() :
                    ar & false;
            }
            else //if (ar.isRead())
            {
                bool hasClientStub = false;
                ar & hasClientStub;
                if (hasClientStub)
                {
                    if (!rcfClient.mClientStubPtr)
                    {
                        typedef typename RcfClientT::Interface Interface;
                        std::string interfaceName = getInterfaceName( (Interface*) 0);
                        ClientStubPtr clientStubPtr(new ClientStub(interfaceName));
                        rcfClient.setClientStubPtr(clientStubPtr);
                    }
                    ar & rcfClient.getClientStub();
                }
                else
                {
                    rcfClient.setClientStubPtr( ClientStubPtr());
                }
            }
        }

        template<typename Archive, typename RcfClientT>
        void serialize(
            Archive &                           ar, 
            RcfClientT &                        rcfClient,
            const unsigned int)
        {
            typedef typename Archive::is_saving IsSaving;
            const bool isSaving = IsSaving::value;

            if (isSaving)
            {
                bool hasClientStub = rcfClient.mClientStubPtr;
                ar & hasClientStub;
                if (hasClientStub)
                {
                    ar & rcfClient.getClientStub();
                }
            }
            else //if (ar.isRead())
            {
                bool hasClientStub = false;
                ar & hasClientStub;
                
                if (hasClientStub)
                {
                    if (!rcfClient.mClientStubPtr)
                    {
                        typedef typename RcfClientT::Interface Interface;
                        std::string interfaceName = getInterfaceName( (Interface*) 0);
                        ClientStubPtr clientStubPtr(new ClientStub(interfaceName));
                        rcfClient.setClientStubPtr(clientStubPtr);
                    }
                    ar & rcfClient.getClientStub();
                }
                else
                {
                    rcfClient.setClientStubPtr( ClientStubPtr());
                }
            }
        }

        template<typename RcfClientT>
        ClientStubPtr getClientStubPtr(const RcfClientT &rcfClient)
        {
            return rcfClient.mClientStubPtr;
        }

    };

    template<typename InterfaceT, typename ImplementationT>
    inline void invoke(
        InterfaceT &                    interface_,
        ImplementationT &               t,
        int                             fnId,
        RcfSession  &                   session)
    {
        switch (fnId) {
        case  0: StubAccess().invoke(interface_, boost::mpl::int_< 0>(), session, t); break;
        case  1: StubAccess().invoke(interface_, boost::mpl::int_< 1>(), session, t); break;
        case  2: StubAccess().invoke(interface_, boost::mpl::int_< 2>(), session, t); break;
        case  3: StubAccess().invoke(interface_, boost::mpl::int_< 3>(), session, t); break;
        case  4: StubAccess().invoke(interface_, boost::mpl::int_< 4>(), session, t); break;
        case  5: StubAccess().invoke(interface_, boost::mpl::int_< 5>(), session, t); break;
        case  6: StubAccess().invoke(interface_, boost::mpl::int_< 6>(), session, t); break;
        case  7: StubAccess().invoke(interface_, boost::mpl::int_< 7>(), session, t); break;
        case  8: StubAccess().invoke(interface_, boost::mpl::int_< 8>(), session, t); break;
        case  9: StubAccess().invoke(interface_, boost::mpl::int_< 9>(), session, t); break;
        case 10: StubAccess().invoke(interface_, boost::mpl::int_<10>(), session, t); break;
        case 11: StubAccess().invoke(interface_, boost::mpl::int_<11>(), session, t); break;
        case 12: StubAccess().invoke(interface_, boost::mpl::int_<12>(), session, t); break;
        case 13: StubAccess().invoke(interface_, boost::mpl::int_<13>(), session, t); break;
        case 14: StubAccess().invoke(interface_, boost::mpl::int_<14>(), session, t); break;
        case 15: StubAccess().invoke(interface_, boost::mpl::int_<15>(), session, t); break;
        case 16: StubAccess().invoke(interface_, boost::mpl::int_<16>(), session, t); break;
        case 17: StubAccess().invoke(interface_, boost::mpl::int_<17>(), session, t); break;
        case 18: StubAccess().invoke(interface_, boost::mpl::int_<18>(), session, t); break;
        case 19: StubAccess().invoke(interface_, boost::mpl::int_<19>(), session, t); break;
        case 20: StubAccess().invoke(interface_, boost::mpl::int_<20>(), session, t); break;
        case 21: StubAccess().invoke(interface_, boost::mpl::int_<21>(), session, t); break;
        case 22: StubAccess().invoke(interface_, boost::mpl::int_<22>(), session, t); break;
        case 23: StubAccess().invoke(interface_, boost::mpl::int_<23>(), session, t); break;
        case 24: StubAccess().invoke(interface_, boost::mpl::int_<24>(), session, t); break;
        case 25: StubAccess().invoke(interface_, boost::mpl::int_<25>(), session, t); break;
        case 26: StubAccess().invoke(interface_, boost::mpl::int_<26>(), session, t); break;
        case 27: StubAccess().invoke(interface_, boost::mpl::int_<27>(), session, t); break;
        case 28: StubAccess().invoke(interface_, boost::mpl::int_<28>(), session, t); break;
        case 29: StubAccess().invoke(interface_, boost::mpl::int_<29>(), session, t); break;
        case 30: StubAccess().invoke(interface_, boost::mpl::int_<30>(), session, t); break;
        case 31: StubAccess().invoke(interface_, boost::mpl::int_<31>(), session, t); break;
        case 32: StubAccess().invoke(interface_, boost::mpl::int_<32>(), session, t); break;
        case 33: StubAccess().invoke(interface_, boost::mpl::int_<33>(), session, t); break;
        case 34: StubAccess().invoke(interface_, boost::mpl::int_<34>(), session, t); break;
        case 35: StubAccess().invoke(interface_, boost::mpl::int_<35>(), session, t); break;
        default: RCF_THROW(Exception(_RcfError_FnId(fnId)))(fnId);
        }
    }

    template<typename InterfaceT, typename DerefPtrT>
    class InterfaceInvocator
    {
    public:
        InterfaceInvocator(InterfaceT &interface_, DerefPtrT derefPtr) :
            mInterface(interface_),
            mDerefPtr(derefPtr)
        {}

        void operator()(
            int                         fnId,
            RcfSession &                session)
        {
            invoke<InterfaceT>(mInterface, mDerefPtr->deref(), fnId, session);
        }

    private:
        InterfaceT &    mInterface;
        DerefPtrT       mDerefPtr;
    };

    template<typename InterfaceT, typename DerefPtrT>
    void registerInvokeFunctors(
        InterfaceT &                    interface_,
        InvokeFunctorMap &              invokeFunctorMap,
        DerefPtrT                       derefPtr)
    {
        // NB: same interface may occur more than once in the inheritance hierarchy of another interface, and in
        // that case, via overwriting, only one InterfaceInvocator is registered, so only the functions in one of the interfaces will ever be called.
        // But it doesn't matter, since even if an interface occurs several times in the inheritance hierarchy, each occurrence
        // of the interface will be bound to derefPtr in exactly the same way.

        typedef typename InterfaceT::Interface Interface;
        std::string interfaceName = ::RCF::getInterfaceName( (Interface *) NULL);

        invokeFunctorMap[ interfaceName ] =
            InterfaceInvocator<InterfaceT, DerefPtrT>(interface_, derefPtr);
    }

    class ServerStub;

    typedef boost::shared_ptr<ServerStub> ServerStubPtr;

    class RCF_EXPORT ServerStub
    {
    public:

        template<typename InterfaceT, typename DerefPtrT>
        void registerInvokeFunctors(InterfaceT &interface_, DerefPtrT derefPtr)
        {
            StubAccess access;
            access.registerInvokeFunctors(
                interface_,
                mInvokeFunctorMap,
                derefPtr);
        }

        void invoke(
            const std::string &         subInterface,
            int                         fnId,
            RcfSession &                session);

        void merge(RcfClientPtr rcfClientPtr);

    private:
        // TODO: too much overhead per server stub?
        InvokeFunctorMap                mInvokeFunctorMap;
        std::vector<RcfClientPtr>       mMergedStubs;
    };

    template<typename InterfaceT, typename ImplementationT, typename ImplementationPtrT>
    RcfClientPtr createServerStub(
        InterfaceT *,
        ImplementationT *,
        ImplementationPtrT px)
    {
        typedef typename InterfaceT::RcfClientT RcfClientT;
        return RcfClientPtr( new RcfClientT(
            ServerStubPtr(new ServerStub()),
            px,
            (boost::mpl::true_ *) NULL));
    }
    
} // namespace RCF

#endif // ! INCLUDE_RCF_SERVERSTUB_HPP
