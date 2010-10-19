
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/RcfServer.hpp>

#include <algorithm>

#include <boost/bind.hpp>

#include <RCF/AsyncFilter.hpp>
#include <RCF/CurrentSession.hpp>
#include <RCF/Endpoint.hpp>
#include <RCF/FilterService.hpp>
#include <RCF/IpServerTransport.hpp>
#include <RCF/Marshal.hpp>
#include <RCF/MethodInvocation.hpp>
#include <RCF/ObjectFactoryService.hpp>
#include <RCF/PingBackService.hpp>
#include <RCF/RcfClient.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ServerTask.hpp>
#include <RCF/SessionTimeoutService.hpp>
#include <RCF/Service.hpp>
#include <RCF/StubEntry.hpp>
#include <RCF/ThreadLocalCache.hpp>
#include <RCF/Token.hpp>
#include <RCF/Tools.hpp>
#include <RCF/Version.hpp>

#ifdef RCF_USE_SF_SERIALIZATION
#include <SF/memory.hpp>
#endif

#ifdef RCF_USE_BOOST_SERIALIZATION
#include <RCF/BsAutoPtr.hpp>
#endif

#ifdef RCF_USE_BOOST_FILESYSTEM
#include <RCF/FileTransferService.hpp>
#else
namespace RCF { class FileTransferService {}; }
#endif

namespace RCF {

    void repeatCycleServer(RcfServer &server, int timeoutMs)
    {
        RCF_TRACE("");
        while (!server.cycle(timeoutMs));
        RCF_TRACE("");
    }

    // RcfServer

    RcfServer::RcfServer() :
        mServerThreadsStopFlag(RCF_DEFAULT_INIT),
        mOpened(RCF_DEFAULT_INIT),
        mStarted(RCF_DEFAULT_INIT),
        mStubMapMutex(WriterPriority),
        mServicesMutex(WriterPriority),
        mRcfRuntimeVersion(RCF::getRuntimeVersion())
    {
        RCF_TRACE("");
    }

    RcfServer::RcfServer(const I_Endpoint &endpoint) :
        mServerThreadsStopFlag(RCF_DEFAULT_INIT),
        mOpened(RCF_DEFAULT_INIT),
        mStarted(RCF_DEFAULT_INIT),
        mStubMapMutex(WriterPriority),
        mServicesMutex(WriterPriority),
        mRcfRuntimeVersion(RCF::getRuntimeVersion())
    {
        RCF_TRACE("");

        ServerTransportPtr serverTransportPtr(
            endpoint.createServerTransport().release());

        ServicePtr servicePtr(
            boost::dynamic_pointer_cast<I_Service>(serverTransportPtr) );

        addService(servicePtr);
    }

    RcfServer::RcfServer(const ServicePtr &servicePtr) :
        mServerThreadsStopFlag(RCF_DEFAULT_INIT),
        mOpened(RCF_DEFAULT_INIT),
        mStarted(RCF_DEFAULT_INIT),
        mStubMapMutex(WriterPriority),
        mServicesMutex(WriterPriority),
        mRcfRuntimeVersion(RCF::getRuntimeVersion())
    {
        RCF_TRACE("");

        addService(servicePtr);
    }

    RcfServer::RcfServer(const ServerTransportPtr &serverTransportPtr) :
        mServerThreadsStopFlag(RCF_DEFAULT_INIT),
        mOpened(RCF_DEFAULT_INIT),
        mStarted(RCF_DEFAULT_INIT),
        mStubMapMutex(WriterPriority),
        mServicesMutex(WriterPriority),
        mRcfRuntimeVersion(RCF::getRuntimeVersion())
    {
        RCF_TRACE("");

        addService( boost::dynamic_pointer_cast<I_Service>(serverTransportPtr) );
    }

    RcfServer::RcfServer(std::vector<ServerTransportPtr> serverTransports) :
        mServerThreadsStopFlag(RCF_DEFAULT_INIT),
        mOpened(RCF_DEFAULT_INIT),
        mStarted(RCF_DEFAULT_INIT),
        mStubMapMutex(WriterPriority),
        mServicesMutex(WriterPriority),
        mRcfRuntimeVersion(RCF::getRuntimeVersion())
    {
        RCF_TRACE("");

        std::for_each(serverTransports.begin(), serverTransports.end(),
            boost::bind(&RcfServer::addServerTransport, this, _1));
    }

