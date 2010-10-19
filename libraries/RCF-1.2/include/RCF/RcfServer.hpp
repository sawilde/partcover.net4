
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_RCFSERVER_HPP
#define INCLUDE_RCF_RCFSERVER_HPP

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/weak_ptr.hpp>

#include <RCF/CheckRtti.hpp>
#include <RCF/Export.hpp>
#include <RCF/GetInterfaceName.hpp>
#include <RCF/RcfClient.hpp>
#include <RCF/ServerStub.hpp>
#include <RCF/ServerTransport.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/ThreadManager.hpp>

#if !defined(RCF_NO_DIAGNOSTIC_WARNINGS)
#if !defined(BOOST_WINDOWS) && !defined(RCF_USE_BOOST_ASIO) && defined(__GNUC__)
#warning "Warning: On non-Windows platforms, RCF server-side components require Boost.Asio. To configure RCF to use Boost.Asio, define RCF_USE_BOOST_ASIO."
#endif
#endif

namespace RCF {

    class I_ServerTransport;
    class StubEntry;
    class I_Service;
    class I_Session;
    class RcfSession;
    class I_Endpoint;
    class I_FilterFactoryLookupProvider;
    class I_RcfClient;
    class I_IpServerTransport;
    class PingBackService;
    class FileTransferService;
    class ObjectFactoryService;
    class FilterService;
    class SessionTimeoutService;

    typedef boost::shared_ptr<I_ServerTransport>                ServerTransportPtr;
    typedef boost::shared_ptr<I_Proactor>                       ProactorPtr;
    typedef boost::shared_ptr<StubEntry>                        StubEntryPtr;
    typedef boost::shared_ptr<I_Service>                        ServicePtr;
    typedef boost::shared_ptr<RcfSession>                       RcfSessionPtr;
    typedef boost::shared_ptr<I_FilterFactoryLookupProvider>    FilterFactoryLookupProviderPtr;
    typedef boost::shared_ptr<I_RcfClient>                      RcfClientPtr;
    typedef boost::shared_ptr<I_Endpoint>                       EndpointPtr;
    typedef boost::shared_ptr<PingBackService>                  PingBackServicePtr;
    typedef boost::shared_ptr<FileTransferService>              FileTransferServicePtr;
    typedef boost::shared_ptr<ObjectFactoryService>             ObjectFactoryServicePtr;
    typedef boost::shared_ptr<FilterService>                    FilterServicePtr;
    typedef boost::shared_ptr<SessionTimeoutService>            SessionTimeoutServicePtr;
    typedef boost::weak_ptr<RcfSession>                         RcfSessionWeakPtr;

#if defined(_MSC_VER) && _MSC_VER < 1310

    template<typename T>
    struct BindDirect
    {
        typedef char (&yes_type)[1];
        typedef char (&no_type)[2];
        template<typename U> static yes_type dummyfunc(std::auto_ptr<U> *);
        template<typename U> static yes_type dummyfunc(boost::shared_ptr<U> *);
        template<typename U> static yes_type dummyfunc(boost::weak_ptr<U> *);
        static no_type dummyfunc(...);
        typedef boost::mpl::bool_< sizeof(no_type) == sizeof(dummyfunc( (T *) 0)) > type;
    };

#else

