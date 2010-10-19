
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/MethodInvocation.hpp>

#include <vector>

#include <boost/mpl/assert.hpp>

#include <RCF/CurrentSession.hpp>
#include <RCF/ObjectFactoryService.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/Service.hpp>
#include <RCF/ThreadLocalCache.hpp>

#include <SF/Encoding.hpp>

namespace RCF {

    void encodeServerError(ByteBuffer & byteBuffer, int error)
    {
        const std::size_t Len = 4+1+1+4;

        if (byteBuffer.getLength() + byteBuffer.getLeftMargin() < Len)
        {
            byteBuffer = ByteBuffer(Len);
        }

        byteBuffer.setLeftMargin(4);

        // without RCF:: qualifiers, borland chooses not to generate any code at all...
        std::size_t pos = 0;
        SF::encodeInt(Descriptor_Error, byteBuffer, pos);
        SF::encodeInt(0, byteBuffer, pos);
        SF::encodeInt(error, byteBuffer, pos);
    }

    void encodeServerError(ByteBuffer & byteBuffer, int error, int arg0)
    {

        const std::size_t Len = 4+1+1+4+4;

        if (byteBuffer.getLength() + byteBuffer.getLeftMargin() < Len)
        {
            byteBuffer = ByteBuffer(Len);
        }

        byteBuffer.setLeftMargin(4);

        // without RCF:: qualifiers, borland chooses not to generate any code at all...
        std::size_t pos = 0;
        SF::encodeInt(Descriptor_Error, byteBuffer, pos);
        SF::encodeInt(0, byteBuffer, pos);
        SF::encodeInt(error, byteBuffer, pos);
        SF::encodeInt(arg0, byteBuffer, pos);
    }

    //*************************************
    // MethodInvocationRequest

    MethodInvocationRequest::MethodInvocationRequest() :
        mToken(),
        mSubInterface(),
        mFnId(RCF_DEFAULT_INIT),
        mSerializationProtocol(DefaultSerializationProtocol),
        mMarshalingProtocol(DefaultMarshalingProtocol),
        mOneway(RCF_DEFAULT_INIT),
        mClose(RCF_DEFAULT_INIT),
        mService(),
        mRcfRuntimeVersion(RCF_DEFAULT_INIT),
        mIgnoreRcfRuntimeVersion(RCF_DEFAULT_INIT),
        mPingBackIntervalMs(RCF_DEFAULT_INIT),
        mArchiveVersion(RCF_DEFAULT_INIT),
        mException(RCF_DEFAULT_INIT)
    {
        initProtobuf();
    }

    void MethodInvocationRequest::init(
        const Token &           token,
        const std::string &     service,
        const std::string &     subInterface,
        int                     fnId,
        SerializationProtocol   serializationProtocol,
        MarshalingProtocol      marshalingProtocol,
        bool                    oneway,
        bool                    close,
        int                     rcfRuntimeVersion,
        bool                    ignoreRcfRuntimeVersion,
        boost::uint32_t         pingBackIntervalMs,
        int                     archiveVersion)
    {
        mToken                      = token;
        mService                    = service;
        mSubInterface               = subInterface;
        mFnId                       = fnId;
        mSerializationProtocol      = serializationProtocol;
        mMarshalingProtocol         = marshalingProtocol;
        mOneway                     = oneway;
        mClose                      = close;
        mRcfRuntimeVersion          = rcfRuntimeVersion;
        mIgnoreRcfRuntimeVersion    = ignoreRcfRuntimeVersion;
        mPingBackIntervalMs         = pingBackIntervalMs;
        mArchiveVersion             = archiveVersion;
        mException = false;
    }

    Token MethodInvocationRequest::getToken() const
    {
        return mToken;
    }

    std::string MethodInvocationRequest::getSubInterface() const
    {
        return mSubInterface;
    }

    int MethodInvocationRequest::getFnId() const
    {
        return mFnId;
    }

    bool MethodInvocationRequest::getOneway() const
    {
        return mOneway;
    }

    bool MethodInvocationRequest::getClose() const
    {
        return mClose;
    }

    std::string MethodInvocationRequest::getService() const
    {
        return mService;
    }

    void MethodInvocationRequest::setService(const std::string &service)
    {
        mService = service;
    }

    int MethodInvocationRequest::getPingBackIntervalMs()
    {
        return mPingBackIntervalMs;
    }

