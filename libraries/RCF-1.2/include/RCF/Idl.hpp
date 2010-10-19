
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_IDL_HPP
#define INCLUDE_RCF_IDL_HPP

#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/Endpoint.hpp>
#include <RCF/Exception.hpp>
#include <RCF/GetInterfaceName.hpp>
#include <RCF/Marshal.hpp>
#include <RCF/RcfClient.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ServerStub.hpp>
#include <RCF/ThreadLocalData.hpp>
    
#define RCF_BEGIN(InterfaceT, Name) RCF_BEGIN_I0(InterfaceT, Name)

#define RCF_BEGIN_I0(InterfaceT, Name)                                                                  \
    RCF_BEGIN_IMPL_PRELUDE(InterfaceT, Name)                                                            \
    RCF_BEGIN_IMPL_INHERITED_0(InterfaceT, Name)                                                        \
    RCF_BEGIN_IMPL_POSTLUDE(InterfaceT, Name)

#define RCF_BEGIN_I1(InterfaceT, Name, InheritT1)                                                       \
    RCF_BEGIN_IMPL_PRELUDE(InterfaceT, Name)                                                            \
    RCF_BEGIN_IMPL_INHERITED_1(InterfaceT, Name, InheritT1)                                             \
    RCF_BEGIN_IMPL_POSTLUDE(InterfaceT, Name)

#define RCF_BEGIN_I2(InterfaceT, Name, InheritT1, InheritT2)                                            \
    RCF_BEGIN_IMPL_PRELUDE(InterfaceT, Name)                                                            \
    RCF_BEGIN_IMPL_INHERITED_2(InterfaceT, Name, InheritT1, InheritT2)                                  \
    RCF_BEGIN_IMPL_POSTLUDE(InterfaceT, Name)

#define RCF_BEGIN_IMPL_PRELUDE(InterfaceT, Name)                                                        \
                                                                                                        \
    template<typename T>                                                                                \
    class RcfClient;                                                                                    \
                                                                                                        \
    class InterfaceT                                                                                    \
    {                                                                                                   \
    public:                                                                                             \
        typedef RcfClient<InterfaceT> RcfClientT;                                                       \
        static std::string getInterfaceName()                                                           \
        {                                                                                               \
            return std::string(Name) == "" ? #InterfaceT : Name;                                        \
        }                                                                                               \
    };