    template<typename T> struct BindDirect                          { typedef boost::mpl::true_ type; };
    template<typename T> struct BindDirect< std::auto_ptr<T> >      { typedef boost::mpl::false_ type; };
    template<typename T> struct BindDirect< boost::shared_ptr<T> >  { typedef boost::mpl::false_ type; };
    template<typename T> struct BindDirect< boost::weak_ptr<T> >    { typedef boost::mpl::false_ type; };

#endif


#define RCF_SERVER_BINDING_SECTION_1_1(i1, T, arg)                                          \
    template<typename i1, typename ImplementationT>                                         \
    bool bind_impl_1(i1*, T t, const std::string &name, arg*)

#define RCF_SERVER_BINDING_SECTION_1_2(i1, i2, T, arg)                                      \
    template<typename i1, typename i2, typename ImplementationT>                            \
    bool bind_impl_2(i1*, i2 *, T t, const std::string &name, arg*)

#define RCF_SERVER_BINDING_SECTION_1_3(i1, i2, i3, T, arg)                                  \
    template<typename i1, typename i2, typename i3, typename ImplementationT>               \
    bool bind_impl_3(i1*, i2*, i3*, T t, const std::string &name, arg*)

#define RCF_SERVER_BINDING_SECTION_1_4(i1, i2, i3, i4, T, arg)                              \
    template<typename i1, typename i2, typename i3, typename i4, typename ImplementationT>  \
    bool bind_impl_4(i1*, i2*, i3*, i4*, T t, const std::string &name, arg*)

#define RCF_SERVER_BINDING_SECTION_2(DerefT)                                                \
    {                                                                                       \
        boost::shared_ptr< I_Deref<ImplementationT> > derefPtr(                             \
            new DerefT(t) );                                                                \
                                                                                            \
        RcfClientPtr rcfClientPtr = createServerStub(                                       \
            (I1 *) 0,                                                                       \
            (ImplementationT *) 0,                                                          \
            derefPtr);                                                                      \

#define RCF_SERVER_BINDING_SECTION_3(I_n)                                                   \
        {                                                                                   \
            RcfClientPtr mergeePtr = createServerStub(                                      \
                (I_n *) 0,                                                                  \
                (ImplementationT *) 0,                                                      \
                derefPtr);                                                                  \
                                                                                            \
            rcfClientPtr->getServerStub().merge(mergeePtr);                                 \
    }

#define RCF_SERVER_BINDING_SECTION_4                                                        \
        return bindShared(                                                                  \
            name.empty() ? I1::getInterfaceName() : name ,                                  \
            rcfClientPtr);                                                                  \
    }                                                                                       \

