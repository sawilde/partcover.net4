
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_OBJECTFACTORYSERVICE_HPP
#define INCLUDE_RCF_OBJECTFACTORYSERVICE_HPP

#include <map>
#include <string>
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/GetInterfaceName.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/Service.hpp>
#include <RCF/StubFactory.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/Token.hpp>
#include <RCF/TypeTraits.hpp>

namespace RCF {

    class RcfServer;
    class StubEntry;
    class I_StubFactory;
    class I_RcfClient;
    class Token;
    class TokenMapped;

    typedef boost::shared_ptr<StubEntry> StubEntryPtr;
    typedef boost::shared_ptr<I_StubFactory> StubFactoryPtr;
    typedef boost::shared_ptr<TokenMapped> TokenMappedPtr;

    class RCF_EXPORT StubFactoryRegistry
    {
    public:

        StubFactoryRegistry();

        /// Binds an object factory to a name.
        /// \param name Name by which clients will access the object factory.
        /// \return true if successful, otherwise false.

#if !defined(_MSC_VER) || _MSC_VER > 1200

        template<typename I1, typename ImplementationT>
        bool bind(const std::string &name_ = "")
        {
            return bind( (I1 *) NULL, (ImplementationT **) NULL, name_);
        }

        template<typename I1, typename I2, typename ImplementationT>
        bool bind(const std::string &name_ = "")
        {
            return bind( (I1 *) NULL, (I2 *) NULL, (ImplementationT **) NULL, name_);
        }

        template<typename I1, typename I2, typename I3, typename ImplementationT>
        bool bind(const std::string &name_ = "")
        {
            return bind( (I1 *) NULL, (I2 *) NULL, (I3 *) NULL, (ImplementationT **) NULL, name_);
        }

        template<typename I1, typename I2, typename I3, typename I4, typename ImplementationT>
        bool bind(const std::string &name_ = "")
        {
            return bind( (I1 *) NULL, (I2 *) NULL, (I3 *) NULL, (I4 *) NULL, (ImplementationT **) NULL, name_);
        }

#endif

        template<typename I1, typename ImplementationT>
        bool bind(I1 *, ImplementationT **, const std::string &name_ = "")
        {
            const std::string &name = (name_ == "") ?
                getInterfaceName((I1 *) NULL) :
                name_;

            StubFactoryPtr stubFactoryPtr(
                new RCF::StubFactory_1<ImplementationT, I1>());

            std::string desc;
            return insertStubFactory(name, desc, stubFactoryPtr);
        }

        template<typename I1, typename I2, typename ImplementationT>
        bool bind(I1 *, I2 *, ImplementationT **, const std::string &name_ = "")
        {
            const std::string &name = (name_ == "") ?
                getInterfaceName((I1 *) NULL) :
                name_;

            StubFactoryPtr stubFactoryPtr(
                new RCF::StubFactory_2<ImplementationT, I1, I2>());

            std::string desc;
            return insertStubFactory(name, desc, stubFactoryPtr);
        }

        template<typename I1, typename I2, typename I3, typename ImplementationT>
        bool bind(I1 *, I2 *, I3 *, ImplementationT **, const std::string &name_ = "")
        {
            const std::string &name = (name_ == "") ?
                getInterfaceName((I1 *) NULL) :
                name_;

            StubFactoryPtr stubFactoryPtr(
                new RCF::StubFactory_3<ImplementationT, I1, I2, I3>());

            std::string desc;
            return insertStubFactory(name, desc, stubFactoryPtr);
        }

        template<typename I1, typename I2, typename I3, typename I4, typename ImplementationT>
        bool bind(I1 *, I2 *, I3 *, I4 *, ImplementationT **, const std::string &name_ = "")
        {
            const std::string &name = (name_ == "") ?
                getInterfaceName((I1 *) NULL) :
                name_;

            StubFactoryPtr stubFactoryPtr(
                new RCF::StubFactory_4<ImplementationT, I1, I2, I3, I4>());

            std::string desc;
            return insertStubFactory(name, desc, stubFactoryPtr);
        }

    protected:

        bool            insertStubFactory(
                            const std::string &objectName,
                            const std::string &desc,
                            StubFactoryPtr stubFactoryPtr);

        bool            removeStubFactory(
                            const std::string &objectName);

        StubFactoryPtr  getStubFactory(
                            const std::string &objectName);

    private:

        typedef std::map<
            std::string, 
            StubFactoryPtr>             StubFactoryMap;

        ReadWriteMutex                  mStubFactoryMapMutex;
        StubFactoryMap                  mStubFactoryMap;

    };