#define RCF_BEGIN_IMPL_INHERITED_0(InterfaceT, Name)                                                    \
    template<>                                                                                          \
    class RcfClient< InterfaceT > :                                                                     \
        public virtual ::RCF::I_RcfClient                                                               \
    {                                                                                                   \
    private:                                                                                            \
        template<typename DerefPtrT>                                                                    \
        void registerInvokeFunctors(::RCF::InvokeFunctorMap &invokeFunctorMap, DerefPtrT derefPtr)      \
        {                                                                                               \
            ::RCF::registerInvokeFunctors(*this, invokeFunctorMap, derefPtr);                           \
        }                                                                                               \
        void setClientStubPtr(::RCF::ClientStubPtr clientStubPtr)                                       \
        {                                                                                               \
            mClientStubPtr = clientStubPtr;                                                             \
        }

#define RCF_BEGIN_IMPL_INHERITED_1(InterfaceT, Name, InheritT1)                                         \
    template<>                                                                                          \
    class RcfClient< InterfaceT > :                                                                     \
        public virtual ::RCF::I_RcfClient,                                                              \
        public virtual ::RCF::GetInterface<InheritT1>::type                                             \
    {                                                                                                   \
    private:                                                                                            \
        template<typename DerefPtrT>                                                                    \
        void registerInvokeFunctors(::RCF::InvokeFunctorMap &invokeFunctorMap, DerefPtrT derefPtr)      \
        {                                                                                               \
            ::RCF::registerInvokeFunctors(*this, invokeFunctorMap, derefPtr);                           \
            ::RCF::StubAccess().registerParentInvokeFunctors(                                           \
                (InheritT1 *) NULL,                                                                     \
                *this,                                                                                  \
                invokeFunctorMap,                                                                       \
                derefPtr);                                                                              \
        }                                                                                               \
        void setClientStubPtr(::RCF::ClientStubPtr clientStubPtr)                                       \
        {                                                                                               \
            mClientStubPtr = clientStubPtr;                                                             \
            ::RCF::StubAccess().setClientStubPtr( (InheritT1*) 0, *this);                               \
        }

#define RCF_BEGIN_IMPL_INHERITED_2(InterfaceT, Name, InheritT1, InheritT2)                              \
    template<>                                                                                          \
    class RcfClient< InterfaceT > :                                                                     \
        public virtual ::RCF::I_RcfClient,                                                              \
        public virtual ::RCF::GetInterface<InheritT1>::type,                                            \
        public virtual ::RCF::GetInterface<InheritT2>::type                                             \
    {                                                                                                   \
    private:                                                                                            \
        template<typename DerefPtrT>                                                                    \
        void registerInvokeFunctors(::RCF::InvokeFunctorMap &invokeFunctorMap, DerefPtrT derefPtr)      \
        {                                                                                               \
            ::RCF::registerInvokeFunctors(*this, invokeFunctorMap, derefPtr);                           \
                                                                                                        \
            ::RCF::StubAccess().registerParentInvokeFunctors(                                           \
                (InheritT1 *) NULL,                                                                     \
                *this,                                                                                  \
                invokeFunctorMap,                                                                       \
                derefPtr);                                                                              \
                                                                                                        \
            ::RCF::StubAccess().registerParentInvokeFunctors(                                           \
                (InheritT2 *) NULL,                                                                     \
                *this,                                                                                  \
                invokeFunctorMap,                                                                       \
                derefPtr);                                                                              \
        }                                                                                               \
        void setClientStubPtr(::RCF::ClientStubPtr clientStubPtr)                                       \
        {                                                                                               \
            mClientStubPtr = clientStubPtr;                                                             \
            ::RCF::StubAccess().setClientStubPtr( (InheritT1*) 0, *this);                               \
            ::RCF::StubAccess().setClientStubPtr( (InheritT2*) 0, *this);                               \
        }

#define RCF_BEGIN_IMPL_POSTLUDE(InterfaceT, Name)                                                       \
    public:                                                                                             \
                                                                                                        \
        RcfClient()                                                                                     \
        {}                                                                                              \
                                                                                                        \
        template<typename DerefPtrT>                                                                    \
        RcfClient(                                                                                      \
            ::RCF::ServerStubPtr serverStubPtr,                                                         \
            DerefPtrT derefPtr,                                                                         \
            boost::mpl::true_ *)                                                                        \
        {                                                                                               \
            serverStubPtr->registerInvokeFunctors(*this, derefPtr);                                     \
            mServerStubPtr = serverStubPtr;                                                             \
        }                                                                                               \
                                                                                                        \
        RcfClient(                                                                                      \
            const ::RCF::I_Endpoint &endpoint)                                                          \
        {                                                                                               \
            const std::string &interfaceName = ::RCF::getInterfaceName( (InterfaceT *) NULL);           \
            const std::string &targetName = interfaceName;                                              \
            ::RCF::ClientStubPtr clientStubPtr( new ::RCF::ClientStub(interfaceName, targetName) );     \
            clientStubPtr->setEndpoint(endpoint);                                                       \
            setClientStubPtr(clientStubPtr);                                                            \
        }                                                                                               \
                                                                                                        \
        RcfClient(                                                                                      \
            const ::RCF::I_Endpoint &endpoint,                                                          \
            const std::string &targetName)                                                              \
        {                                                                                               \
            const std::string &interfaceName = ::RCF::getInterfaceName( (InterfaceT *) NULL);           \
            ::RCF::ClientStubPtr clientStubPtr( new ::RCF::ClientStub(interfaceName, targetName) );     \
            clientStubPtr->setEndpoint(endpoint);                                                       \
            setClientStubPtr(clientStubPtr);                                                            \
        }                                                                                               \
                                                                                                        \
        RcfClient(                                                                                      \
            ::RCF::ClientTransportAutoPtr clientTransportAutoPtr)                                       \
        {                                                                                               \
            const std::string &interfaceName = ::RCF::getInterfaceName( (InterfaceT *) NULL);           \
            const std::string &targetName = interfaceName;                                              \
            ::RCF::ClientStubPtr clientStubPtr( new ::RCF::ClientStub(interfaceName, targetName) );     \
            clientStubPtr->setTransport(clientTransportAutoPtr);                                        \
            setClientStubPtr(clientStubPtr);                                                            \
        }                                                                                               \
                                                                                                        \
        RcfClient(                                                                                      \
            ::RCF::ClientTransportAutoPtr clientTransportAutoPtr,                                       \
            const std::string &targetName)                                                              \
        {                                                                                               \
            const std::string &interfaceName = ::RCF::getInterfaceName( (InterfaceT *) NULL);           \
            ::RCF::ClientStubPtr clientStubPtr( new ::RCF::ClientStub(interfaceName, targetName) );     \
            clientStubPtr->setTransport(clientTransportAutoPtr);                                        \
            setClientStubPtr(clientStubPtr);                                                            \
        }                                                                                               \
                                                                                                        \
        RcfClient(                                                                                      \
            const ::RCF::ClientStub &clientStub)                                                        \
        {                                                                                               \
            const std::string &interfaceName = ::RCF::getInterfaceName( (InterfaceT *) NULL);           \
            const std::string &targetName = interfaceName;                                              \
            ::RCF::ClientStubPtr clientStubPtr( new ::RCF::ClientStub(clientStub) );                    \
            clientStubPtr->setInterfaceName(interfaceName);                                             \
            clientStubPtr->setTargetName(targetName);                                                   \
            clientStubPtr->setTargetToken(::RCF::Token());                                              \
            setClientStubPtr(clientStubPtr);                                                            \
        }                                                                                               \
                                                                                                        \
        RcfClient(                                                                                      \
            const ::RCF::ClientStub &clientStub,                                                        \
            const std::string &targetName)                                                              \
        {                                                                                               \
            const std::string &interfaceName = ::RCF::getInterfaceName( (InterfaceT *) NULL);           \
            ::RCF::ClientStubPtr clientStubPtr( new ::RCF::ClientStub(clientStub) );                    \
            clientStubPtr->setInterfaceName(interfaceName);                                             \
            clientStubPtr->setTargetName(targetName);                                                   \
            clientStubPtr->setTargetToken(::RCF::Token());                                              \
            setClientStubPtr(clientStubPtr);                                                            \
        }                                                                                               \
                                                                                                        \
        RcfClient(                                                                                      \
            const ::RCF::I_RcfClient & rhs)                                                             \
        {                                                                                               \
            if (rhs.getClientStubPtr())                                                                 \
            {                                                                                           \
                const std::string &interfaceName = ::RCF::getInterfaceName( (InterfaceT *) NULL);       \
                const std::string &targetName = interfaceName;                                          \
                ::RCF::ClientStubPtr clientStubPtr( new ::RCF::ClientStub(rhs.getClientStub()));        \
                clientStubPtr->setInterfaceName(interfaceName);                                         \
                clientStubPtr->setTargetName(targetName);                                               \
                clientStubPtr->setTargetToken(::RCF::Token());                                          \
                setClientStubPtr(clientStubPtr);                                                        \
            }                                                                                           \
        }                                                                                               \
                                                                                                        \
        ~RcfClient()                                                                                    \
        {                                                                                               \
            if (mClientStubPtr)                                                                         \
            {                                                                                           \
                mClientStubPtr->disconnect();                                                           \
            }                                                                                           \
        }                                                                                               \
                                                                                                        \
        RcfClient &operator=(const RcfClient &rhs)                                                      \
        {                                                                                               \
            if (&rhs != this)                                                                           \
            {                                                                                           \
                if (rhs.mClientStubPtr)                                                                 \
                {                                                                                       \
                    const std::string &interfaceName = ::RCF::getInterfaceName( (InterfaceT *) NULL);   \
                    const std::string &targetName = interfaceName;                                      \
                    ::RCF::ClientStubPtr clientStubPtr( new ::RCF::ClientStub(rhs.getClientStub()));    \
                    clientStubPtr->setInterfaceName(interfaceName);                                     \
                    clientStubPtr->setTargetName(targetName);                                           \
                    clientStubPtr->setTargetToken(::RCF::Token());                                      \
                    setClientStubPtr(clientStubPtr);                                                    \
                }                                                                                       \
                else                                                                                    \
                {                                                                                       \
                    RCF_ASSERT(!rhs.mServerStubPtr);                                                    \
                    mClientStubPtr = rhs.mClientStubPtr;                                                \
                }                                                                                       \
            }                                                                                           \
            return *this;                                                                               \
        }                                                                                               \
                                                                                                        \
        RcfClient &operator=(const ::RCF::I_RcfClient &rhs)                                             \
        {                                                                                               \
            if (rhs.getClientStubPtr())                                                                 \
            {                                                                                           \
                const std::string &interfaceName = ::RCF::getInterfaceName( (InterfaceT *) NULL);       \
                const std::string &targetName = interfaceName;                                          \
                ::RCF::ClientStubPtr clientStubPtr( new ::RCF::ClientStub(rhs.getClientStub()));        \
                clientStubPtr->setInterfaceName(interfaceName);                                         \
                clientStubPtr->setTargetName(targetName);                                               \
                clientStubPtr->setTargetToken(::RCF::Token());                                          \
                setClientStubPtr(clientStubPtr);                                                        \
            }                                                                                           \
            else                                                                                        \
            {                                                                                           \
                RCF_ASSERT(!rhs.getServerStubPtr());                                                    \
                mClientStubPtr.reset();                                                                 \
            }                                                                                           \
            return *this;                                                                               \
        }                                                                                               \
                                                                                                        \
        void swap(RcfClient & rhs)                                                                      \
        {                                                                                               \
            ::RCF::ClientStubPtr clientStubPtr = rhs.mClientStubPtr;                                    \
            ::RCF::ServerStubPtr serverStubPtr = rhs.mServerStubPtr;                                    \
                                                                                                        \
            rhs.mClientStubPtr = mClientStubPtr;                                                        \
            rhs.mServerStubPtr = mServerStubPtr;                                                        \
                                                                                                        \
            mClientStubPtr = clientStubPtr;                                                             \
            mServerStubPtr = serverStubPtr;                                                             \
        }                                                                                               \
                                                                                                        \
    public:                                                                                             \
        ::RCF::ClientStub &getClientStub()                                                              \
        {                                                                                               \
            return *mClientStubPtr;                                                                     \
        }                                                                                               \
                                                                                                        \
        const ::RCF::ClientStub &getClientStub() const                                                  \
        {                                                                                               \
            return *mClientStubPtr;                                                                     \
        }                                                                                               \
                                                                                                        \
        ::RCF::ClientStubPtr getClientStubPtr() const                                                   \
        {                                                                                               \
            return mClientStubPtr;                                                                      \
        }                                                                                               \
                                                                                                        \
        ::RCF::ServerStubPtr getServerStubPtr() const                                                   \
        {                                                                                               \
            return mServerStubPtr;                                                                      \
        }                                                                                               \
                                                                                                        \
    private:                                                                                            \
        ::RCF::ServerStub &getServerStub()                                                              \
        {                                                                                               \
            return *mServerStubPtr;                                                                     \
        }                                                                                               \
                                                                                                        \
    public:                                                                                             \
        template<typename Archive>                                                                      \
        void serialize(Archive &ar)                                                                     \
        {                                                                                               \
            ::RCF::StubAccess().serialize(ar, *this);                                                   \
        }                                                                                               \
                                                                                                        \
        template<typename Archive>                                                                      \
        void serialize(Archive &ar, const unsigned int)                                                 \
        {                                                                                               \
            ::RCF::StubAccess().serialize(ar, *this, 0);                                                \
        }                                                                                               \
                                                                                                        \
    private:                                                                                            \
                                                                                                        \
        template<typename N, typename T>                                                                \
        void invoke(                                                                                    \
            const N &,                                                                                  \
            ::RCF::RcfSession &,                                                                        \
            const T &)                                                                                  \
        {                                                                                               \
            RCF_THROW(::RCF::Exception(RCF::_RcfError_FnId(N::value)))(N::value);                       \
        }                                                                                               \
                                                                                                        \
        ::RCF::ClientStubPtr            mClientStubPtr;                                                 \
        ::RCF::ServerStubPtr            mServerStubPtr;                                                 \
                                                                                                        \
        typedef ::RCF::Void             V;                                                              \
        typedef RcfClient< InterfaceT > ThisT;                                                          \
        typedef ::RCF::Dummy<ThisT>     DummyThisT;                                                     \
                                                                                                        \
        friend class ::RCF::StubAccess;                                                                 \
        friend ::RCF::default_ RCF_make_next_dispatch_id_func(DummyThisT *, ThisT *,...);               \
    public:                                                                                             \
        typedef InterfaceT              Interface;
        


#define RCF_END( InterfaceT )                                                                           \
    };

#define RCF_METHOD_PLACEHOLDER()                                                                        \
    RCF_METHOD_PLACEHOLDER_(RCF_MAKE_UNIQUE_ID(PlaceHolder, V0))

#define RCF_METHOD_PLACEHOLDER_(id)                                                                     \
    public:                                                                                             \
        RCF_MAKE_NEXT_DISPATCH_ID(id);                                                                  \
    private:

#define RCF_METHOD_R0(R,func)                                                                           \
            RCF_METHOD_R0_(R,func, RCF_MAKE_UNIQUE_ID(func, R0))

#define RCF_METHOD_R0_(R, func, id)                                                                     \
        public:                                                                                         \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<R > func()                                                                \
            {                                                                                           \
                return func(::RCF::CallOptions());                                                      \
            }                                                                                           \
            ::RCF::FutureImpl<R > func(                                                                 \
                const ::RCF::CallOptions &callOptions)                                                  \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<R >(                                                             \
                    ::RCF::AllocateClientParameters<R,V,V,V,V,V,V,V,V >()(                              \
                        getClientStub(), V(), V(), V(), V(), V(), V(), V(), V()).r.get(),               \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<R > &p =                                                        \
                    ::RCF::AllocateServerParameters<R >()(session);                                     \
                p.r.set(session.getAutoSend(), t.func());                                               \
            }


#define RCF_METHOD_R1(R,func,A1)                                                                        \
            RCF_METHOD_R1_(R,func,A1, RCF_MAKE_UNIQUE_ID(func, R1))

#define RCF_METHOD_R1_(R,func,A1,id)                                                                    \
        public:                                                                                         \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<R > func(A1 a1)                                                           \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1);                                                  \
            }                                                                                           \
            ::RCF::FutureImpl<R > func(                                                                 \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1)                                                                                  \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<R >(                                                             \
                    ::RCF::AllocateClientParameters<R,A1,V,V,V,V,V,V,V >()(                             \
                        getClientStub(), a1, V(), V(), V(), V(), V(), V(), V()).r.get(),                \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<R, A1 > &p =                                                    \
                    ::RCF::AllocateServerParameters<R, A1 >()(session);                                 \
                p.r.set(session.getAutoSend(), t.func(p.a1.get()));                                     \
            }