    RcfServer::RcfServer(std::vector<ServicePtr> services) :
        mServerThreadsStopFlag(RCF_DEFAULT_INIT),
        mOpened(RCF_DEFAULT_INIT),
        mStarted(RCF_DEFAULT_INIT),
        mStubMapMutex(WriterPriority),
        mServicesMutex(WriterPriority),
        mRcfRuntimeVersion(RCF::getRuntimeVersion())
    {
        RCF_TRACE("");

        std::for_each(services.begin(), services.end(),
            boost::bind(&RcfServer::addService, this, _1));
    }

    RcfServer::RcfServer(std::vector<EndpointPtr> endpoints) :
        mServerThreadsStopFlag(RCF_DEFAULT_INIT),
        mOpened(RCF_DEFAULT_INIT),
        mStarted(RCF_DEFAULT_INIT),
        mStubMapMutex(WriterPriority),
        mServicesMutex(WriterPriority),
        mRcfRuntimeVersion(RCF::getRuntimeVersion())
    {
        RCF_TRACE("");

        std::vector<ServicePtr> services;

        for (std::size_t i=0; i<endpoints.size(); ++i)
        {
            ServerTransportPtr serverTransportPtr(
                endpoints[i]->createServerTransport().release());

            ServicePtr servicePtr(
                boost::dynamic_pointer_cast<I_Service>(serverTransportPtr) );

            services.push_back(servicePtr);
        }

        std::for_each(services.begin(), services.end(),
            boost::bind(&RcfServer::addService, this, _1));
    }

    RcfServer::~RcfServer()
    {
        RCF_DTOR_BEGIN
            RCF_TRACE("");
            close();
        RCF_DTOR_END
    }

    bool RcfServer::addService(const ServicePtr &servicePtr)
    {
        RCF_TRACE("")(typeid(*servicePtr).name());

        bool ret = false;
        {
            WriteLock writeLock(mServicesMutex);
            if (
                std::find(
                    mServices.begin(),
                    mServices.end(),
                    servicePtr) == mServices.end())
            {
                mServices.push_back(servicePtr);
                ret = true;

                ObjectFactoryServicePtr objectFactoryServicePtr =
                    boost::dynamic_pointer_cast<ObjectFactoryService>(servicePtr);

                if (objectFactoryServicePtr)
                {
                    mObjectFactoryServicePtr = objectFactoryServicePtr;
                }

                FilterServicePtr filterServicePtr =
                    boost::dynamic_pointer_cast<FilterService>(servicePtr);

                if (filterServicePtr)
                {
                    mFilterServicePtr = filterServicePtr;
                }

                ServerTransportPtr serverTransportPtr =
                    boost::dynamic_pointer_cast<I_ServerTransport>(servicePtr);

                if (serverTransportPtr)
                {
                    mServerTransports.push_back(serverTransportPtr);
                }

                PingBackServicePtr pingBackServicePtr =
                    boost::dynamic_pointer_cast<PingBackService>(servicePtr);

                if (pingBackServicePtr)
                {
                    mPingBackServicePtr = pingBackServicePtr;
                }

                FileTransferServicePtr fileTransferServicePtr =
                    boost::dynamic_pointer_cast<FileTransferService>(servicePtr);

                if (fileTransferServicePtr)
                {
                    mFileTransferServicePtr = fileTransferServicePtr;
                }

                SessionTimeoutServicePtr sessionTimeoutServicePtr =
                    boost::dynamic_pointer_cast<SessionTimeoutService>(servicePtr);

                if (sessionTimeoutServicePtr)
                {
                    mSessionTimeoutServicePtr = sessionTimeoutServicePtr;
                }
            }
        }
        if (ret)
        {
            servicePtr->onServiceAdded(*this);
        }
        {
            Lock lock(mStartedMutex);
            if (mStarted)
            {
                startService(servicePtr);
            }
        }
        return ret;
    }

