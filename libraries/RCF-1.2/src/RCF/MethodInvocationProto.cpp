
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/Exception.hpp>
#include <RCF/MethodInvocation.hpp>

#ifdef RCF_USE_PROTOBUF

#include <RCF\protobuf\RcfMessages.pb.h>

namespace RCF {

    std::size_t encodeProtobuf(std::ostrstream & os, const google::protobuf::Message & message)
    {
        // Make room for the length field (4 bytes).
        os.write("%%%%", 4);

        std::size_t beginPos = os.tellp();
        RCF_ASSERT(beginPos >= 4);

        // Make room for the protobuf object.
        // TODO: Less obtuse way of reserving space.
        int byteSize = message.ByteSize();
        for (int i=0; i<byteSize; ++i)
        {
            os.write("%", 1);
        }
        std::size_t endPos = os.tellp();
        RCF_ASSERT(endPos > beginPos);

        // Write the protobuf object straight into the underlying buffer.
        char * pch = os.str();
        os.freeze(false);
        bool ok = message.SerializeToArray(pch + beginPos, endPos - beginPos);
        RCF_VERIFY(ok, Exception(_RcfError_ProtobufWrite(typeid(message).name())))(typeid(message));

        // Write the prepended length field.
        boost::uint32_t len = endPos - beginPos;
        char buffer[4] = {0};
        memcpy(buffer, &len, 4);
        RCF::machineToNetworkOrder(buffer, 4, 1);
        os.seekp(beginPos-4);
        os.write(buffer, 4);
        os.seekp(endPos);

        return 4 + endPos - beginPos;
    }

    ByteBuffer encodeProtobuf(
        boost::shared_ptr<std::ostrstream>& osPtr, 
        const google::protobuf::Message & message)
    {
        // TODO: error checking.

        RCF_ASSERT(!osPtr || osPtr.unique());
        if (!osPtr)
        {
            osPtr.reset(new std::ostrstream());
        }

        osPtr->clear();
        osPtr->rdbuf()->freeze(false);
        osPtr->rdbuf()->pubseekoff(0, std::ios::beg, std::ios::out);

        std::size_t pos = encodeProtobuf(*osPtr, message);
        RCF_UNUSED_VARIABLE(pos);

        return ByteBuffer(osPtr->str(), osPtr->tellp(), osPtr);
    }

    std::size_t decodeProtobuf(ByteBuffer buffer, google::protobuf::Message & message)
    {
        // TODO: error checking.

        boost::uint32_t len = 0;
        memcpy( &len, buffer.getPtr(), 4);
        RCF::networkToMachineOrder(&len, 4, 1);
        bool ok = message.ParseFromArray(buffer.getPtr() + 4, len);
        RCF_VERIFY(ok, Exception(_RcfError_ProtobufRead(typeid(message).name())))(typeid(message));
        return 4 + len;
    }

    class Protobufs
    {
    public:
        RequestHeader   mRequestHeader;
        ResponseHeader  mResponseHeader;
        FilterHeader    mFilterHeader;
    };

    void MethodInvocationRequest::initProtobuf()
    {
        mProtobufsPtr.reset( new Protobufs() );
    }

    std::size_t MethodInvocationRequest::decodeRequestHeaderProto(const ByteBuffer & buffer)
    {
        RequestHeader & requestHeader = mProtobufsPtr->mRequestHeader;

        std::size_t pos = decodeProtobuf(buffer, requestHeader);

        mService                    = requestHeader.service();
        mToken                      = Token(requestHeader.token());
        mSubInterface               = requestHeader.subinterface();
        mFnId                       = requestHeader.fnid();
        mSerializationProtocol      = SerializationProtocol( requestHeader.serializationprotocol() );
        mOneway                     = requestHeader.oneway();
        mClose                      = requestHeader.close();
        mRcfRuntimeVersion          = requestHeader.rcfruntimeversion();
        mPingBackIntervalMs         = requestHeader.pingbackinterval();
        mArchiveVersion             = requestHeader.archiveversion();

        return pos;
    }