#define RCF_METHOD_R2(R,func,A1,A2)                                                                     \
            RCF_METHOD_R2_(R,func,A1,A2, RCF_MAKE_UNIQUE_ID(func, R2))

#define RCF_METHOD_R2_(R,func,A1,A2,id)                                                                 \
        public:                                                                                         \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<R > func(A1 a1, A2 a2)                                                    \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2);                                              \
            }                                                                                           \
            ::RCF::FutureImpl<R > func(                                                                 \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2)                                                                           \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<R >(                                                             \
                    ::RCF::AllocateClientParameters<R,A1,A2,V,V,V,V,V,V >()(                            \
                        getClientStub(), a1, a2, V(), V(), V(), V(), V(), V()).r.get(),                 \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<R, A1, A2 > &p =                                                \
                    ::RCF::AllocateServerParameters<R, A1, A2 >()(session);                             \
                p.r.set(session.getAutoSend(), t.func(p.a1.get(), p.a2.get()));                         \
            }


#define RCF_METHOD_R3(R,func,A1,A2,A3)                                                                  \
            RCF_METHOD_R3_(R,func,A1,A2,A3, RCF_MAKE_UNIQUE_ID(func, R3))

#define RCF_METHOD_R3_(R,func,A1,A2,A3,id)                                                              \
        public:                                                                                         \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<R > func(A1 a1, A2 a2, A3 a3)                                             \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2, a3);                                          \
            }                                                                                           \
            ::RCF::FutureImpl<R > func(                                                                 \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2, A3 a3)                                                                    \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<R >(                                                             \
                    ::RCF::AllocateClientParameters<R,A1,A2,A3,V,V,V,V,V >()(                           \
                        getClientStub(), a1, a2, a3, V(), V(), V(), V(), V()).r.get(),                  \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<R, A1, A2, A3 > &p =                                            \
                    ::RCF::AllocateServerParameters<R, A1, A2, A3 >()(session);                         \
                p.r.set(session.getAutoSend(), t.func(p.a1.get(), p.a2.get(), p.a3.get()));             \
            }

