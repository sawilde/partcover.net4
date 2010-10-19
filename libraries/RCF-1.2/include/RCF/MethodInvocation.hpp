
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_METHODINVOCATION_HPP
#define INCLUDE_RCF_METHODINVOCATION_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <RCF/AsyncFilter.hpp>
#include <RCF/ByteBuffer.hpp>
#include <RCF/Export.hpp>
#include <RCF/Exception.hpp>
#include <RCF/Protocol/Protocol.hpp>
#include <RCF/Token.hpp>

namespace RCF {

    class RcfServer;
    class StubEntry;
    typedef boost::shared_ptr<StubEntry> StubEntryPtr;
    class RcfSession;
    typedef boost::shared_ptr<RcfSession> RcfSessionPtr;
    class SerializationProtocolIn;
    class SerializationProtocolOut;

    class MethodInvocationResponse;

    // message types
    static const int Descriptor_Error               = 0;
    static const int Descriptor_Request             = 1;
    static const int Descriptor_Response            = 2;
    static const int Descriptor_FilteredPayload     = 3;

    void encodeServerError(ByteBuffer & byteBuffer, int error);

    void encodeServerError(ByteBuffer & byteBuffer, int error, int arg0);

    class Protobufs;

    class RCF_EXPORT MethodInvocationRequest
    {
    public:
        MethodInvocationRequest();

        void            init(
                            const Token &                   token,
                            const std::string &             service,
                            const std::string &             subInterface,
                            int                             fnId,
                            SerializationProtocol           serializationProtocol,
                            MarshalingProtocol              marshalingProtocol,
                            bool                            oneway,
                            bool                            close,
                            int                             rcfRuntimeVersion,
                            bool                            ignoreRcfRuntimeVersion,
                            boost::uint32_t                 pingBackIntervalMs,
                            int                             archiveVersion);

        Token           getToken() const;
        std::string     getSubInterface() const;
        int             getFnId() const;
        bool            getOneway() const;
        bool            getClose() const;
        std::string     getService() const;
        void            setService(const std::string &service);
        int             getPingBackIntervalMs();

        ByteBuffer      encodeRequestHeader();

        void            encodeRequest(
                            const std::vector<ByteBuffer> & buffers,
                            std::vector<ByteBuffer> &       message,
                            const std::vector<FilterPtr> &  filters);

        bool            decodeRequest(
                            const ByteBuffer &              message,
                            ByteBuffer &                    messageBody,
                            RcfSessionPtr                   rcfSessionPtr,
                            RcfServer &                     rcfServer);

        bool            encodeResponse(
                            const RemoteException *         pRe,
                            ByteBuffer &                    buffer);

        void            decodeResponse(
                            const ByteBuffer &              message,
                            ByteBuffer &                    buffer,
                            MethodInvocationResponse &      response,
                            const std::vector<FilterPtr> &  filters);

        StubEntryPtr    locateStubEntryPtr(
                            RcfServer &                     rcfServer);

    private:

        friend class RcfSession;

        void            decodeFromMessage(
                            const ByteBuffer &              message,
                            ByteBuffer &                    buffer,
                            RcfServer *                     pRcfServer,
                            RcfSessionPtr                   rcfSessionPtr,
                            const std::vector<FilterPtr> &  existingFilters);

        void            encodeToMessage(
                            std::vector<ByteBuffer> &       message,
                            const std::vector<ByteBuffer> & buffers,
                            const std::vector<FilterPtr> &  filters);


        // Protocol Buffer functionality.
        ByteBuffer      encodeRequestHeaderProto();

        void            encodeToMessageProto(
                            std::vector<ByteBuffer> &       message,
                            const std::vector<ByteBuffer> & buffers,
                            const std::vector<FilterPtr> &  filters);

        std::size_t     decodeRequestHeaderProto(
                            const ByteBuffer &              buffer);

        std::size_t     decodeFromMessageProto(
                            const ByteBuffer &              message,
                            std::vector<int> &              filterIds,
                            std::size_t &                   unfilteredLen);

        void            encodeResponseProto(
                            const RemoteException *         pRe,
                            ByteBuffer &                    buffer);

        std::size_t     decodeResponseHeaderProto(
                            const ByteBuffer &              buffer,
                            MethodInvocationResponse &      response);

        void            initProtobuf();

        Token                   mToken;
        std::string             mSubInterface;
        int                     mFnId;
        SerializationProtocol   mSerializationProtocol;
        MarshalingProtocol      mMarshalingProtocol;
        bool                    mOneway;
        bool                    mClose;
        std::string             mService;
        int                     mRcfRuntimeVersion;
        bool                    mIgnoreRcfRuntimeVersion; // Legacy field, no longer used.
        int                     mPingBackIntervalMs;
        int                     mArchiveVersion;
        bool                    mException;

        boost::shared_ptr<std::vector<char> >   mVecPtr;
        
        // Protobuf specific stuff.
        boost::shared_ptr<std::ostrstream>      mOsPtr;
        boost::shared_ptr<Protobufs>            mProtobufsPtr;
    };

    class RCF_EXPORT MethodInvocationResponse
    {
    public:
        MethodInvocationResponse();

        bool    isException() const;
        bool    isError() const;
        int     getError() const;
        int     getArg0() const;

        std::auto_ptr<RemoteException> getExceptionPtr();

    private:
        friend class MethodInvocationRequest;

        typedef std::auto_ptr<RemoteException> RemoteExceptionPtr;

        bool                mException;
        RemoteExceptionPtr  mExceptionPtr;
        bool                mError;
        int                 mErrorCode;
        int                 mArg0;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_METHODINVOCATION_HPP