    class RCF_EXPORT RcfServer :
        public virtual I_SessionManager,
        boost::noncopyable
    {
    public:

        RcfServer();
        RcfServer(const I_Endpoint &endpoint);
        RcfServer(const ServicePtr &servicePtr);
        RcfServer(const ServerTransportPtr &serverTransportPtr);
        RcfServer(std::vector<ServicePtr> services);
        RcfServer(std::vector<ServerTransportPtr> serverTransports);
        RcfServer(std::vector<EndpointPtr> endpoints);

        ~RcfServer();

        typedef boost::function0<void> JoinFunctor;
        typedef boost::function1<void, RcfServer&> StartCallback;

#ifdef RCF_MULTI_THREADED

        void start();

#endif

        void    startSt();
        void    addJoinFunctor(const JoinFunctor &joinFunctor);
        void    startInThisThread();
        void    startInThisThread(const JoinFunctor &joinFunctor);
        void    stop();
        bool    cycle(int timeoutMs = 0);
        void    cycleSessions(int timeoutMs, const volatile bool &stopFlag);
        void    open();
        void    close();

        I_ServerTransport &     
                getServerTransport();

        I_Service &             
                getServerTransportService();

        ServerTransportPtr      
                getServerTransportPtr();

        I_IpServerTransport &   
                getIpServerTransport();

        bool    addService(
                    const ServicePtr &servicePtr);

        bool    removeService(
                    const ServicePtr &servicePtr);

        PingBackServicePtr 
                getPingBackServicePtr();

        FileTransferServicePtr
                getFileTransferServicePtr();

        ObjectFactoryServicePtr
                getObjectFactoryServicePtr();

        SessionTimeoutServicePtr 
                getSessionTimeoutServicePtr();

        bool    addServerTransport(
                    const ServerTransportPtr &serverTransportPtr);

        bool    removeServerTransport(
                    const ServerTransportPtr &serverTransportPtr);        
        
        void    setPrimaryThreadCount(
                    std::size_t threadCount);

        void    setPrimaryThreadManager(
                    ThreadManagerPtr threadManagerPtr);

        void    setPrimaryThreadCount(
                    std::size_t threadCount, 
                    I_ThreadManager::ThreadInitFunctor onThreadInit, 
                    I_ThreadManager::ThreadInitFunctor onThreadDeinit);

        void    setStartCallback(const StartCallback &startCallback);
        
    private:
        void    invokeStartCallback();

    private:
        bool    bindShared(
                    const std::string &name, 
                    const RcfClientPtr &rcfClientPtr);

        //*************************************
        // async io transport interface

    private:
        SessionPtr  createSession();
        void        onReadCompleted(const SessionPtr &sessionPtr);
        void        onWriteCompleted(const SessionPtr &sessionPtr);


        //*************************************
        // transports, queues and threads

    private:
        //SessionQueue &getSessionQueue(const RcfSession &session);
        //typedef ThreadSpecificPtr<SessionQueue>::Val ThreadSpecificSessionQueuePtr;
        //ThreadSpecificSessionQueuePtr mThreadSpecificSessionQueuePtr;
        // eventually other specialized session queues...

        volatile bool                                   mServerThreadsStopFlag;
        Mutex                                           mOpenedMutex;
        bool                                            mOpened;

        Mutex                                           mStartedMutex;
        bool                                            mStarted;

        void startImpl(bool spawnThreads);

    public:

        bool getStopFlag() const;

        template<typename Iter>
        void enumerateSessions(const Iter & iter)
        {
            Lock lock(mSessionsMutex);
            std::copy(mSessions.begin(), mSessions.end(), iter);
        }

        //*************************************
        // stub management

    private:
        ReadWriteMutex                                  mStubMapMutex;
        typedef std::map<std::string, StubEntryPtr>     StubMap;
        StubMap                                         mStubMap;

        Mutex                                           mSessionsMutex;
        std::set<RcfSessionWeakPtr>                     mSessions;

        friend class RcfSession;

        void registerSession(
            RcfSessionPtr rcfSessionPtr);

        void unregisterSession(
            RcfSessionWeakPtr rcfSessionWeakPtr);


        //*************************************
        // service management

    private:
        ReadWriteMutex                                  mServicesMutex;
        std::vector<ServicePtr>                         mServices;
        ObjectFactoryServicePtr                         mObjectFactoryServicePtr;
        FilterServicePtr                                mFilterServicePtr;
        std::vector<ServerTransportPtr>                 mServerTransports;

        std::vector<JoinFunctor>                        mJoinFunctors;

        PingBackServicePtr                              mPingBackServicePtr;
        FileTransferServicePtr                          mFileTransferServicePtr;
        SessionTimeoutServicePtr                        mSessionTimeoutServicePtr;

        void startService(const ServicePtr &servicePtr) const;
        void stopService(const ServicePtr &servicePtr, bool wait = true) const;

        FilterPtr createFilter(int filterId);

    private:
        // start/stop functionality
        StartCallback                                   mStartCallback;
        Platform::Threads::condition                    mStartEvent;
        Platform::Threads::condition                    mStopEvent;

    public:

#ifdef RCF_MULTI_THREADED

        /// Waits for the server to be stopped.
        void waitForStopEvent();

        /// Waits for the server to be started.
        void waitForStartEvent();

#endif

        // TODO: get rid of this hack
    private:
        friend class MethodInvocationRequest;

    public:
        int getRcfRuntimeVersion();
        void setRcfRuntimeVersion(int version);

    private:
        int                                             mRcfRuntimeVersion;


    public:

        // binding infrastructure

        // following functions are defined inline for reasons of portability (vc6 in particular)

        // direct
        RCF_SERVER_BINDING_SECTION_1_1(I1, ImplementationT &, boost::mpl::true_)
        RCF_SERVER_BINDING_SECTION_2(DerefObj<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_4

        RCF_SERVER_BINDING_SECTION_1_2(I1, I2, ImplementationT &, boost::mpl::true_)
        RCF_SERVER_BINDING_SECTION_2(DerefObj<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_3(I2)
        RCF_SERVER_BINDING_SECTION_4

        RCF_SERVER_BINDING_SECTION_1_3(I1, I2, I3, ImplementationT &, boost::mpl::true_)
        RCF_SERVER_BINDING_SECTION_2(DerefObj<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_3(I2)
        RCF_SERVER_BINDING_SECTION_3(I3)
        RCF_SERVER_BINDING_SECTION_4

        RCF_SERVER_BINDING_SECTION_1_4(I1, I2, I3, I4, ImplementationT &, boost::mpl::true_)
        RCF_SERVER_BINDING_SECTION_2(DerefObj<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_3(I2)
        RCF_SERVER_BINDING_SECTION_3(I3)
        RCF_SERVER_BINDING_SECTION_3(I4)
        RCF_SERVER_BINDING_SECTION_4

        // indirect - shared_ptr
        RCF_SERVER_BINDING_SECTION_1_1(I1, boost::shared_ptr<ImplementationT>, boost::mpl::false_)
        RCF_SERVER_BINDING_SECTION_2(DerefSharedPtr<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_4

        RCF_SERVER_BINDING_SECTION_1_2(I1, I2, boost::shared_ptr<ImplementationT>, boost::mpl::false_)
        RCF_SERVER_BINDING_SECTION_2(DerefSharedPtr<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_3(I2)
        RCF_SERVER_BINDING_SECTION_4

        RCF_SERVER_BINDING_SECTION_1_3(I1, I2, I3, boost::shared_ptr<ImplementationT>, boost::mpl::false_)
        RCF_SERVER_BINDING_SECTION_2(DerefSharedPtr<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_3(I2)
        RCF_SERVER_BINDING_SECTION_3(I3)
        RCF_SERVER_BINDING_SECTION_4

        RCF_SERVER_BINDING_SECTION_1_4(I1, I2, I3, I4, boost::shared_ptr<ImplementationT>, boost::mpl::false_)
        RCF_SERVER_BINDING_SECTION_2(DerefSharedPtr<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_3(I2)
        RCF_SERVER_BINDING_SECTION_3(I3)
        RCF_SERVER_BINDING_SECTION_3(I4)
        RCF_SERVER_BINDING_SECTION_4

        // indirect - weak_ptr
        RCF_SERVER_BINDING_SECTION_1_1(I1, boost::weak_ptr<ImplementationT>, boost::mpl::false_)
        RCF_SERVER_BINDING_SECTION_2(DerefWeakPtr<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_4

        RCF_SERVER_BINDING_SECTION_1_2(I1, I2, boost::weak_ptr<ImplementationT>, boost::mpl::false_)
        RCF_SERVER_BINDING_SECTION_2(DerefWeakPtr<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_3(I2)
        RCF_SERVER_BINDING_SECTION_4

        RCF_SERVER_BINDING_SECTION_1_3(I1, I2, I3, boost::weak_ptr<ImplementationT>, boost::mpl::false_)
        RCF_SERVER_BINDING_SECTION_2(DerefWeakPtr<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_3(I2)
        RCF_SERVER_BINDING_SECTION_3(I3)
        RCF_SERVER_BINDING_SECTION_4

        RCF_SERVER_BINDING_SECTION_1_4(I1, I2, I3, I4, boost::weak_ptr<ImplementationT>, boost::mpl::false_)
        RCF_SERVER_BINDING_SECTION_2(DerefWeakPtr<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_3(I2)
        RCF_SERVER_BINDING_SECTION_3(I3)
        RCF_SERVER_BINDING_SECTION_3(I4)
        RCF_SERVER_BINDING_SECTION_4

        // indirect - auto_ptr
        RCF_SERVER_BINDING_SECTION_1_1(I1, const std::auto_ptr<ImplementationT> &, boost::mpl::false_)
        RCF_SERVER_BINDING_SECTION_2(DerefAutoPtr<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_4

        RCF_SERVER_BINDING_SECTION_1_2(I1, I2, const std::auto_ptr<ImplementationT> &, boost::mpl::false_)
        RCF_SERVER_BINDING_SECTION_2(DerefAutoPtr<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_3(I2)
        RCF_SERVER_BINDING_SECTION_4

        RCF_SERVER_BINDING_SECTION_1_3(I1, I2, I3, const std::auto_ptr<ImplementationT> &, boost::mpl::false_)
        RCF_SERVER_BINDING_SECTION_2(DerefAutoPtr<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_3(I2)
        RCF_SERVER_BINDING_SECTION_3(I3)
        RCF_SERVER_BINDING_SECTION_4

        RCF_SERVER_BINDING_SECTION_1_4(I1, I2, I3, I4, const std::auto_ptr<ImplementationT> &, boost::mpl::false_)
        RCF_SERVER_BINDING_SECTION_2(DerefAutoPtr<ImplementationT>)
        RCF_SERVER_BINDING_SECTION_3(I2)
        RCF_SERVER_BINDING_SECTION_3(I3)
        RCF_SERVER_BINDING_SECTION_3(I4)
        RCF_SERVER_BINDING_SECTION_4

#if !defined(_MSC_VER) || _MSC_VER > 1200

        template<typename I1, typename ImplementationT>
        bool bind(ImplementationT &t, const std::string &name = "")
        {
            typedef typename BindDirect<ImplementationT>::type type;
            return bind_impl_1((I1 *) NULL, t, name, (type *) NULL);
        }

        template<typename I1, typename I2, typename ImplementationT>
        bool bind(ImplementationT &t, const std::string &name = "")
        {
            typedef typename BindDirect<ImplementationT>::type type;
            return bind_impl_2((I1 *) NULL, (I2 *) NULL, t, name, (type *) NULL);
        }

        template<typename I1, typename I2, typename I3, typename ImplementationT>
        bool bind(ImplementationT &t, const std::string &name = "")
        {
            typedef typename BindDirect<ImplementationT>::type type;
            return bind_impl_3((I1 *) NULL, (I2 *) NULL, (I3 *) NULL, t, name, (type *) NULL);
        }

        template<typename I1, typename I2, typename I3, typename I4, typename ImplementationT>
        bool bind(ImplementationT &t, const std::string &name = "")
        {
            typedef typename BindDirect<ImplementationT>::type type;
            return bind_impl_4((I1 *) NULL, (I2 *) NULL, (I3 *) NULL, (I4 *) NULL, t, name, (type *) NULL);
        }


        template<typename I1, typename ImplementationT>
        bool bind(const ImplementationT &t, const std::string &name = "")
        {
            typedef typename BindDirect<ImplementationT>::type type;
            return bind_impl_1((I1 *) NULL, t, name, (type *) NULL);
        }

        template<typename I1, typename I2, typename ImplementationT>
        bool bind(const ImplementationT &t, const std::string &name = "")
        {
            typedef typename BindDirect<ImplementationT>::type type;
            return bind_impl_2((I1 *) NULL, (I2 *) NULL, t, name, (type *) NULL);
        }

        template<typename I1, typename I2, typename I3, typename ImplementationT>
        bool bind(const ImplementationT &t, const std::string &name = "")
        {
            typedef typename BindDirect<ImplementationT>::type type;
            return bind_impl_3((I1 *) NULL, (I2 *) NULL, (I3 *) NULL, t, name, (type *) NULL);
        }

        template<typename I1, typename I2, typename I3, typename I4, typename ImplementationT>
        bool bind(const ImplementationT &t, const std::string &name = "")
        {
            typedef typename BindDirect<ImplementationT>::type type;
            return bind_impl_4((I1 *) NULL, (I2 *) NULL, (I3 *) NULL, (I4 *) NULL, t, name, (type *) NULL);
        }


        template<typename InterfaceT>
        bool unbind(const std::string &name = "")
        {
            return unbind((InterfaceT *) NULL, name);
        }

#endif

        template<typename I1, typename ImplementationT>
        bool bind(I1 *i1, ImplementationT &t, const std::string &name = "")
        {
            typedef typename BindDirect<ImplementationT>::type type;
            return bind_impl_1(i1, t, name, (type *) NULL);
        }

        template<typename I1, typename I2, typename ImplementationT>
        bool bind(I1 *i1, I2 *i2, ImplementationT &t, const std::string &name = "")
        {
            typedef typename BindDirect<ImplementationT>::type type;
            return bind_impl_2(i1, i2, t, name, (type *) NULL);
        }

        template<typename I1, typename I2, typename I3, typename ImplementationT>
        bool bind(I1 *i1, I2 *i2, I3 *i3, ImplementationT &t, const std::string &name = "")
        {
            typedef typename BindDirect<ImplementationT>::type type;
            return bind_impl_3(i1, i2, i3, t, name, (type *) NULL);
        }

        template<typename I1, typename I2, typename I3, typename I4, typename ImplementationT>
        bool bind(I1 *i1, I2 *i2, I3 *i3, I4 *i4, ImplementationT &t, const std::string &name = "")
        {
            typedef typename BindDirect<ImplementationT>::type type;
            return bind_impl_4(i1, i2, i3, i4, t, name, (type *) NULL);
        }

        template<typename InterfaceT>
        bool unbind(InterfaceT * , const std::string &name_ = "")
        {
            const std::string &name = (name_ == "") ?
                getInterfaceName((InterfaceT *) NULL) :
                name_;

            WriteLock writeLock(mStubMapMutex);
            mStubMap.erase(name);
            return true;
        }

    };

} // namespace RCF

#endif // ! INCLUDE_RCF_RCFSERVER_HPP