#define RCF_METHOD_R4(R,func,A1,A2,A3,A4)                                                               \
            RCF_METHOD_R4_(R,func,A1,A2,A3,A4, RCF_MAKE_UNIQUE_ID(func, R4))

#define RCF_METHOD_R4_(R,func,A1,A2,A3,A4,id)                                                           \
        public:                                                                                         \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<R > func(A1 a1, A2 a2, A3 a3, A4 a4)                                      \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2, a3, a4);                                      \
            }                                                                                           \
            ::RCF::FutureImpl<R > func(                                                                 \
                const ::RCF::CallOptions &callOptions, A1 a1, A2 a2, A3 a3, A4 a4)                      \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<R >(                                                             \
                    ::RCF::AllocateClientParameters<R,A1,A2,A3,A4,V,V,V,V >()(                          \
                        getClientStub(), a1, a2, a3, a4, V(), V(), V(), V()).r.get(),                   \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<R, A1, A2, A3, A4 > &p =                                        \
                    ::RCF::AllocateServerParameters<R, A1, A2, A3, A4 >()(session);                     \
                p.r.set(session.getAutoSend(), t.func(p.a1.get(), p.a2.get(), p.a3.get(), p.a4.get())); \
            }

#define RCF_METHOD_R5(R,func,A1,A2,A3,A4,A5)                                                            \
            RCF_METHOD_R5_(R,func,A1,A2,A3,A4,A5, RCF_MAKE_UNIQUE_ID(func, R5))

