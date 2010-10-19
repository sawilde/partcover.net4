
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/ObjectFactoryService.hpp>
#include <RCF/SessionObjectFactoryService.hpp>

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/RcfServer.hpp>
#include <RCF/ServerInterfaces.hpp>
#include <RCF/StubEntry.hpp>
#include <RCF/StubFactory.hpp>

#ifdef RCF_USE_PROTOBUF
#include <RCF/protobuf/RcfMessages.pb.h>
#endif

namespace RCF {

    ObjectFactoryService::ObjectFactoryService(
        unsigned int numberOfTokens,
        unsigned int clientStubTimeoutS,
        unsigned int cleanupIntervalS,
        float cleanupThreshold) :
            mTokenFactory(numberOfTokens),
            mClientStubTimeoutS(clientStubTimeoutS),            
            mCleanupIntervalS(cleanupIntervalS),
            mCleanupThreshold(cleanupThreshold),
            mStubMapMutex(WriterPriority),
            mStopFlag(RCF_DEFAULT_INIT)
    {
        RCF_ASSERT(0.0 <= cleanupThreshold && mCleanupThreshold <= 1.0);

        // up-front initialization, before threads get into the picture
        typedef std::vector<Token>::const_iterator Iter;
        for (
            Iter iter = mTokenFactory.getTokenSpace().begin();
            iter != mTokenFactory.getTokenSpace().end();
            ++iter)
        {
            mStubMap[*iter].first.reset(new Mutex());
        }
    }

    // remotely accessible
    boost::int32_t ObjectFactoryService::createObject(
        const std::string &objectName,
        Token &token)
    {
        RCF_TRACE("")(objectName);

        // TODO: seems unnecessary to be triggering a sweep here
        std::size_t nAvail = mTokenFactory.getAvailableTokenCount();
        std::size_t nTotal = mTokenFactory.getTokenSpace().size();
        float used = float(nTotal - nAvail) / float(nTotal);
        if (used > mCleanupThreshold)
        {
            mCleanupThresholdCondition.notify_one();
        }

        boost::int32_t ret = RcfError_Ok;

        StubFactoryPtr stubFactoryPtr( getStubFactory( objectName));
        if (stubFactoryPtr.get())
        {
            RcfClientPtr rcfClientPtr( stubFactoryPtr->makeServerStub());
            StubEntryPtr stubEntryPtr( new StubEntry(rcfClientPtr));

            ret = addObject(TokenMappedPtr(stubEntryPtr), token);
            
            if (ret == RcfError_Ok)
            {
                getCurrentRcfSession().setCachedStubEntryPtr(stubEntryPtr);
            }
        }
        else
        {
            ret = RcfError_ObjectFactoryNotFound;
        }

        return ret;
    }

    boost::int32_t ObjectFactoryService::addObject(
        TokenMappedPtr tokenMappedPtr, 
        Token & token)
    {
        // TODO: exception safety
        Token myToken;
        bool ok = mTokenFactory.requestToken(myToken);
        if (ok)
        {
            WriteLock writeLock(mStubMapMutex);
            RCF_UNUSED_VARIABLE(writeLock);
            RCF_ASSERT(mStubMap.find(myToken) != mStubMap.end())(myToken);
            mStubMap[myToken].second = tokenMappedPtr;
            token = myToken;
            return RcfError_Ok;
        }
        else
        {
            return RcfError_TokenRequestFailed;
        }
    }

    Token ObjectFactoryService::addObjectImpl(RcfClientPtr rcfClientPtr)
    {
        Token myToken;
        bool ok = mTokenFactory.requestToken(myToken);
        RCF_VERIFY(ok, Exception(_RcfError_TokenRequestFailed()));
        WriteLock writeLock(mStubMapMutex);
        RCF_UNUSED_VARIABLE(writeLock);
        RCF_ASSERT(mStubMap.find(myToken) != mStubMap.end())(myToken);
        StubEntryPtr stubEntryPtr(new StubEntry(rcfClientPtr));
        mStubMap[myToken].second = stubEntryPtr;
        return myToken;        
    }