    ByteBuffer MethodInvocationRequest::encodeRequestHeaderProto()
    {
        RequestHeader & requestHeader = mProtobufsPtr->mRequestHeader;
        requestHeader.Clear();
        requestHeader.set_token(mToken.getId());
        requestHeader.set_subinterface(mSubInterface);
        requestHeader.set_fnid(mFnId);
        requestHeader.set_serializationprotocol(mSerializationProtocol);
        requestHeader.set_oneway(mOneway);
        requestHeader.set_close(mClose);
        requestHeader.set_service(mService);
        requestHeader.set_rcfruntimeversion(mRcfRuntimeVersion);
        requestHeader.set_pingbackinterval(mPingBackIntervalMs);
        requestHeader.set_archiveversion(mArchiveVersion);

        return encodeProtobuf(mOsPtr, requestHeader);
    }

    void MethodInvocationRequest::encodeToMessageProto(
        std::vector<ByteBuffer> &message,
        const std::vector<ByteBuffer> &buffers,
        const std::vector<FilterPtr> &filters)
    {
        FilterHeader & filterHeader = mProtobufsPtr->mFilterHeader;
        filterHeader.Clear();

        if (filters.empty())
        {
            filterHeader.set_unfilteredlen(0);
        }
        else
        {
            filterHeader.set_unfilteredlen(lengthByteBuffers(buffers));
            for (std::size_t i=0; i<filters.size(); ++i)
            {
                filterHeader.add_filterids(filters[i]->getFilterDescription().getId());
            }
        }

        ByteBuffer buffer = encodeProtobuf(mOsPtr, filterHeader);

        if (filters.empty())
        {
            std::copy(
                buffers.begin(), 
                buffers.end(), 
                std::back_inserter(message));
        }
        else
        {
            ThreadLocalCached< std::vector<ByteBuffer> > tlcFilteredBuffers;
            std::vector<ByteBuffer> &filteredBuffers = tlcFilteredBuffers.get();

            std::size_t unfilteredLen = lengthByteBuffers(buffers);
            bool ok = filterData(buffers, filteredBuffers, filters);
            RCF_VERIFY(ok, Exception(_RcfError_FilterMessage()));

            std::copy(
                filteredBuffers.begin(), 
                filteredBuffers.end(), 
                std::back_inserter(message));
        }

        // Copy the filter header into the left margin of the first buffer.
        RCF_ASSERT(
            !message.empty() &&
            message.front().getLeftMargin() >= buffer.getLength())
            (message.front().getLeftMargin())(buffer.getLength());

        ByteBuffer &front = message.front();
        front.expandIntoLeftMargin(buffer.getLength());
        memcpy(front.getPtr(), buffer.getPtr(), buffer.getLength());
    }

    std::size_t MethodInvocationRequest::decodeFromMessageProto(
        const ByteBuffer & message,
        std::vector<int> & filterIds,
        std::size_t & unfilteredLen)
    {
        FilterHeader & filterHeader = mProtobufsPtr->mFilterHeader;

        std::size_t pos = decodeProtobuf(message, filterHeader);
        int filterCount = filterHeader.filterids_size();
        for (int i=0; i<filterCount; ++i)
        {
            filterIds.push_back(filterHeader.filterids(i));
        }
        unfilteredLen = filterHeader.unfilteredlen();
        return pos;
    }

    void MethodInvocationRequest::encodeResponseProto(
        const RemoteException *         pRe,
        ByteBuffer &                    buffer)
    {
        ResponseHeader & responseHeader = mProtobufsPtr->mResponseHeader;
        responseHeader.Clear();

        if (pRe)
        {
            responseHeader.set_responsetype(ResponseHeader::Exception);
            responseHeader.mutable_responseexception()->set_what(pRe->getWhat());
            responseHeader.mutable_responseexception()->set_context(pRe->getContext());
            responseHeader.mutable_responseexception()->set_error(pRe->getError().getErrorId());
            responseHeader.mutable_responseexception()->set_subsystemerror(pRe->getSubSystemError());
            responseHeader.mutable_responseexception()->set_subsystem(pRe->getSubSystem());
            responseHeader.mutable_responseexception()->set_remoteexceptiontype(pRe->getRemoteExceptionType());
        }
        else
        {
            responseHeader.set_responsetype(ResponseHeader::Ok);
        }

        buffer = encodeProtobuf(mOsPtr, responseHeader);
    }