    bool RcfServer::removeService(const ServicePtr &servicePtr)
    {
        RCF_TRACE("")(typeid(*servicePtr).name());

        bool found = false;
        {
            WriteLock writeLock(mServicesMutex);
            std::vector<ServicePtr>::iterator iter =
                std::find(mServices.begin(), mServices.end(), servicePtr);

            if (iter != mServices.end())
            {
                stopService(*iter);
                mServices.erase(iter);
                found = true;
            }
        }

        if (found)
        {
            ObjectFactoryServicePtr objectFactoryServicePtr =
                boost::dynamic_pointer_cast<ObjectFactoryService>(servicePtr);

            if (objectFactoryServicePtr)
            {
                mObjectFactoryServicePtr.reset();
            }

            FilterServicePtr filterServicePtr =
                boost::dynamic_pointer_cast<FilterService>(servicePtr);

            if (filterServicePtr)
            {
                mFilterServicePtr.reset();
            }

            ServerTransportPtr serverTransportPtr =
                boost::dynamic_pointer_cast<I_ServerTransport>(servicePtr);

            if (serverTransportPtr)
            {
                eraseRemove(mServerTransports, serverTransportPtr);
            }

            PingBackServicePtr pingBackServicePtr =
                boost::dynamic_pointer_cast<PingBackService>(servicePtr);

            if (pingBackServicePtr)
            {
                mPingBackServicePtr.reset();
            }

            FileTransferServicePtr fileTransferServicePtr =
                boost::dynamic_pointer_cast<FileTransferService>(servicePtr);

            if (fileTransferServicePtr)
            {
                mFileTransferServicePtr.reset();
            }

            SessionTimeoutServicePtr sessionTimeoutServicePtr =
                boost::dynamic_pointer_cast<SessionTimeoutService>(servicePtr);

            if (sessionTimeoutServicePtr)
            {
                mSessionTimeoutServicePtr.reset();
            }

            servicePtr->onServiceRemoved(*this);
        }
        return found;
    }

    bool RcfServer::addServerTransport(const ServerTransportPtr &serverTransportPtr)
    {
        return addService(
            boost::dynamic_pointer_cast<I_Service>(serverTransportPtr));
    }

    bool RcfServer::removeServerTransport(const ServerTransportPtr &serverTransportPtr)
    {
        return removeService(
            boost::dynamic_pointer_cast<I_Service>(serverTransportPtr));
    }

    void RcfServer::open()
    {
        RCF_TRACE("");

        Lock lock(mOpenedMutex);
        if (!mOpened)
        {
            std::vector<ServicePtr> services;
            {
                ReadLock readLock(mServicesMutex);
                std::copy(
                    mServices.begin(),
                    mServices.end(),
                    std::back_inserter(services));
            }

            std::for_each(
                services.begin(),
                services.end(),
                boost::bind(&I_Service::onServerOpen, _1, boost::ref(*this)));

            mOpened = true;
        }
    }

#ifdef RCF_MULTI_THREADED

    void RcfServer::start()
    {
        startImpl(true);
    }

#endif

    void RcfServer::startSt()
    {
        startImpl(false);
    }

    void RcfServer::startImpl(bool spawnThreads)
    {
        RCF_TRACE("");

        Lock lock(mStartedMutex);
        if (!mStarted)
        {
            mServerThreadsStopFlag = false;

            // open the server
            open();

            // make a local copy of the service table
            std::vector<ServicePtr> services;
            {
                ReadLock readLock(mServicesMutex);
                std::copy(
                    mServices.begin(),
                    mServices.end(),
                    std::back_inserter(services));
            }

            // notify all services
            std::for_each(
                services.begin(),
                services.end(),
                boost::bind(&I_Service::onServerStart, _1, boost::ref(*this)));

            // spawn internal worker threads
            if (spawnThreads)
            {
                std::for_each(
                    services.begin(),
                    services.end(),
                    boost::bind(&RcfServer::startService, boost::cref(*this), _1));
            }

            mStarted = true;

            // call the start notification callback, if there is one
            invokeStartCallback();

            // notify anyone who was waiting on the stop event
            mStartEvent.notify_all();
        }
    }

    void RcfServer::addJoinFunctor(const JoinFunctor &joinFunctor)
    {
        if (joinFunctor)
        {
            mJoinFunctors.push_back(joinFunctor);
        }
    }

    void RcfServer::startInThisThread()
    {
        startInThisThread(JoinFunctor());
    }

    void RcfServer::startInThisThread(const JoinFunctor &joinFunctor)
    {
        RCF_TRACE("");

        startSt();

        // register the join functor
        mJoinFunctors.push_back(joinFunctor);

        // run all tasks sequentially in this thread
        repeatCycleServer(*this, 500);

    }

    bool RcfServer::cycle(int timeoutMs)
    {
        RCF_TRACE("")(timeoutMs);

        // sequentially run each task
        // only first task gets to use the timeout
        // if tasks are being dynamically added or removed, a given task might be cycled twice or not at all

        unsigned int i=0;
        while (true)
        {
            ServicePtr servicePtr;
            {
                ReadLock readLock(mServicesMutex);
                if (i < mServices.size())
                {
                    servicePtr = mServices[i];
                }
            }
            if (servicePtr)
            {
                unsigned int j=0;
                while (true)
                {
                    Task task;
                    bool ok = false;
                    {
                        ReadLock readLock(servicePtr->getTaskEntriesMutex());
                        TaskEntries &taskEntries = servicePtr->getTaskEntries();
                        if (j < taskEntries.size())
                        {
                            task = taskEntries[j].getTask();
                            ok = true;
                        }
                    }
                    if (ok)
                    {
                        task(
                            i == 0  && j == 0 ? timeoutMs : 0,
                            mServerThreadsStopFlag,
                            true); //JL
                        ++j;
                    }
                    else
                    {
                        break;
                    }
                }
                ++i;
            }
            else
            {
                break;
            }
        }

        return mServerThreadsStopFlag;
    }