    // remotely accessible
    boost::int32_t ObjectFactoryService::createSessionObject(
        const std::string &objectName)
    {
        StubFactoryPtr stubFactoryPtr( getStubFactory( objectName));
        if (stubFactoryPtr.get())
        {
            RcfClientPtr rcfClientPtr( stubFactoryPtr->makeServerStub());
            getCurrentRcfSession().setDefaultStubEntryPtr(
                StubEntryPtr( new StubEntry(rcfClientPtr)));
            return RcfError_Ok;
        }
        return RcfError_ObjectFactoryNotFound;
    }

    // remotely accessible
    boost::int32_t SessionObjectFactoryService::createSessionObject(
        const std::string &objectName)
    {
        StubFactoryPtr stubFactoryPtr( getStubFactory( objectName));
        if (stubFactoryPtr.get())
        {
            RcfClientPtr rcfClientPtr( stubFactoryPtr->makeServerStub());
            getCurrentRcfSession().setDefaultStubEntryPtr(
                StubEntryPtr( new StubEntry(rcfClientPtr)));
            return RcfError_Ok;
        }
        return RcfError_ObjectFactoryNotFound;
    }

    // remotely accessible
    boost::int32_t ObjectFactoryService::deleteObject(const Token &token)
    {
        WriteLock writeLock(mStubMapMutex);
        RCF_UNUSED_VARIABLE(writeLock);

        if (mStubMap.find(token) == mStubMap.end())
        {
            return RcfError_Unspecified;
        }
        else
        {
            mStubMap[token].second.reset();
            mTokenFactory.returnToken(token);
            RCF_TRACE("Token returned")(token);
            return RcfError_Ok;
        }
    }

    // remotely accessible
    boost::int32_t ObjectFactoryService::deleteSessionObject()
    {
        getCurrentRcfSession().setDefaultStubEntryPtr(StubEntryPtr());
        return RcfError_Ok;
    }

    // remotely accessible
    boost::int32_t SessionObjectFactoryService::deleteSessionObject()
    {
        getCurrentRcfSession().setDefaultStubEntryPtr(StubEntryPtr());
        return RcfError_Ok;
    }

    StubEntryPtr ObjectFactoryService::getStubEntryPtr(const Token &token)
    {
        return boost::dynamic_pointer_cast<StubEntry>(
            getTokenMappedPtr(token));
    }

    TokenMappedPtr ObjectFactoryService::getTokenMappedPtr(const Token &token)
    {
        ReadLock readLock(mStubMapMutex);
        RCF_VERIFY(
            mStubMap.find(token) != mStubMap.end(),
            Exception(_RcfError_DynamicObjectNotFound(token.getId())))
            (token);

        Lock lock(*mStubMap[token].first);
        TokenMappedPtr tokenMappedPtr = mStubMap[token].second;
        return tokenMappedPtr;
    }

#ifdef RCF_USE_PROTOBUF

    class ObjectFactoryServicePb
    {
    public:
        ObjectFactoryServicePb(ObjectFactoryService& ofs) : mOfs(ofs)
        {
        }

        CreateRemoteObjectResponse createRemoteObject(const CreateRemoteObject & request)
        {
            Token token;
            int error = mOfs.createObject(request.objectname(), token);

            if (error != RCF::RcfError_Ok)
            {
                RCF_THROW( RemoteException( Error(error) ) );
            }

            CreateRemoteObjectResponse response;
            response.set_token( token.getId() );
            return CreateRemoteObjectResponse();
        }

        void deleteRemoteObject(const DeleteRemoteObject & request)
        {
            int error = mOfs.deleteObject(request.token());

            if (error != RCF::RcfError_Ok)
            {
                RCF_THROW( RemoteException( Error(error) ) );
            }
        }

    private:

        ObjectFactoryService & mOfs;

    };

    void onServiceAddedProto(ObjectFactoryService & ofs, RcfServer & server)
    {
        boost::shared_ptr<ObjectFactoryServicePb> ofsPbPtr( 
            new ObjectFactoryServicePb(ofs) );

        server.bind((I_ObjectFactoryPb *) NULL, ofsPbPtr);
    }

    void onServiceRemovedProto(ObjectFactoryService &, RcfServer & server)
    {
        server.unbind( (I_ObjectFactoryPb *) NULL);
    }

#else

    void onServiceAddedProto(ObjectFactoryService &, RcfServer &)
    {
    }

    void onServiceRemovedProto(ObjectFactoryService &, RcfServer &)
    {
    }

#endif // RCF_USE_PROTOBUF