    bool MethodInvocationRequest::decodeRequest(
        const ByteBuffer &message,
        ByteBuffer & messageBody,
        RcfSessionPtr rcfSessionPtr,
        RcfServer &rcfServer)
    {
        ByteBuffer buffer;

        ThreadLocalCached< std::vector<FilterPtr> > tlcNoFilters;
        std::vector<FilterPtr> &noFilters = tlcNoFilters.get();

        // Unfilter the message.
        decodeFromMessage(
            message,
            buffer,
            &rcfServer,
            rcfSessionPtr,
            noFilters);

        // Decode the request header.
        std::size_t pos = 0;
        mRcfRuntimeVersion = 1;
         
        if (mMarshalingProtocol == Mp_Protobuf)
        {
            pos = decodeRequestHeaderProto(buffer);
        }
        else
        {
            int tokenId = 0;
            int msgId = 0;
            int messageVersion = 0;
            
            bool ignoreRuntimeVersion = false;

            SF::decodeInt(msgId, buffer, pos);
            RCF_VERIFY(msgId == Descriptor_Request, Exception(_RcfError_Decoding()))(msgId);
            SF::decodeInt(messageVersion, buffer, pos);
            
            if (messageVersion > 3)
            {
                return false;
            }

            SF::decodeString(mService, buffer, pos);
            SF::decodeInt(tokenId, buffer, pos);
            SF::decodeString(mSubInterface, buffer, pos);
            SF::decodeInt(mFnId, buffer, pos);
            
            int sp = 0;
            SF::decodeInt(sp, buffer, pos);
            mSerializationProtocol = SerializationProtocol(sp);

            SF::decodeBool(mOneway, buffer, pos);
            SF::decodeBool(mClose, buffer, pos);

            if (messageVersion == 1)
            {
                SF::decodeInt(mRcfRuntimeVersion, buffer, pos);
                SF::decodeBool(ignoreRuntimeVersion, buffer, pos);
                mPingBackIntervalMs = 0;
            }
            else if (messageVersion == 2)
            {
                SF::decodeInt(mRcfRuntimeVersion, buffer, pos);
                SF::decodeBool(ignoreRuntimeVersion, buffer, pos);
                SF::decodeInt(mPingBackIntervalMs, buffer, pos);
            }
            else if (messageVersion == 3)
            {
                SF::decodeInt(mRcfRuntimeVersion, buffer, pos);
                SF::decodeBool(ignoreRuntimeVersion, buffer, pos);
                SF::decodeInt(mPingBackIntervalMs, buffer, pos);
                SF::decodeInt(mArchiveVersion, buffer, pos);
            }

            mToken = Token(tokenId);            
        }

        if (mSubInterface.empty())
        {
            mSubInterface = mService;
        }

        if (mRcfRuntimeVersion > rcfServer.getRcfRuntimeVersion())
        {
            return false;
        }
        else
        {
            rcfSessionPtr->setRcfRuntimeVersion(mRcfRuntimeVersion);
        }

        rcfSessionPtr->setArchiveVersion(mArchiveVersion);

        messageBody = ByteBuffer(buffer, pos);

        return true;
    }

    bool MethodInvocationRequest::encodeResponse(
        const RemoteException *         pRe,
        ByteBuffer &                    buffer)
    {
        if (mMarshalingProtocol == Mp_Protobuf)
        {
            encodeResponseProto(pRe, buffer);
            return false;
        }
        else
        {
            bool isException = pRe ? true : false;

            RCF_ASSERT(!mVecPtr || mVecPtr.unique());
            if (!mVecPtr)
            {
                mVecPtr.reset(new std::vector<char>(50));
            }

            std::size_t pos = 0;
            BOOST_STATIC_ASSERT(0 <= Descriptor_Response && Descriptor_Response < 255);
            SF::encodeInt(Descriptor_Response, *mVecPtr, pos);
            SF::encodeInt(0, *mVecPtr, pos);
            SF::encodeBool(isException, *mVecPtr, pos);

            mVecPtr->resize(pos);

            buffer = ByteBuffer(mVecPtr);

            // If there was an exception, it still needs to be serialized. 
            return true;
        }
    }