#define RCF_METHOD_R5_(R,func,A1,A2,A3,A4,A5, id)                                                       \
        public:                                                                                         \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<R > func(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)                               \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2, a3, a4, a5);                                  \
            }                                                                                           \
            ::RCF::FutureImpl<R > func(                                                                 \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)                                                      \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<R >(                                                             \
                    ::RCF::AllocateClientParameters<R,A1,A2,A3,A4,A5,V,V,V >()(                         \
                        getClientStub(), a1, a2, a3, a4, a5, V(), V(), V()).r.get(),                    \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<R, A1, A2, A3, A4, A5 > &p =                                    \
                    ::RCF::AllocateServerParameters<R, A1, A2, A3, A4, A5 >()(session);                 \
                p.r.set(session.getAutoSend(), t.func(                                                  \
                    p.a1.get(), p.a2.get(), p.a3.get(), p.a4.get(), p.a5.get()));                       \
            }

#define RCF_METHOD_R6(R,func,A1,A2,A3,A4,A5,A6)                                                         \
            RCF_METHOD_R6_(R,func,A1,A2,A3,A4,A5,A6, RCF_MAKE_UNIQUE_ID(func, R6))

#define RCF_METHOD_R6_(R,func,A1,A2,A3,A4,A5,A6, id)                                                    \
        public:                                                                                         \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<R > func(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)                        \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2, a3, a4, a5, a6);                              \
            }                                                                                           \
            ::RCF::FutureImpl<R > func(                                                                 \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)                                               \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<R >(                                                             \
                    ::RCF::AllocateClientParameters<R,A1,A2,A3,A4,A5,A6,V,V >()(                        \
                        getClientStub(), a1, a2, a3, a4, a5, a6, V(), V()).r.get(),                     \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<R, A1, A2, A3, A4, A5, A6 > &p =                                \
                    ::RCF::AllocateServerParameters<R, A1, A2, A3, A4, A5, A6 >()(session);             \
                p.r.set(session.getAutoSend(), t.func(                                                  \
                    p.a1.get(), p.a2.get(), p.a3.get(), p.a4.get(), p.a5.get(), p.a6.get()));           \
            }

#define RCF_METHOD_R7(R,func,A1,A2,A3,A4,A5,A6,A7)                                                      \
            RCF_METHOD_R7_(R,func,A1,A2,A3,A4,A5,A6,A7, RCF_MAKE_UNIQUE_ID(func, R7))

#define RCF_METHOD_R7_(R,func,A1,A2,A3,A4,A5,A6,A7, id)                                                 \
        public:                                                                                         \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<R > func(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)                 \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2, a3, a4, a5, a6, a7);                          \
            }                                                                                           \
            ::RCF::FutureImpl<R > func(                                                                 \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)                                        \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<R >(                                                             \
                    ::RCF::AllocateClientParameters<R,A1,A2,A3,A4,A5,A6,A7,V >()(                       \
                        getClientStub(), a1, a2, a3, a4, a5, a6, a7, V()).r.get(),                      \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<R, A1, A2, A3, A4, A5, A6, A7 > &p =                            \
                    ::RCF::AllocateServerParameters<R, A1, A2, A3, A4, A5, A6, A7 >()(session);         \
                p.r.set(session.getAutoSend(), t.func(                                                  \
                    p.a1.get(), p.a2.get(), p.a3.get(), p.a4.get(),                                     \
                    p.a5.get(), p.a6.get(), p.a7.get()));                                               \
            }

#define RCF_METHOD_R8(R,func,A1,A2,A3,A4,A5,A6,A7,A8)                                                   \
            RCF_METHOD_R8_(R,func,A1,A2,A3,A4,A5,A6,A7,A8, RCF_MAKE_UNIQUE_ID(func, R8))