    void RcfServer::startService(const ServicePtr &servicePtr) const
    {
        RCF_TRACE("")(typeid(*servicePtr));

        WriteLock writeLock(servicePtr->getTaskEntriesMutex());
        TaskEntries &taskEntries = servicePtr->getTaskEntries();
        std::for_each(
            taskEntries.begin(),
            taskEntries.end(),
            boost::bind(
                &TaskEntry::start,
                _1,
                boost::ref(mServerThreadsStopFlag)));
    }

    void RcfServer::stopService(const ServicePtr &servicePtr, bool wait) const
    {
        RCF_TRACE("")(typeid(*servicePtr))(wait);

        typedef void (TaskEntry::*Pfn)(bool);

        WriteLock writeLock(servicePtr->getTaskEntriesMutex());
        TaskEntries &taskEntries = servicePtr->getTaskEntries();

#if defined(__SGI_STL_PORT) || defined(_STLPORT_VERSION)

        TaskEntries::reverse_iterator iter;
        for (iter=taskEntries.rbegin(); iter != taskEntries.rend(); ++iter)
        {
            (*iter).stop(wait);
        }

#else

        std::for_each(
            taskEntries.rbegin(),
            taskEntries.rend(),
            boost::bind( (Pfn) &TaskEntry::stop, _1, wait));

#endif

    }

    void RcfServer::stop()
    {
        RCF_TRACE("");

        bool wait = true;

        Lock lock(mStartedMutex);
        if (mStarted)
        {
            // set stop flag
            mServerThreadsStopFlag = true;

            // make a local copy of the service table
            std::vector<ServicePtr> services;
            {
                ReadLock readLock(mServicesMutex);
                std::copy(
                    mServices.begin(),
                    mServices.end(),
                    std::back_inserter(services));
            }

            // notify and optionally join all internal worker threads
            typedef void (RcfServer::*Pfn)(const ServicePtr &, bool) const;
            std::for_each(
                services.rbegin(),
                services.rend(),
                boost::bind( (Pfn) &RcfServer::stopService, boost::cref(*this), _1, wait));

            if (wait)
            {
                // join all external worker threads
                std::for_each(
                    mJoinFunctors.rbegin(),
                    mJoinFunctors.rend(),
                    boost::bind(&JoinFunctor::operator(), _1));

                mJoinFunctors.clear();

                // notify all services
                std::for_each(
                    services.rbegin(),
                    services.rend(),
                    boost::bind(&I_Service::onServerStop, _1, boost::ref(*this)));

                // clear stop flag, since all the threads have been joined
                mServerThreadsStopFlag = false;

                mStarted = false;

                // notify anyone who was waiting on the stop event
                mStopEvent.notify_all();
            }
        }
    }

    void RcfServer::close()
    {
        RCF_TRACE("");

        Lock lock(mOpenedMutex);
        if (mOpened)
        {
            // stop the server
            stop();

            std::vector<ServicePtr> services;
            {
                ReadLock readLock(mServicesMutex);
                std::copy(
                    mServices.begin(),
                    mServices.end(),
                    std::back_inserter(services));
            }
            std::for_each(
                services.rbegin(),
                services.rend(),
                boost::bind(&I_Service::onServerClose, _1, boost::ref(*this)));

            // set status
            mOpened = false;
        }
    }

    void RcfServer::setPrimaryThreadManager(ThreadManagerPtr threadManagerPtr)
    {
        getServerTransportService().getTaskEntries().front()
            .setThreadManagerPtr(threadManagerPtr);
    }

    void RcfServer::setPrimaryThreadCount(std::size_t threadCount)
    {
        ThreadManagerPtr threadManagerPtr;
        if (threadCount == 1)
        {
            threadManagerPtr.reset(
                new FixedThreadPool(threadCount));
        }
        else
        {
            threadManagerPtr.reset(
                new DynamicThreadPool(threadCount, threadCount));
        }

        setPrimaryThreadManager(threadManagerPtr);
    }