    ByteBuffer MethodInvocationRequest::encodeRequestHeader()
    {
        if (mMarshalingProtocol == Mp_Protobuf)
        {
            return encodeRequestHeaderProto();
        }
        else
        {
            RCF_ASSERT(!mVecPtr || mVecPtr.unique());
            if (!mVecPtr)
            {
                mVecPtr.reset(new std::vector<char>(50));
            }

            int runtimeVersion = mRcfRuntimeVersion;
            int messageVersion = 0;

            if (runtimeVersion < 2)
            {
                messageVersion = 0;
            }
            else if (runtimeVersion < 5)
            {
                messageVersion = 1;
            }
            else if (runtimeVersion < 6)
            {
                messageVersion = 2;
            }
            else
            {
                messageVersion = 3;
            }

            std::size_t pos = 0;
            SF::encodeInt(Descriptor_Request, *mVecPtr, pos);
            SF::encodeInt(messageVersion, *mVecPtr, pos);
            SF::encodeString(mService, *mVecPtr, pos);
            SF::encodeInt(mToken.getId(), *mVecPtr, pos);

            mSubInterface == mService ?
                SF::encodeString("", *mVecPtr, pos) :
                SF::encodeString(mSubInterface, *mVecPtr, pos);

            SF::encodeInt(mFnId, *mVecPtr, pos);
            SF::encodeInt(mSerializationProtocol, *mVecPtr, pos);
            SF::encodeBool(mOneway, *mVecPtr, pos);
            SF::encodeBool(mClose, *mVecPtr, pos);

            if (messageVersion == 1)
            {
                SF::encodeInt(mRcfRuntimeVersion, *mVecPtr, pos);
                SF::encodeBool(mIgnoreRcfRuntimeVersion, *mVecPtr, pos);
            }
            else if (messageVersion == 2)
            {
                SF::encodeInt(mRcfRuntimeVersion, *mVecPtr, pos);
                SF::encodeBool(mIgnoreRcfRuntimeVersion, *mVecPtr, pos);
                SF::encodeInt(mPingBackIntervalMs, *mVecPtr, pos);
            }
            else if (messageVersion == 3)
            {
                SF::encodeInt(mRcfRuntimeVersion, *mVecPtr, pos);
                SF::encodeBool(mIgnoreRcfRuntimeVersion, *mVecPtr, pos);
                SF::encodeInt(mPingBackIntervalMs, *mVecPtr, pos);
                SF::encodeInt(mArchiveVersion, *mVecPtr, pos);
            }

            mVecPtr->resize(pos);

            return ByteBuffer(mVecPtr);
        }
    }

    void MethodInvocationRequest::encodeRequest(
        const std::vector<ByteBuffer> &buffers,
        std::vector<ByteBuffer> &message,
        const std::vector<FilterPtr> &filters)
    {
        encodeToMessage(
            message,
            buffers,
            filters);
    }

    void MethodInvocationRequest::decodeResponse(
        const ByteBuffer &message,
        ByteBuffer &buffer,
        MethodInvocationResponse &response,
        const std::vector<FilterPtr> &filters)
    {
        decodeFromMessage(
            message,
            buffer,
            NULL,
            RcfSessionPtr(),
            filters);

        std::size_t pos = 0;

        if (mMarshalingProtocol == Mp_Protobuf)
        {
            pos = decodeResponseHeaderProto(buffer, response);
        }
        else
        {
            // Decode response header.
            int msgId = 0;
            SF::decodeInt(msgId, buffer, pos);

            int ver = 0;
            SF::decodeInt(ver, buffer, pos);
           
            if (msgId == Descriptor_Error)
            {
                RCF_VERIFY(ver == 0, Exception(_RcfError_Decoding()))(ver);

                int error = 0;
                SF::decodeInt(error, buffer, pos);
                response.mException = false;
                response.mError = true;
                response.mErrorCode = error;
                if (
                    error == RcfError_VersionMismatch ||
                    error == RcfError_PingBack)
                {
                    SF::decodeInt(response.mArg0, buffer, pos);
                }
            }
            else
            {
                RCF_VERIFY(msgId == Descriptor_Response, Exception(_RcfError_Decoding()))(msgId);
                RCF_VERIFY(ver == 0, Exception(_RcfError_Decoding()))(ver);

                SF::decodeBool(response.mException, buffer, pos);

                response.mError = false;
                response.mErrorCode = 0;
                response.mArg0 = 0;
            }
        }

        // Return the response body.
        buffer = ByteBuffer(buffer, pos);
    }

    StubEntryPtr MethodInvocationRequest::locateStubEntryPtr(
        RcfServer &rcfServer)
    {
        Token targetToken = getToken();
        std::string targetName = getService();
        StubEntryPtr stubEntryPtr;
        RcfSessionPtr rcfSessionPtr = getCurrentRcfSessionPtr();
        if (targetToken != Token())
        {
            ReadLock readLock(rcfServer.mServicesMutex);
            if (rcfServer.mObjectFactoryServicePtr)
            {
                stubEntryPtr = rcfServer.mObjectFactoryServicePtr
                    ->getStubEntryPtr(targetToken);
            }
        }
        else if (!targetName.empty())
        {
            ReadLock readLock(rcfServer.mStubMapMutex);
            std::string servantName = getService();
            RcfServer::StubMap::iterator iter = rcfServer.mStubMap.find(servantName);
            if (iter != rcfServer.mStubMap.end())
            {
                stubEntryPtr = (*iter).second;
            }
        }
        else
        {
            stubEntryPtr = rcfSessionPtr->getDefaultStubEntryPtr();
        }

        return stubEntryPtr;
    }

   
    //*******************************************
    // MethodInvocationResponse