#define RCF_METHOD_R8_(R,func,A1,A2,A3,A4,A5,A6,A7,A8, id)                                              \
        public:                                                                                         \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<R > func(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)          \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2, a3, a4, a5, a6, a7, a8);                      \
            }                                                                                           \
            ::RCF::FutureImpl<R > func(                                                                 \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)                                 \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<R >(                                                             \
                    ::RCF::AllocateClientParameters<R,A1,A2,A3,A4,A5,A6,A7,A8 >()(                      \
                        getClientStub(), a1, a2, a3, a4, a5, a6, a7, a8).r.get(),                       \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<R, A1, A2, A3, A4, A5, A6, A7, A8 > &p =                        \
                    ::RCF::AllocateServerParameters<R, A1, A2, A3, A4, A5, A6, A7, A8 >()(session);     \
                p.r.set(session.getAutoSend(), t.func(                                                  \
                    p.a1.get(), p.a2.get(), p.a3.get(), p.a4.get(),                                     \
                    p.a5.get(), p.a6.get(), p.a7.get(), p.a8.get()));                                   \
            }

#define RCF_METHOD_V0(R,func)                                                                           \
            RCF_METHOD_V0_(R,func, RCF_MAKE_UNIQUE_ID(func, V0))

#define RCF_METHOD_V0_(R,func, id)                                                                      \
        public:                                                                                         \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));                                    \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<V> func()                                                                 \
            {                                                                                           \
                return func(::RCF::CallOptions());                                                      \
            }                                                                                           \
            ::RCF::FutureImpl<V> func(                                                                  \
                const ::RCF::CallOptions &callOptions)                                                  \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<V>(                                                              \
                    ::RCF::AllocateClientParameters<V,V,V,V,V,V,V,V,V >()(                              \
                        getClientStub(), V(), V(), V(), V(), V(), V(), V(), V()).r.get(),               \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<V > &p =                                                        \
                    ::RCF::AllocateServerParameters<V >()(session);                                     \
                RCF_UNUSED_VARIABLE(p);                                                                 \
                t.func();                                                                               \
            }

#define RCF_METHOD_V1(R,func,A1)                                                                        \
            RCF_METHOD_V1_(R,func,A1, RCF_MAKE_UNIQUE_ID(func, V1))

#define RCF_METHOD_V1_(R,func,A1, id)                                                                   \
        public:                                                                                         \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));                                    \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<V> func(A1 a1)                                                            \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1);                                                  \
            }                                                                                           \
            ::RCF::FutureImpl<V> func(                                                                  \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1)                                                                                  \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<V>(                                                              \
                    ::RCF::AllocateClientParameters<V,A1,V,V,V,V,V,V,V >()(                             \
                        getClientStub(), a1, V(), V(), V(), V(), V(), V(), V()).r.get(),                \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<V, A1 > &p =                                                    \
                    ::RCF::AllocateServerParameters<V, A1 >()(session);                                 \
                RCF_UNUSED_VARIABLE(p);                                                                 \
                t.func(p.a1.get());                                                                     \
            }


#define RCF_METHOD_V2(R,func,A1,A2)                                                                     \
            RCF_METHOD_V2_(R,func,A1,A2, RCF_MAKE_UNIQUE_ID(func, V2))

#define RCF_METHOD_V2_(R,func,A1,A2, id)                                                                \
        public:                                                                                         \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));                                    \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<V> func(A1 a1, A2 a2)                                                     \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2);                                              \
            }                                                                                           \
            ::RCF::FutureImpl<V> func(                                                                  \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2)                                                                           \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<V>(                                                              \
                    ::RCF::AllocateClientParameters<V,A1,A2,V,V,V,V,V,V >()(                            \
                        getClientStub(), a1, a2, V(), V(), V(), V(), V(), V()).r.get(),                 \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<V, A1, A2 > &p =                                                \
                    ::RCF::AllocateServerParameters<V, A1, A2 >()(session);                             \
                RCF_UNUSED_VARIABLE(p);                                                                 \
                t.func(p.a1.get(), p.a2.get());                                                         \
            }


#define RCF_METHOD_V3(R,func,A1,A2,A3)                                                                  \
            RCF_METHOD_V3_(R,func,A1,A2,A3, RCF_MAKE_UNIQUE_ID(func, V3))

#define RCF_METHOD_V3_(R,func,A1,A2,A3, id)                                                             \
        public:                                                                                         \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));                                    \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<V> func(A1 a1, A2 a2, A3 a3)                                              \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2, a3);                                          \
            }                                                                                           \
            ::RCF::FutureImpl<V> func(                                                                  \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2, A3 a3)                                                                    \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<V>(                                                              \
                    ::RCF::AllocateClientParameters<V,A1,A2,A3,V,V,V,V,V >()(                           \
                        getClientStub(), a1, a2, a3, V(), V(), V(), V(), V()).r.get(),                  \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<V, A1, A2, A3 > &p =                                            \
                    ::RCF::AllocateServerParameters<V, A1, A2, A3 >()(session);                         \
                RCF_UNUSED_VARIABLE(p);                                                                 \
                t.func(p.a1.get(), p.a2.get(), p.a3.get());                                             \
            }

#define RCF_METHOD_V4(R,func,A1,A2,A3,A4)                                                               \
            RCF_METHOD_V4_(R,func,A1,A2,A3,A4, RCF_MAKE_UNIQUE_ID(func, V4))