    std::size_t MethodInvocationRequest::decodeResponseHeaderProto(
        const ByteBuffer &              buffer,
        MethodInvocationResponse &      response)
    {
        std::size_t pos = 0;

        ResponseHeader responseHeader;

        pos = decodeProtobuf(buffer, responseHeader);

        if (responseHeader.responsetype() == ResponseHeader::Ok)
        {
            response.mException = false;
            response.mError = false;
        }
        else if (responseHeader.responsetype() == ResponseHeader::Exception)
        {
            response.mException = true;
            response.mError = false;

            response.mExceptionPtr.reset( new RemoteException(
                Error(responseHeader.responseexception().error()), 
                responseHeader.responseexception().subsystemerror(), 
                responseHeader.responseexception().subsystem(), 
                responseHeader.responseexception().what(), 
                responseHeader.responseexception().context(), 
                responseHeader.responseexception().remoteexceptiontype()));
        }
        else
        {
            RCF_ASSERT( responseHeader.responsetype() == ResponseHeader::Error );

            response.mException = false;
            response.mError = true;

            response.mErrorCode = responseHeader.responseerror().error();
            response.mArg0 = responseHeader.responseerror().errorarg();
        }

        return pos;
    }

} // namespace RCF

#else

namespace RCF {

    ByteBuffer MethodInvocationRequest::encodeRequestHeaderProto()
    {
        RCF_THROW( Exception("Protocol Buffer support not enabled.") );
    }

    void MethodInvocationRequest::encodeToMessageProto(
        std::vector<ByteBuffer> &       message,
        const std::vector<ByteBuffer> & buffers,
        const std::vector<FilterPtr> &  filters)
    {
        RCF_UNUSED_VARIABLE(message);
        RCF_UNUSED_VARIABLE(buffers);
        RCF_UNUSED_VARIABLE(filters);

        RCF_THROW( Exception("Protocol Buffer support not enabled.") );
    }

    std::size_t MethodInvocationRequest::decodeRequestHeaderProto(
        const ByteBuffer &              buffer)
    {
        RCF_UNUSED_VARIABLE(buffer);

        RCF_THROW( Exception("Protocol Buffer support not enabled.") );
    }

    std::size_t MethodInvocationRequest::decodeFromMessageProto(
        const ByteBuffer &              message,
        std::vector<int> &              filterIds,
        std::size_t &                   unfilteredLen)
    {
        RCF_UNUSED_VARIABLE(message);
        RCF_UNUSED_VARIABLE(filterIds);
        RCF_UNUSED_VARIABLE(unfilteredLen);

        RCF_THROW( Exception("Protocol Buffer support not enabled.") );
    }

    void MethodInvocationRequest::encodeResponseProto(
        const RemoteException *         pRe,
        ByteBuffer &                    buffer)
    {
        RCF_UNUSED_VARIABLE(pRe);
        RCF_UNUSED_VARIABLE(buffer);

        RCF_THROW( Exception("Protocol Buffer support not enabled.") );
    }

    std::size_t MethodInvocationRequest::decodeResponseHeaderProto(
        const ByteBuffer &              buffer,
        MethodInvocationResponse &      response)
    {
        RCF_UNUSED_VARIABLE(buffer);
        RCF_UNUSED_VARIABLE(response);

        RCF_THROW( Exception("Protocol Buffer support not enabled.") );
    }

    class Protobufs {};

    void MethodInvocationRequest::initProtobuf()
    {
    }

} // namespace RCF

#endif