    MethodInvocationResponse::MethodInvocationResponse() :
        mException(RCF_DEFAULT_INIT),
        mError(RCF_DEFAULT_INIT),
        mErrorCode(RCF_DEFAULT_INIT),
        mArg0(RCF_DEFAULT_INIT)
    {}

    bool MethodInvocationResponse::isException() const
    {
        return mException;
    }

    bool MethodInvocationResponse::isError() const
    {
        return mError;
    }

    int MethodInvocationResponse::getError() const
    {
        return mErrorCode;
    }

    int MethodInvocationResponse::getArg0() const
    {
        return mArg0;
    }

    std::auto_ptr<RemoteException> MethodInvocationResponse::getExceptionPtr()
    {
        return mExceptionPtr;
    }

    //*******************************************

    void MethodInvocationRequest::encodeToMessage(
        std::vector<ByteBuffer> &message,
        const std::vector<ByteBuffer> &buffers,
        const std::vector<FilterPtr> &filters)
    {
        if (mMarshalingProtocol == Mp_Protobuf)
        {
            encodeToMessageProto(message, buffers, filters);
        }
        else
        {
            if (filters.empty())
            {
                // No filters, so nothing to do really.

                message.resize(0);

                std::copy(
                    buffers.begin(),
                    buffers.end(),
                    std::back_inserter(message));
            }
            else
            {
                // Pass the buffers through the filters.

                ThreadLocalCached< std::vector<ByteBuffer> > tlcFilteredBuffers;
                std::vector<ByteBuffer> &filteredBuffers = tlcFilteredBuffers.get();

                std::size_t unfilteredLen = lengthByteBuffers(buffers);
                bool ok = filterData(buffers, filteredBuffers, filters);
                RCF_VERIFY(ok, Exception(_RcfError_FilterMessage()));

                message.resize(0);

                std::copy(
                    filteredBuffers.begin(),
                    filteredBuffers.end(),
                    std::back_inserter(message));

                if (filteredBuffers.empty())
                {
                    // Can happen if buffers has e.g. a single zero length buffer.
                    RCF_ASSERT(lengthByteBuffers(buffers) == 0);
                    RCF_ASSERT(!buffers.empty());
                    RCF_ASSERT(buffers.front().getLength() == 0);
                    message.push_back(buffers.front());
                }

                // Encode the filter header.
                const std::size_t VecLen = (5+10)*4;
                char vec[VecLen];
                ByteBuffer byteBuffer(&vec[0], VecLen);

                std::size_t pos = 0;

                // Descriptor
                SF::encodeInt(Descriptor_FilteredPayload, byteBuffer, pos);

                // Version.
                SF::encodeInt(0, byteBuffer, pos);

                RCF_VERIFY(
                    filters.size() <= 10,
                    Exception(_RcfError_FilterCount(filters.size(), 10)));

                // Number of filters.
                SF::encodeInt(static_cast<int>(filters.size()), byteBuffer, pos);

                // Filter ids.
                for (std::size_t i=0; i<filters.size(); ++i)
                {
                    int filterId = filters[i]->getFilterDescription().getId();
                    SF::encodeInt(filterId, byteBuffer, pos);
                }

                // Legacy, not used anymore.
                int filterHeaderLen = 0;
                SF::encodeInt(filterHeaderLen, byteBuffer, pos);

                // Length of the message when it is unfiltered.
                int len = static_cast<int>(unfilteredLen);
                SF::encodeInt(len, byteBuffer, pos);

                // Copy the filter header into the left margin of the first buffer.
                RCF_ASSERT(pos <= VecLen);
                std::size_t headerLen = pos;
                
                RCF_ASSERT(
                    !message.empty() &&
                    message.front().getLeftMargin() >= headerLen)
                    (message.front().getLeftMargin())(headerLen);

                ByteBuffer &front = message.front();
                front.expandIntoLeftMargin(headerLen);
                memcpy(front.getPtr(), &vec[0], headerLen);
            }
        }
    }

    struct FilterIdComparison
    {
        bool operator()(FilterPtr filterPtr, int filterId)
        {
            return filterPtr->getFilterDescription().getId() == filterId;
        }
    };