    void ObjectFactoryService::onServiceAdded(RcfServer &server)
    {
        server.bind((I_ObjectFactory *) NULL, *this);

        onServiceAddedProto(*this, server);

        {
            WriteLock writeLock(getTaskEntriesMutex());
            getTaskEntries().clear();
            getTaskEntries().push_back(
                TaskEntry(
                boost::bind(&ObjectFactoryService::cycleCleanup, this, _1, _2),
                boost::bind(&ObjectFactoryService::stopCleanup, this),
                "RCF Ofs cleanup"));
        }
        mStopFlag = false;
    }

    void ObjectFactoryService::onServiceRemoved(RcfServer &server)
    {
        server.unbind( (I_ObjectFactory *) NULL);

        onServiceRemovedProto(*this, server);
    }

    void ObjectFactoryService::onServerStart(RcfServer &)
    {
    }

    void ObjectFactoryService::onServerStop(RcfServer &)
    {
        mStopFlag = false;
    }

    void ObjectFactoryService::stopCleanup()
    {
        mStopFlag = true;
        Lock lock(mCleanupThresholdMutex);
        mCleanupThresholdCondition.notify_one();
    }

    bool ObjectFactoryService::cycleCleanup(
        int timeoutMs,
        const volatile bool &stopFlag)
    {
        if (timeoutMs == 0)
        {
            cleanupStubMap(mClientStubTimeoutS);
        }
        else
        {
            Lock lock(mCleanupThresholdMutex);
            if (!stopFlag && !mStopFlag)
            {
                unsigned int cleanupIntervalMs = 1000*mCleanupIntervalS;
                mCleanupThresholdCondition.timed_wait(lock, cleanupIntervalMs);
                if (!stopFlag && !mStopFlag)
                {
                    cleanupStubMap(mClientStubTimeoutS);
                }
                else
                {
                    return true;
                }
            }
        }
        return stopFlag || mStopFlag;
    }

    StubFactoryRegistry::StubFactoryRegistry() :
        mStubFactoryMapMutex(WriterPriority)
    {}

    bool StubFactoryRegistry::insertStubFactory(
        const std::string &objectName,
        const std::string &desc,
        StubFactoryPtr stubFactoryPtr)
    {
        RCF_UNUSED_VARIABLE(desc);
        WriteLock writeLock(mStubFactoryMapMutex);
        mStubFactoryMap[ objectName ] = stubFactoryPtr;
        return true;
    }

    bool StubFactoryRegistry::removeStubFactory(
        const std::string &objectName)
    {
        WriteLock writeLock(mStubFactoryMapMutex);
        mStubFactoryMap.erase(mStubFactoryMap.find(objectName));
        return true;
    }

    StubFactoryPtr StubFactoryRegistry::getStubFactory(
        const std::string &objectName)
    {
        ReadLock readLock(mStubFactoryMapMutex);
        return mStubFactoryMap.find(objectName)  != mStubFactoryMap.end() ?
            mStubFactoryMap[objectName] :
            StubFactoryPtr();
    }

    void ObjectFactoryService::cleanupStubMap(unsigned int timeoutS)
    {
        // Clean up the stub map
        RCF_TRACE("");
        std::size_t nAvail = mTokenFactory.getAvailableTokenCount();
        std::size_t nTotal = mTokenFactory.getTokenSpace().size();
        float used = float(nTotal - nAvail) / float(nTotal);
        if (used > mCleanupThreshold)
        {
            typedef std::vector<Token>::const_iterator Iter;
            for (
                Iter iter = mTokenFactory.getTokenSpace().begin();
                iter != mTokenFactory.getTokenSpace().end();
                ++iter)
            {
                Token token = *iter;

                bool removeStub = false;
                {
                    ReadLock readLock(mStubMapMutex);
                    RCF_ASSERT(mStubMap.find(token) != mStubMap.end())(token);
                    Lock lock(*mStubMap[token].first);
                    TokenMappedPtr & tokenMappedPtr = mStubMap[token].second;
                    if (
                        tokenMappedPtr.get() &&
                        tokenMappedPtr.unique() &&
                        tokenMappedPtr->getElapsedTimeS() > timeoutS)
                    {
                        removeStub = true;
                        tokenMappedPtr.reset();
                    }
                }
                if (removeStub)
                {
                    mTokenFactory.returnToken(token);
                    RCF_TRACE("Token returned")(token);
                }
            }
        }
    }

} // namespace RCF