    void RcfServer::setPrimaryThreadCount(
        std::size_t threadCount,
        I_ThreadManager::ThreadInitFunctor onThreadInit, 
        I_ThreadManager::ThreadInitFunctor onThreadDeinit)
    {
        ThreadManagerPtr threadManagerPtr;
        if (threadCount == 1)
        {
            threadManagerPtr.reset(
                new FixedThreadPool(threadCount));
        }
        else
        {
            threadManagerPtr.reset(
                new DynamicThreadPool(threadCount, threadCount));
        }

        threadManagerPtr->addThreadInitFunctor(onThreadInit);
        threadManagerPtr->addThreadDeinitFunctor(onThreadDeinit);

        setPrimaryThreadManager(threadManagerPtr);
    }

#ifdef RCF_MULTI_THREADED

    void RcfServer::waitForStopEvent()
    {
        RCF_TRACE("");

        Lock lock(mStartedMutex);
        if (mStarted)
        {
            mStopEvent.wait(lock);
        }
    }

    void RcfServer::waitForStartEvent()
    {
        RCF_TRACE("");

        Lock lock(mStartedMutex);
        if (!mStarted)
        {
            mStartEvent.wait(lock);
        }
    }

#endif

    SessionPtr RcfServer::createSession()
    {
        RCF_TRACE("");

        RcfSessionPtr rcfSessionPtr(new RcfSession(*this));

        rcfSessionPtr->setWeakThisPtr();

        {
            Lock lock(mSessionsMutex);
            mSessions.insert(rcfSessionPtr);
        }

        return rcfSessionPtr;
    }

    void RcfServer::registerSession(
        RcfSessionPtr rcfSessionPtr)
    {
        Lock lock(mSessionsMutex);
        mSessions.insert( RcfSessionWeakPtr(rcfSessionPtr));
    }

    void RcfServer::unregisterSession(
        RcfSessionWeakPtr rcfSessionWeakPtr)
    {
        Lock lock(mSessionsMutex);

        std::set<RcfSessionWeakPtr>::iterator iter =
            mSessions.find(rcfSessionWeakPtr);

        RCF_ASSERT(iter != mSessions.end());

        mSessions.erase(iter);
    }

    void RcfServer::onReadCompleted(const SessionPtr &sessionPtr)
    {
        RcfSessionPtr rcfSessionPtr =
            boost::static_pointer_cast<RcfSession>(sessionPtr);

        // Need a recursion limiter here. When processing many sequential oneway
        // calls, over a caching transport filter such as the zlib filter, we 
        // would otherwise be at risk of encountering unlimited recursion and 
        // eventual stack overflow.

        RecursionState<int, int> & recursionState = 
            getCurrentRcfSessionRecursionState();

        applyRecursionLimiter(
            recursionState, 
            &RcfSession::onReadCompleted, 
            *rcfSessionPtr);

        //rcfSessionPtr->onReadCompleted();
    }

    void RcfSession::onReadCompleted()
    {
        // 1. Deserialize request data
        // 2. Store request data in session
        // 3. Move session to corresponding queue

        Lock lock(mStopCallInProgressMutex);
        if (!mStopCallInProgress)
        {

            ByteBuffer readByteBuffer = getProactor().getReadByteBuffer();

            RCF_TRACE("")(this)(readByteBuffer.getLength());

            ByteBuffer messageBody;

            bool ok = mRequest.decodeRequest(
                readByteBuffer,
                messageBody,
                shared_from_this(),
                mRcfServer);

            // Setup the in stream for this remote call.
            mIn.reset(
                messageBody, 
                mRequest.mSerializationProtocol, 
                mRcfRuntimeVersion, 
                mArchiveVersion);

            messageBody.clear();
            
            readByteBuffer.clear();

            if (!ok)
            {
                // version mismatch (client is newer than we are)
                // send a control message back to client, with our runtime version

                std::vector<ByteBuffer> byteBuffers(1);

                encodeServerError(
                    byteBuffers.front(),
                    RcfError_VersionMismatch, 
                    mRcfServer.getRcfRuntimeVersion());

                getProactor().postWrite(byteBuffers);
            }
            else
            {
                if (mRequest.getClose()) 
                {
                    getProactor().postClose();
                }
                else
                {
                    // TODO: downside of calling processRequest() now is that
                    // the stack might already be quite deep. Might be better
                    // to unwind the stack first and then call handleSession().
                    processRequest();
                }
            }
        }
    }

    void RcfServer::onWriteCompleted(const SessionPtr &sessionPtr)
    {
        RcfSessionPtr rcfSessionPtr = 
            boost::static_pointer_cast<RcfSession>(sessionPtr);

        rcfSessionPtr->onWriteCompleted();
    }