    void MethodInvocationRequest::decodeFromMessage(
        const ByteBuffer &message,
        ByteBuffer &buffer,
        RcfServer *pRcfServer,
        RcfSessionPtr rcfSessionPtr,
        const std::vector<FilterPtr> &existingFilters)
    {
        ThreadLocalCached< std::vector<int> > tlcFilterIds;
        std::vector<int> &filterIds = tlcFilterIds.get();

        // Decode the filter header, if there is one.
        std::size_t pos = 0;
        std::size_t unfilteredLen = 0;

        // On the server, simple heuristics to determine which marshaling 
        // protocol is being used.
        if (pRcfServer)
        {
            if (
                    message.getLength() >= 4 
                &&  message.getPtr()[1] == 0 
                &&  message.getPtr()[2] == 0 
                &&  message.getPtr()[3] == 0)
            {
                mMarshalingProtocol = Mp_Protobuf;
            }
            else
            {
                mMarshalingProtocol = Mp_Rcf;
            }
        }

        if (mMarshalingProtocol == Mp_Protobuf)
        {
            pos = decodeFromMessageProto(message, filterIds, unfilteredLen);
        }
        else
        {
            char * const pch = (char*) message.getPtr() ;
            if (pch[0] == Descriptor_FilteredPayload)
            {
                int descriptor = 0;
                SF::decodeInt(descriptor, message, pos);
                RCF_VERIFY(descriptor == Descriptor_FilteredPayload, Exception(_RcfError_Decoding()))(descriptor);

                int version = 0;
                SF::decodeInt(version, message, pos);
                RCF_VERIFY(version == 0, Exception(_RcfError_Decoding()))(version);

                int filterCount = 0;
                SF::decodeInt(filterCount, message, pos);
                RCF_VERIFY(0 < filterCount && filterCount <= 10, Exception(_RcfError_Decoding()))(filterCount);

                filterIds.resize(0);
                for (int i=0; i<filterCount; ++i)
                {
                    int filterId = 0;
                    SF::decodeInt(filterId, message, pos);
                    filterIds.push_back(filterId);
                }

                int clearLen = 0;
                SF::decodeInt(clearLen, message, pos);
                RCF_VERIFY(0 <= clearLen, Exception(_RcfError_Decoding()))(clearLen);
               
                int unfilteredLen_ = 0;
                SF::decodeInt(unfilteredLen_, message, pos);
                RCF_VERIFY(0 <= unfilteredLen, Exception(_RcfError_Decoding()))(unfilteredLen);
                unfilteredLen = unfilteredLen_;
            }
        }

        ByteBuffer filteredData(message, pos);

        if (pRcfServer)
        {
            // Server side decoding.

            if (filterIds.empty())
            {
                rcfSessionPtr->setFiltered(false);
                buffer = filteredData;
            }
            else
            {
                rcfSessionPtr->setFiltered(true);
                std::vector<FilterPtr> &filters = rcfSessionPtr->getFilters();

                if (    filters.size() != filterIds.size() 
                    ||    !std::equal(
                            filters.begin(),
                            filters.end(),
                            filterIds.begin(),
                            FilterIdComparison()))
                {
                    filters.clear();

                    std::transform(
                        filterIds.begin(),
                        filterIds.end(),
                        std::back_inserter(filters),
                        boost::bind( &RcfServer::createFilter, pRcfServer, _1) );

                    if (
                        std::find_if(
                            filters.begin(),
                            filters.end(),
                            SharedPtrIsNull()) == filters.end())
                    {
                        connectFilters(filters);
                    }
                    else
                    {
                        // TODO: better not to throw exceptions here?
                        RCF_THROW(Exception(_RcfError_UnknownFilter()));
                    }
                }

                bool bRet = unfilterData(
                    filteredData,
                    buffer,
                    unfilteredLen,
                    filters);

                RCF_ASSERT(bRet);
            }
        }
        else
        {
            // Client side decoding.

            if (    existingFilters.size() == filterIds.size() 
                &&    std::equal(
                        existingFilters.begin(),
                        existingFilters.end(),
                        filterIds.begin(),
                        FilterIdComparison()))
            {
                if (existingFilters.empty())
                {
                    buffer = filteredData;
                }
                else
                {
                    bool bRet = unfilterData(
                        filteredData,
                        buffer,
                        unfilteredLen,
                        existingFilters);

                    RCF_VERIFY(bRet, Exception(_RcfError_UnfilterMessage()));
                }
            }
            else
            {
                RCF_THROW(
                    Exception(_RcfError_PayloadFilterMismatch()))
                    (filterIds.size())(existingFilters.size());
            }
        }
    }

} // namespace RCF