    /// Service allowing remote clients to create objects on the server side,
    /// as opposed to just calling methods on pre-existing objects.
    class RCF_EXPORT ObjectFactoryService :
        public I_Service,
        public StubFactoryRegistry,
        boost::noncopyable
    {
    public:

        /// Constructor.
        /// \param numberOfTokens Maximum number of tokens, thereby also maximum number of created objects.
        /// \param objectTimeoutS Duration of time, in seconds, which must pass after a client invocation, before a created object may be deleted.
        ObjectFactoryService(
            unsigned int    numberOfTokens,
            unsigned int    objectTimeoutS,
            unsigned int    cleanupIntervalS = 30,
            float           cleanupThreshold = .67);

        /// Remotely accessible function, via I_ObjectFactory, allows a client to request the creation of an object.
        /// \param objectName Name of the type of the object to be created.
        /// \param token Token assigned to the created object, if the object was created.
        /// \return true if successful, otherwise false.
        boost::int32_t createObject(const std::string &objectName, Token &token);
        boost::int32_t deleteObject(const Token &token);

        boost::int32_t addObject(TokenMappedPtr tokenMappedPtr, Token &token);

        boost::int32_t createSessionObject(const std::string &objectName);
        boost::int32_t deleteSessionObject();

        StubEntryPtr    getStubEntryPtr(const Token &token);
        TokenMappedPtr  getTokenMappedPtr(const Token & token);
        
    private:
        void            onServiceAdded(RcfServer &server);
        void            onServiceRemoved(RcfServer &server);
        void            onServerStart(RcfServer &);
        void            onServerStop(RcfServer &);
        void            stopCleanup();
        bool            cycleCleanup(int timeoutMs, const volatile bool &stopFlag);
        void            cleanupStubMap(unsigned int timeoutS);

        typedef std::map<
            Token, 
            std::pair<
                MutexPtr, 
                TokenMappedPtr> >         StubMap;

        // TokenFactory is internally synchronized
        TokenFactory                    mTokenFactory;

        unsigned int                    mClientStubTimeoutS;
        Mutex                           mCleanupThresholdMutex;
        Condition                       mCleanupThresholdCondition;
        unsigned int                    mCleanupIntervalS;
        float                           mCleanupThreshold;

        ReadWriteMutex                  mStubMapMutex;
        StubMap                         mStubMap;

        volatile bool                   mStopFlag;

    public:

#if !defined(_MSC_VER) || _MSC_VER > 1200

        template<typename I1, typename T>
        Token addObject(boost::shared_ptr<T> tPtr)
        {
            typedef typename BindDirect<T>::type type;
            return addObject( (I1 *) 0, tPtr, (type *) NULL);
        }

        template<typename I1, typename T>
        Token addObject(boost::weak_ptr<T> tWeakPtr)
        {
            typedef typename BindDirect<T>::type type;
            return addObject( (I1 *) 0, tWeakPtr, (type *) NULL);
        }

        template<typename I1, typename T>
        Token addObject(std::auto_ptr<T> tAutoPtr)
        {
            typedef typename BindDirect<T>::type type;
            return addObject( (I1 *) 0, tAutoPtr, (type *) NULL);
        }

        template<typename I1, typename T>
        Token addObject(T & t)
        {
            typedef typename BindDirect<T>::type type;
            return addObject( (I1 *) 0, t, (type *) NULL);
        }

#endif

        template<typename I1, typename T>
        Token addObject(I1 *, boost::shared_ptr<T> tPtr, boost::mpl::false_ *)
        {
            boost::shared_ptr< RCF::I_Deref<T> > derefPtr(
                new RCF::DerefSharedPtr<T>(tPtr) );

            RCF::RcfClientPtr rcfClientPtr =
                createServerStub( (I1 *) NULL, (T *) NULL, derefPtr);

            return addObjectImpl(rcfClientPtr);
        }

        template<typename I1, typename T>
        Token addObject(I1 *, boost::weak_ptr<T> tWeakPtr, boost::mpl::false_ *)
        {
            boost::shared_ptr< RCF::I_Deref<T> > derefPtr(
                new RCF::DerefWeakPtr<T>(tWeakPtr) );

            RCF::RcfClientPtr rcfClientPtr =
                createServerStub( (I1 *) NULL, (T *) NULL, derefPtr);

            return addObjectImpl(rcfClientPtr);
        }

        template<typename I1, typename T>
        Token addObject(I1 *, std::auto_ptr<T> tAutoPtr, boost::mpl::false_ *)
        {
            boost::shared_ptr< RCF::I_Deref<T> > derefPtr(
                new RCF::DerefAutoPtr<T>(tAutoPtr) );

            RCF::RcfClientPtr rcfClientPtr =
                createServerStub( (I1 *) NULL, (T *) NULL, derefPtr);

            return addObjectImpl(rcfClientPtr);
        }

        template<typename I1, typename T>
        Token addObject(I1 *, T & t, boost::mpl::true_ *)
        {
            boost::shared_ptr< RCF::I_Deref<T> > derefPtr(
                new RCF::DerefObj<T>(t) );

            RCF::RcfClientPtr rcfClientPtr =
                createServerStub( (I1 *) NULL, (T *) NULL, derefPtr);

            return addObjectImpl(rcfClientPtr);
        }

        template<typename I1, typename T>
        Token addObject(I1 *, T & t)
        {
            typedef typename BindDirect<T>::type type;
            return addObject( (I1 *) 0, t, (type *) NULL);
        }

        Token addObjectImpl(RcfClientPtr rcfClientPtr);
    };

    typedef boost::shared_ptr<ObjectFactoryService> 
        ObjectFactoryServicePtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_OBJECTFACTORYSERVICE_HPP