    void RcfSession::onWriteCompleted()
    {
        RCF_TRACE("")(this);

        {
            Lock lock(mIoStateMutex);

            if (mWritingPingBack)
            {
                mWritingPingBack = false;

                typedef std::vector<ByteBuffer> ByteBuffers;
                ThreadLocalCached< ByteBuffers > tlcQueuedBuffers;
                ByteBuffers & queuedBuffers = tlcQueuedBuffers.get();

                queuedBuffers = mQueuedSendBuffers;
                mQueuedSendBuffers.clear();
                if (!queuedBuffers.empty())
                {
                    lock.unlock();
                    getProactor().postWrite(queuedBuffers);
                    RCF_ASSERT(queuedBuffers.empty());
                }

                return;
            }
        }

        typedef std::vector<RcfSession::OnWriteCompletedCallback> OnWriteCompletedCallbacks;
        ThreadLocalCached< OnWriteCompletedCallbacks > tlcOwcc;
        OnWriteCompletedCallbacks &onWriteCompletedCallbacks = tlcOwcc.get();
        
        extractOnWriteCompletedCallbacks(onWriteCompletedCallbacks);

        std::for_each(
            onWriteCompletedCallbacks.begin(),
            onWriteCompletedCallbacks.end(),
            boost::bind(
                &RcfSession::OnWriteCompletedCallback::operator(),
                _1,
                boost::ref(*this)));

        onWriteCompletedCallbacks.resize(0);

        mIn.clear();
        mOut.clear();

        if (!mCloseSessionAfterWrite)
        {
            getProactor().postRead();
        }        
    }

    void RcfSession::sendSessionResponse()
    {
        mIn.clearByteBuffer();

        ThreadLocalCached< std::vector<ByteBuffer> > tlcByteBuffers;
        std::vector<ByteBuffer> &byteBuffers = tlcByteBuffers.get();

        mOut.extractByteBuffers(byteBuffers);
        const std::vector<FilterPtr> &filters = mFilters;
        ThreadLocalCached< std::vector<ByteBuffer> > tlcEncodedByteBuffers;
        std::vector<ByteBuffer> &encodedByteBuffers = tlcEncodedByteBuffers.get();

        ThreadLocalCached< std::vector<FilterPtr> > tlcNoFilters;
        std::vector<FilterPtr> &noFilters = tlcNoFilters.get();

        mRequest.encodeToMessage(
            encodedByteBuffers, 
            byteBuffers, 
            mFiltered ? filters : noFilters);

        RCF2_TRACE("twoway call - sending response")
            (this)
            (lengthByteBuffers(byteBuffers))
            (lengthByteBuffers(encodedByteBuffers));

        byteBuffers.resize(0);

        bool okToWrite = false;
        {
            Lock lock(mIoStateMutex);
            unregisterForPingBacks();
            if (mWritingPingBack)
            {
                mQueuedSendBuffers = encodedByteBuffers;
                encodedByteBuffers.resize(0);
                byteBuffers.resize(0);
            }
            else
            {
                okToWrite = true;
            }
        }

        if (okToWrite)
        {
            getProactor().postWrite(encodedByteBuffers);
            RCF_ASSERT(encodedByteBuffers.empty());
            RCF_ASSERT(byteBuffers.empty());
        }

        setCurrentRcfSessionPtr();
    }

    void RcfSession::sendResponseUncaughtException()
    {
        RCF2_TRACE("")(this);
        sendResponseException( RemoteException(_RcfError_NonStdException()));
    }

    void RcfSession::encodeRemoteException(
        SerializationProtocolOut & out, 
        const RemoteException & e)
    {
        ByteBuffer buffer;
        bool shouldSerializeException = mRequest.encodeResponse(&e, buffer);

        mOut.reset(
            mRequest.mSerializationProtocol, 
            32, 
            buffer, 
            mRcfRuntimeVersion, 
            mArchiveVersion);

        if (shouldSerializeException)
        {
            if (
                out.getSerializationProtocol() == Sp_BsBinary 
                || out.getSerializationProtocol() == Sp_BsText 
                || out.getSerializationProtocol() == Sp_BsXml)
            {
                // Boost serialization is very picky about serializing pointers 
                // vs values. Since the client will deserialize an auto_ptr, we 
                // are forced to create an auto_ptr here as well.

                std::auto_ptr<RemoteException> apRe( 
                    static_cast<RemoteException *>(e.clone().release()) );

                serialize(out, apRe);
            }
            else
            {
                // SF is a bit more flexible.
                serialize(out, e);
            }
        }
    }