#define RCF_METHOD_V4_(R,func,A1,A2,A3,A4, id)                                                          \
        public:                                                                                         \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));                                    \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<V> func(A1 a1, A2 a2, A3 a3, A4 a4)                                       \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2, a3, a4);                                      \
            }                                                                                           \
            ::RCF::FutureImpl<V> func(                                                                  \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2, A3 a3, A4 a4)                                                             \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<V>(                                                              \
                    ::RCF::AllocateClientParameters<V,A1,A2,A3,A4,V,V,V,V >()(                          \
                        getClientStub(), a1, a2, a3, a4, V(), V(), V(), V()).r.get(),                   \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<V, A1, A2, A3, A4 > &p =                                        \
                    ::RCF::AllocateServerParameters<V, A1, A2, A3, A4 >()(session);                     \
                RCF_UNUSED_VARIABLE(p);                                                                 \
                t.func(p.a1.get(), p.a2.get(), p.a3.get(), p.a4.get());                                 \
            }

#define RCF_METHOD_V5(R,func,A1,A2,A3,A4,A5)                                                            \
            RCF_METHOD_V5_(R,func,A1,A2,A3,A4,A5, RCF_MAKE_UNIQUE_ID(func, V5))

#define RCF_METHOD_V5_(R,func,A1,A2,A3,A4,A5, id)                                                       \
        public:                                                                                         \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));                                    \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<V> func(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)                                \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2, a3, a4, a5);                                  \
            }                                                                                           \
            ::RCF::FutureImpl<V> func(                                                                  \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)                                                      \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<V>(                                                              \
                    ::RCF::AllocateClientParameters<V,A1,A2,A3,A4,A5,V,V,V >()(                         \
                        getClientStub(), a1, a2, a3, a4, a5, V(), V(), V()).r.get(),                    \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<V, A1, A2, A3, A4, A5 > &p =                                    \
                    ::RCF::AllocateServerParameters<V, A1, A2, A3, A4, A5 >()(session);                 \
                RCF_UNUSED_VARIABLE(p);                                                                 \
                t.func(p.a1.get(), p.a2.get(), p.a3.get(), p.a4.get(), p.a5.get());                     \
            }

#define RCF_METHOD_V6(R,func,A1,A2,A3,A4,A5,A6)                                                         \
            RCF_METHOD_V6_(R,func,A1,A2,A3,A4,A5,A6, RCF_MAKE_UNIQUE_ID(func, V6))

#define RCF_METHOD_V6_(R,func,A1,A2,A3,A4,A5,A6, id)                                                    \
        public:                                                                                         \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));                                    \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<V> func(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)                         \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2, a3, a4, a5, a6);                              \
            }                                                                                           \
            ::RCF::FutureImpl<V> func(                                                                  \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)                                               \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<V>(                                                              \
                    ::RCF::AllocateClientParameters<V,A1,A2,A3,A4,A5,A6,V,V >()(                        \
                        getClientStub(), a1, a2, a3, a4, a5, a6, V(), V()).r.get(),                     \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<V, A1, A2, A3, A4, A5, A6 > &p =                                \
                    ::RCF::AllocateServerParameters<V, A1, A2, A3, A4, A5, A6 >()(session);             \
                RCF_UNUSED_VARIABLE(p);                                                                 \
                t.func(p.a1.get(), p.a2.get(), p.a3.get(), p.a4.get(), p.a5.get(), p.a6.get());         \
            }

#define RCF_METHOD_V7(R,func,A1,A2,A3,A4,A5,A6,A7)                                                      \
            RCF_METHOD_V7_(R,func,A1,A2,A3,A4,A5,A6,A7, RCF_MAKE_UNIQUE_ID(func, V7))

#define RCF_METHOD_V7_(R,func,A1,A2,A3,A4,A5,A6,A7, id)                                                 \
        public:                                                                                         \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));                                    \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<V> func(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)                  \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2, a3, a4, a5, a6, a7);                          \
            }                                                                                           \
            ::RCF::FutureImpl<V> func(                                                                  \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)                                        \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<V>(                                                              \
                    ::RCF::AllocateClientParameters<V,A1,A2,A3,A4,A5,A6,A7,V >()(                       \
                        getClientStub(), a1, a2, a3, a4, a5, a6, a7, V()).r.get(),                      \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<V, A1, A2, A3, A4, A5, A6, A7 > &p =                            \
                    ::RCF::AllocateServerParameters<V, A1, A2, A3, A4, A5, A6, A7 >()(session);         \
                RCF_UNUSED_VARIABLE(p);                                                                 \
                t.func(                                                                                 \
                    p.a1.get(), p.a2.get(), p.a3.get(), p.a4.get(),                                     \
                    p.a5.get(), p.a6.get(), p.a7.get());                                                \
            }

#define RCF_METHOD_V8(R,func,A1,A2,A3,A4,A5,A6,A7,A8)                                                   \
            RCF_METHOD_V8_(R,func,A1,A2,A3,A4,A5,A6,A7,A8, RCF_MAKE_UNIQUE_ID(func, V8))