    void RcfSession::sendResponse()
    {
        bool exceptionalResponse = false;
        try
        {
            ByteBuffer buffer;
            mRequest.encodeResponse(NULL, buffer);

            mOut.reset(
                mRequest.mSerializationProtocol, 
                32, 
                buffer, 
                mRcfRuntimeVersion, 
                mArchiveVersion);

            mpParameters->write(mOut);
            clearParameters();
        }
        catch(const std::exception &e)
        {
            sendResponseException(e);
            exceptionalResponse = true;
        }

        if (!exceptionalResponse)
        {
            sendSessionResponse();
        }
    }

    void RcfSession::sendResponseException(
        const std::exception &e)
    {
        clearParameters();

        const SerializationException *pSE =
            dynamic_cast<const SerializationException *>(&e);

        const RemoteException *pRE =
            dynamic_cast<const RemoteException *>(&e);

        const Exception *pE =
            dynamic_cast<const Exception *>(&e);

        if (pSE)
        {
            RCF_TRACE(": Serialization exception")(typeid(*pSE))(*pSE);
            encodeRemoteException(
                mOut,
                RemoteException(
                    Error( pSE->getError() ),
                    pSE->what(),
                    pSE->getContext(),
                    typeid(*pSE).name()));
        }
        else if (pRE)
        {
            RCF_TRACE(": User exception")(typeid(*pRE))(*pRE);
            try
            {
                encodeRemoteException(mOut, *pRE);
            }
            catch(const RCF::Exception &e)
            {
                encodeRemoteException(
                    mOut,
                    RemoteException(
                        _RcfError_Serialization(typeid(*pRE).name(), typeid(e).name(), e.getError().getErrorString()),
                        e.getWhat(),
                        e.getContext(),
                        typeid(e).name()));
            }
            catch(const std::exception &e)
            {
                encodeRemoteException(
                    mOut,
                    RemoteException(
                        _RcfError_Serialization(typeid(*pRE).name(), typeid(e).name(), e.what()),
                        e.what(),
                        "",
                        typeid(e).name()));
            }
        }
        else if (pE)
        {
            RCF_TRACE(": User exception")(typeid(*pE))(*pE);
            encodeRemoteException(
                mOut,
                RemoteException(
                    Error( pE->getError() ),
                    pE->getSubSystemError(),
                    pE->getSubSystem(),
                    pE->what(),
                    pE->getContext(),
                    typeid(*pE).name()));
        }
        else
        {
            RCF_TRACE(": User exception")(typeid(e))(e);
            encodeRemoteException(
                mOut,
                RemoteException(
                    _RcfError_UserModeException(typeid(e).name(), e.what()),
                    e.what(),
                    "",
                    typeid(e).name()));
        }

        sendSessionResponse();
    }

    class SessionTouch 
    {
    public:
        SessionTouch(RcfSession &rcfSession) : mRcfSession(rcfSession)
        {
            mRcfSession.touch();
        }
        ~SessionTouch()
        {
            mRcfSession.touch();
        }

    private:
        RcfSession & mRcfSession;
    };

    class StubEntryTouch 
    {
    public:
        StubEntryTouch(StubEntry &stubEntry) : mStubEntry(stubEntry)
        {
            mStubEntry.touch();
        }
        ~StubEntryTouch()
        {
            mStubEntry.touch();
        }

    private:
        StubEntry & mStubEntry;
    };

    void RcfSession::processRequest()
    {
        MethodInvocationRequest &request = mRequest;

        RCF2_TRACE("")
            (this)
            (request.getService())
            (request.getToken())
            (request.getSubInterface())
            (request.getFnId());

        setCurrentRcfSessionPtr(shared_from_this());

        StubEntryPtr stubEntryPtr = request.locateStubEntryPtr(mRcfServer);

        // NB: the following scopeguard is apparently not triggered by 
        // Borland C++, when throwing non std::exception derived exceptions.

        using namespace boost::multi_index::detail;

        scope_guard sendResponseUncaughtExceptionGuard =
            make_obj_guard(
                *this,
                &RcfSession::sendResponseUncaughtException);

        try
        {
            mAutoSend = true;

            if (NULL == stubEntryPtr.get())
            {
                RCF_THROW(
                    Exception(_RcfError_NoServerStub(request.getService(), request.getSubInterface())))
                    (request.getService())(request.getSubInterface())
                    (request.getFnId());
            }
            else
            {
                setCachedStubEntryPtr(stubEntryPtr);

                SessionTouch sessionTouch(*this);

                StubEntryTouch stubEntryTouch(*stubEntryPtr);

                if (request.getFnId() == -1)
                {
                    // Function id -1 is a canned ping request. We set a
                    // timestamp on the current session and return immediately.

                    AllocateServerParameters<Void>()(*this);

                    Lock lock(mMutex);
                    mPingTimestamp = Platform::OS::getCurrentTimeMs();
                }
                else
                {
                    registerForPingBacks();

                    ThreadInfoPtr threadInfoPtr = getThreadInfoPtr();
                    if (threadInfoPtr)
                    {
                        threadInfoPtr->mThreadManagerPtr->notifyBusy();
                    }

                    stubEntryPtr->getRcfClientPtr()->getServerStub().invoke(
                        request.getSubInterface(),
                        request.getFnId(),
                        *this);
                }
                
                sendResponseUncaughtExceptionGuard.dismiss();
                if (mAutoSend && !mRequest.mOneway)
                {
                    sendResponse();
                }
                else if (mRequest.mOneway)
                {
                    RCF_ASSERT(mAutoSend);
                    RCF2_TRACE("oneway call - suppressing response")(this);
                    mIn.clearByteBuffer();
                    clearParameters();
                    setCurrentRcfSessionPtr();
                    onWriteCompleted();
                }
            }
        }
        catch(const std::exception &e)
        {
            sendResponseUncaughtExceptionGuard.dismiss();
            if (mAutoSend)
            {
                sendResponseException(e);
            }
        }
    }