#define RCF_METHOD_V8_(R,func,A1,A2,A3,A4,A5,A6,A7,A8, id)                                              \
        public:                                                                                         \
            BOOST_STATIC_ASSERT(( boost::is_same<R, void>::value ));                                    \
            RCF_MAKE_NEXT_DISPATCH_ID(id);                                                              \
            ::RCF::FutureImpl<V> func(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)           \
            {                                                                                           \
                return func(::RCF::CallOptions(), a1, a2, a3, a4, a5, a6, a7, a8);                      \
            }                                                                                           \
            ::RCF::FutureImpl<V> func(                                                                  \
                const ::RCF::CallOptions &callOptions,                                                  \
                A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)                                 \
            {                                                                                           \
                getClientStub().setAsync(false);                                                        \
                return RCF::FutureImpl<V>(                                                              \
                    ::RCF::AllocateClientParameters<V,A1,A2,A3,A4,A5,A6,A7,A8 >()(                      \
                        getClientStub(), a1, a2, a3, a4, a5, a6, a7, a8).r.get(),                       \
                    getClientStub(),                                                                    \
                    ::RCF::getInterfaceName( (Interface *) NULL),                                       \
                    id::value,                                                                          \
                    callOptions.apply(getClientStub()));                                                \
            }                                                                                           \
                                                                                                        \
        private:                                                                                        \
            template<typename T>                                                                        \
            void invoke(                                                                                \
                const id &,                                                                             \
                ::RCF::RcfSession &session,                                                             \
                T &t)                                                                                   \
            {                                                                                           \
                ::RCF::ServerParameters<V, A1, A2, A3, A4, A5, A6, A7, A8 > &p =                        \
                    ::RCF::AllocateServerParameters<V, A1, A2, A3, A4, A5, A6, A7, A8 >()(session);     \
                RCF_UNUSED_VARIABLE(p);                                                                 \
                t.func(                                                                                 \
                    p.a1.get(), p.a2.get(), p.a3.get(), p.a4.get(),                                     \
                    p.a5.get(), p.a6.get(), p.a7.get(), p.a8.get());                                    \
            }


// RCF_MAKE_UNIQUE_ID

BOOST_STATIC_ASSERT( sizeof(RCF::defined_) != sizeof(RCF::default_));

#define RCF_MAKE_UNIQUE_ID(func, sig)                       RCF_MAKE_UNIQUE_ID_(func, sig, __LINE__)
#define RCF_MAKE_UNIQUE_ID_(func, sig, __LINE__)            RCF_MAKE_UNIQUE_ID__(func, sig, __LINE__)
#define RCF_MAKE_UNIQUE_ID__(func, sig, Line)               rcf_unique_id_##func##_##sig##_##Line

#define RCF_MAKE_NEXT_DISPATCH_ID(next_dispatch_id)                                                                                                                             \
    typedef                                                                                                                                                                     \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_< 0> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_< 1> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_< 2> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_< 3> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_< 4> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_< 5> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_< 6> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_< 7> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_< 8> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_< 9> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<10> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<11> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<12> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<13> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<14> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<15> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<16> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<17> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<18> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<19> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<20> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<21> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<22> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<23> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<24> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<25> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<26> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<27> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<28> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<29> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<30> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<31> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<32> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<33> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::if_< boost::mpl::bool_< (sizeof(RCF_make_next_dispatch_id_func((DummyThisT *) 0, (ThisT *) 0, (boost::mpl::int_<34> *) 0)) == sizeof(RCF::defined_)) >,         \
    boost::mpl::int_<35>,                                                                                                                                                       \
    boost::mpl::int_<34> >::type,                                                                                                                                               \
    boost::mpl::int_<33> >::type,                                                                                                                                               \
    boost::mpl::int_<32> >::type,                                                                                                                                               \
    boost::mpl::int_<31> >::type,                                                                                                                                               \
    boost::mpl::int_<30> >::type,                                                                                                                                               \
    boost::mpl::int_<29> >::type,                                                                                                                                               \
    boost::mpl::int_<28> >::type,                                                                                                                                               \
    boost::mpl::int_<27> >::type,                                                                                                                                               \
    boost::mpl::int_<26> >::type,                                                                                                                                               \
    boost::mpl::int_<25> >::type,                                                                                                                                               \
    boost::mpl::int_<24> >::type,                                                                                                                                               \
    boost::mpl::int_<23> >::type,                                                                                                                                               \
    boost::mpl::int_<22> >::type,                                                                                                                                               \
    boost::mpl::int_<21> >::type,                                                                                                                                               \
    boost::mpl::int_<20> >::type,                                                                                                                                               \
    boost::mpl::int_<19> >::type,                                                                                                                                               \
    boost::mpl::int_<18> >::type,                                                                                                                                               \
    boost::mpl::int_<17> >::type,                                                                                                                                               \
    boost::mpl::int_<16> >::type,                                                                                                                                               \
    boost::mpl::int_<15> >::type,                                                                                                                                               \
    boost::mpl::int_<14> >::type,                                                                                                                                               \
    boost::mpl::int_<13> >::type,                                                                                                                                               \
    boost::mpl::int_<12> >::type,                                                                                                                                               \
    boost::mpl::int_<11> >::type,                                                                                                                                               \
    boost::mpl::int_<10> >::type,                                                                                                                                               \
    boost::mpl::int_< 9> >::type,                                                                                                                                               \
    boost::mpl::int_< 8> >::type,                                                                                                                                               \
    boost::mpl::int_< 7> >::type,                                                                                                                                               \
    boost::mpl::int_< 6> >::type,                                                                                                                                               \
    boost::mpl::int_< 5> >::type,                                                                                                                                               \
    boost::mpl::int_< 4> >::type,                                                                                                                                               \
    boost::mpl::int_< 3> >::type,                                                                                                                                               \
    boost::mpl::int_< 2> >::type,                                                                                                                                               \
    boost::mpl::int_< 1> >::type,                                                                                                                                               \
    boost::mpl::int_< 0> >::type next_dispatch_id;                                                                                                                              \
    friend RCF::defined_ RCF_make_next_dispatch_id_func(DummyThisT *, ThisT *, next_dispatch_id *)

#endif // ! INCLUDE_RCF_IDL_HPP