    void RcfServer::cycleSessions(
        int,                       // timeoutMs, 
        const volatile bool &)     // stopFlag
    {
        //RCF_TRACE("")(timeoutMs);

        //if (mThreadSpecificSessionQueuePtr.get() == NULL)
        //{
        //    mThreadSpecificSessionQueuePtr.reset(new SessionQueue);
        //}

        //while (!stopFlag && !mThreadSpecificSessionQueuePtr->empty())
        //{
        //    RcfSessionPtr sessionPtr = mThreadSpecificSessionQueuePtr->back();
        //    mThreadSpecificSessionQueuePtr->pop_back();
        //    handleSession(sessionPtr);
        //}
    }

    I_ServerTransport &RcfServer::getServerTransport()
    {
        return *getServerTransportPtr();
    }

    I_Service &RcfServer::getServerTransportService()
    {
        return dynamic_cast<I_Service &>(*getServerTransportPtr());
    }

    ServerTransportPtr RcfServer::getServerTransportPtr()
    {
        ReadLock readLock( mServicesMutex );
        RCF_ASSERT( ! mServerTransports.empty() );
        return mServerTransports[0];
    }

    I_IpServerTransport &RcfServer::getIpServerTransport()
    {
        return dynamic_cast<RCF::I_IpServerTransport &>(getServerTransport());
    }    

    bool RcfServer::bindShared(
        const std::string &name,
        const RcfClientPtr &rcfClientPtr)
    {
        RCF_ASSERT(rcfClientPtr.get());
        RCF_TRACE("")(name)(typeid(*rcfClientPtr));

        WriteLock writeLock(mStubMapMutex);
        mStubMap[name] = StubEntryPtr( new StubEntry(rcfClientPtr));
        return true;
    }

    FilterPtr RcfServer::createFilter(int filterId)
    {
        FilterFactoryPtr filterFactoryPtr =
            mFilterServicePtr ?
                mFilterServicePtr->getFilterFactoryPtr(filterId) :
                FilterFactoryPtr();

        return filterFactoryPtr ?
            filterFactoryPtr->createFilter() :
            FilterPtr();
    }

    void RcfServer::setStartCallback(const StartCallback &startCallback)
    {
        mStartCallback = startCallback;
    }

    void RcfServer::invokeStartCallback()
    {
        if (mStartCallback)
        {
            mStartCallback(*this);
        }
    }

    bool RcfServer::getStopFlag() const
    {
        return mServerThreadsStopFlag;
    }

    int RcfServer::getRcfRuntimeVersion()
    {
        return mRcfRuntimeVersion;
    }

    void RcfServer::setRcfRuntimeVersion(int version)
    {
        mRcfRuntimeVersion = version;
    }

    PingBackServicePtr RcfServer::getPingBackServicePtr()
    {
        ReadLock writeLock(mServicesMutex);
        return mPingBackServicePtr;
    }

    FileTransferServicePtr RcfServer::getFileTransferServicePtr()
    {
        ReadLock writeLock(mServicesMutex);
        return mFileTransferServicePtr;
    }

    ObjectFactoryServicePtr RcfServer::getObjectFactoryServicePtr()
    {
        ReadLock writeLock(mServicesMutex);
        return mObjectFactoryServicePtr;
    }

    SessionTimeoutServicePtr RcfServer::getSessionTimeoutServicePtr()
    {
        ReadLock writeLock(mServicesMutex);
        return mSessionTimeoutServicePtr;
    }

} // namespace RCF
