
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_EXCEPTION_HPP
#define INCLUDE_RCF_EXCEPTION_HPP

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <RCF/util/DefaultInit.hpp>
#include <RCF/Export.hpp>
#include <RCF/SerializationDefs.hpp>
#include <RCF/Tools.hpp>
#include <RCF/TypeTraits.hpp>

#include <boost/version.hpp>

#if defined(RCF_USE_BOOST_SERIALIZATION) && BOOST_VERSION > 103301
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/basic_text_iarchive.hpp>
#endif

namespace SF {
    class Archive;
}

namespace RCF {

    class Error
    {
    public:
        Error() : mErrorId(0)
        {
        }

        Error(int errorId) : mErrorId(errorId)
        {
        }
        
        Error(
            int errorId, 
            const std::string & arg1) : 
                mErrorId(errorId)
        {
            mArgs.push_back(arg1);
        }

        Error(
            int errorId, 
            const std::string & arg1, 
            const std::string & arg2) : 
                mErrorId(errorId)
        {
            mArgs.push_back(arg1);
            mArgs.push_back(arg2);
        }

        Error(
            int errorId, 
            const std::string & arg1, 
            const std::string & arg2, 
            const std::string & arg3) : 
                mErrorId(errorId)
        {
            mArgs.push_back(arg1);
            mArgs.push_back(arg2);
            mArgs.push_back(arg3);
        }

        Error(const Error & rhs) : mErrorId(rhs.mErrorId), mArgs(rhs.mArgs)
        {
        }

        int getErrorId() const
        {
            return mErrorId;
        }

        void setErrorId(int errorId)
        {
            mErrorId = errorId;
        }

        const std::vector<std::string> & getArgs() const
        {
            return mArgs;
        }

        std::string getErrorString() const;

#ifdef RCF_USE_SF_SERIALIZATION
        void serialize(SF::Archive & ar);

#endif

#ifdef RCF_USE_BOOST_SERIALIZATION
        template<typename Archive>
        void serialize(Archive & ar, const unsigned int)
        {
            ar & mErrorId & mArgs;
        }
#endif

    private:

        std::string getRawErrorString() const;

        int                             mErrorId;
        std::vector<std::string>        mArgs;
    };

    // RCF error codes
    // range 0-1000 reserved for RCF, remaining range can be used independently of RCF

    static const int RcfError_Ok                                =  0;
    static const int RcfError_Unspecified                       =  1;
    static const int RcfError_ServerMessageLength               =  2;
    static const int RcfError_ClientMessageLength               =  3;
    static const int RcfError_Serialization                     =  4;
    static const int RcfError_Deserialization                   =  5;
    static const int RcfError_UserModeException                 =  6;
    static const int RcfError_UnknownEndpoint                   =  8;
    static const int RcfError_EndpointPassword                  =  9;
    static const int RcfError_EndpointDown                      = 10;
    static const int RcfError_EndpointRetry                     = 11;
    static const int RcfError_ClientConnectTimeout              = 16;
    static const int RcfError_PeerDisconnect                    = 17;
    static const int RcfError_ClientCancel                      = 18;
    static const int RcfError_PayloadFilterMismatch             = 20;
    static const int RcfError_OpenSslFilterInit                 = 21;
    static const int RcfError_OpenSslLoadCert                   = 22;
    static const int RcfError_UnknownPublisher                  = 23;
    static const int RcfError_UnknownFilter                     = 24;
    static const int RcfError_NoServerStub                      = 25;
    static const int RcfError_Sspi                              = 26;
    static const int RcfError_SspiInit                          = 28;
    static const int RcfError_ClientReadTimeout                 = 30;
    static const int RcfError_ClientReadFail                    = 31;
    static const int RcfError_ClientWriteTimeout                = 32;
    static const int RcfError_ClientWriteFail                   = 33;
    static const int RcfError_ClientConnectFail                 = 34;
    static const int RcfError_Socket                            = 36;
    static const int RcfError_FnId                              = 37;
    static const int RcfError_UnknownInterface                  = 38;
    static const int RcfError_NoEndpoint                        = 39;
    static const int RcfError_TransportCreation                 = 40;
    static const int RcfError_FilterCount                       = 41;
    static const int RcfError_FilterMessage                     = 42;
    static const int RcfError_UnfilterMessage                   = 43;
    static const int RcfError_SspiCredentials                   = 44;
    static const int RcfError_SspiEncrypt                       = 45;
    static const int RcfError_SspiDecrypt                       = 46;
    static const int RcfError_SspiImpersonation                 = 47;
    static const int RcfError_SocketClose                       = 49;
    static const int RcfError_ZlibDeflate                       = 50;
    static const int RcfError_ZlibInflate                       = 51;
    static const int RcfError_Zlib                              = 52;
    static const int RcfError_UnknownSerializationProtocol      = 53;
    static const int SfError_NoCtor                             = 55;
    static const int SfError_RefMismatch                        = 56;
    static const int SfError_DataFormat                         = 57;
    static const int SfError_ReadFailure                        = 58;
    static const int SfError_WriteFailure                       = 59;
    static const int SfError_BaseDerivedRegistration            = 60;
    static const int SfError_TypeRegistration                   = 61;
    static const int RcfError_NonStdException                   = 62;
    static const int RcfError_SocketBind                        = 63;
    static const int RcfError_Decoding                          = 64;
    static const int RcfError_Encoding                          = 65;
    static const int RcfError_TokenRequestFailed                = 66;
    static const int RcfError_ObjectFactoryNotFound             = 67;
    static const int RcfError_PortInUse                         = 68;
    static const int RcfError_DynamicObjectNotFound             = 69;
    static const int RcfError_VersionMismatch                   = 70;
    static const int RcfError_SslCertVerification               = 72;
    static const int RcfError_FiltersLocked                     = 74;
    static const int RcfError_Pipe                              = 75;
    static const int RcfError_AnySerializerNotFound             = 76;
    static const int RcfError_ConnectionLimitExceeded           = 77;
    static const int RcfError_DeserializationNullPointer        = 78;
    static const int RcfError_PipeNameTooLong                   = 79;
    static const int RcfError_PingBack                          = 80;
    static const int RcfError_NoPingBackService                 = 81;
    static const int RcfError_NoDownload                        = 82;
    static const int RcfError_FileOffset                        = 83;
    static const int RcfError_NoUpload                          = 84;
    static const int RcfError_FileOpen                          = 85;
    static const int RcfError_FileRead                          = 86;
    static const int RcfError_FileWrite                         = 87;
    static const int RcfError_UploadFailed                      = 88;
    static const int RcfError_UploadInProgress                  = 89;
    static const int RcfError_ConcurrentUpload                  = 90;
    static const int RcfError_UploadFileSize                    = 91;
    static const int RcfError_AccessDenied                      = 92;
    static const int RcfError_PingBackTimeout                   = 93;
    static const int RcfError_AllThreadsBusy                    = 94;
    static const int RcfError_UnsupportedRuntimeVersion         = 95;
    static const int RcfError_FdSetSize                         = 97;
    static const int RcfError_DnsLookup                         = 98;
    static const int RcfError_SspiHandshakeExtraData            = 99;
    static const int RcfError_ProtobufWrite                     = 101;
    static const int RcfError_ProtobufRead                      = 102;
    static const int RcfError_ExtractSlice                      = 103;
    static const int RcfError_ServerStubExpired                 = 104;
    static const int RcfError_VariantDeserialization            = 105;
    static const int RcfError_SspiAuthFailServer                = 106;
    static const int RcfError_SspiAuthFailClient                = 107;


    static const int RcfError_User                              = 1001;

    // Errors that are no longer in use. We keep them around for backwards
    // compatibility (interacting with older clients or servers).

    static const int RcfError_StubAssignment                    = 19;
    static const int RcfError_SspiAuthFail                      = 27;
    static const int RcfError_UnknownSubscriber                 = 29;
    static const int RcfError_Filter                            = 35;
    static const int RcfError_NotConnected                      = 48;
    static const int RcfError_InvalidErrorMessage               = 54;

    template<typename T>
    std::string numberToString(T t)
    {
        std::ostringstream os;
        os << t;
        return os.str();
    }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4100)
#pragma warning(disable:4267)
#endif

    inline Error _RcfError_Ok()                             { return Error(RcfError_Ok); }
    inline Error _RcfError_Unspecified()                    { return Error(RcfError_Unspecified); }
    inline Error _RcfError_ServerMessageLength()            { return Error(RcfError_ServerMessageLength); }
    inline Error _RcfError_ClientMessageLength()            { return Error(RcfError_ClientMessageLength); }

    inline Error _RcfError_Serialization(
        const std::string & typeName, 
        const std::string & eType, 
        const std::string & eWhat)                          { return Error(RcfError_Serialization, typeName, eType, eWhat); }

    inline Error _RcfError_Deserialization(
        const std::string & typeName, 
        const std::string & eType, 
        const std::string & eWhat)                          { return Error(RcfError_Deserialization, typeName, eType, eWhat); }

    inline Error _RcfError_UserModeException(
        const std::string & eType, 
        const std::string & eWhat)                          { return Error(RcfError_UserModeException, eType, eWhat); }

    inline Error _RcfError_UnknownEndpoint()                { return Error(RcfError_UnknownEndpoint); }
    inline Error _RcfError_EndpointPassword()               { return Error(RcfError_EndpointPassword); }
    inline Error _RcfError_EndpointDown()                   { return Error(RcfError_EndpointDown); }
    inline Error _RcfError_EndpointRetry()                  { return Error(RcfError_EndpointRetry); }

    inline Error _RcfError_ClientConnectTimeout(
        unsigned int timeoutMs,
        const std::string endpoint)                         { return Error(RcfError_ClientConnectTimeout, numberToString(timeoutMs), endpoint); }

    inline Error _RcfError_PeerDisconnect()                 { return Error(RcfError_PeerDisconnect); }
    inline Error _RcfError_ClientCancel()                   { return Error(RcfError_ClientCancel); }
    inline Error _RcfError_PayloadFilterMismatch()          { return Error(RcfError_PayloadFilterMismatch); }

    inline Error _RcfError_OpenSslFilterInit(
        const std::string & opensslErrors)                  { return Error(RcfError_OpenSslFilterInit, opensslErrors); }

    inline Error _RcfError_OpenSslLoadCert(
        const std::string & file, 
        const std::string & opensslErrors)                  { return Error(RcfError_OpenSslLoadCert, file, opensslErrors); }

    inline Error _RcfError_UnknownPublisher(
        const std::string & publisherName)                  { return Error(RcfError_UnknownPublisher, publisherName); }

    inline Error _RcfError_UnknownFilter()                  { return Error(RcfError_UnknownFilter); }

    inline Error _RcfError_NoServerStub(
        const std::string & service, 
        const std::string & interface_ )                    { return Error(RcfError_NoServerStub, service, interface_); }

    inline Error _RcfError_Sspi()                           { return Error(RcfError_Sspi); }
    inline Error _RcfError_SspiInit()                       { return Error(RcfError_SspiInit); }
    inline Error _RcfError_ClientReadTimeout()              { return Error(RcfError_ClientReadTimeout); }
    inline Error _RcfError_ClientReadFail()                 { return Error(RcfError_ClientReadFail); }
    inline Error _RcfError_ClientWriteTimeout()             { return Error(RcfError_ClientWriteTimeout); }
    inline Error _RcfError_ClientWriteFail()                { return Error(RcfError_ClientWriteFail); }
    inline Error _RcfError_ClientConnectFail()              { return Error(RcfError_ClientConnectFail); }
    inline Error _RcfError_Socket()                         { return Error(RcfError_Socket); }
    inline Error _RcfError_FnId(int fnId)                   { return Error(RcfError_FnId, numberToString(fnId)); }
    
    inline Error _RcfError_UnknownInterface(
        const std::string & interface_)                     { return Error(RcfError_UnknownInterface, interface_); }

    inline Error _RcfError_NoEndpoint()                     { return Error(RcfError_NoEndpoint); }
    inline Error _RcfError_TransportCreation()              { return Error(RcfError_TransportCreation); }

    inline Error _RcfError_FilterCount(
        std::size_t count, 
        std::size_t max)                                    { return Error(RcfError_FilterCount, numberToString(count), numberToString(max)); }

    inline Error _RcfError_FilterMessage()                  { return Error(RcfError_FilterMessage); }
    inline Error _RcfError_UnfilterMessage()                { return Error(RcfError_UnfilterMessage); }
    inline Error _RcfError_SspiCredentials()                { return Error(RcfError_SspiCredentials); }
    inline Error _RcfError_SspiEncrypt()                    { return Error(RcfError_SspiEncrypt); }
    inline Error _RcfError_SspiDecrypt()                    { return Error(RcfError_SspiDecrypt); }
    inline Error _RcfError_SspiImpersonation()              { return Error(RcfError_SspiImpersonation); }
    inline Error _RcfError_SocketClose()                    { return Error(RcfError_SocketClose); }
    inline Error _RcfError_ZlibDeflate()                    { return Error(RcfError_ZlibDeflate); }
    inline Error _RcfError_ZlibInflate()                    { return Error(RcfError_ZlibInflate); }
    inline Error _RcfError_Zlib()                           { return Error(RcfError_Zlib); }

    inline Error _RcfError_UnknownSerializationProtocol(
        int protocol)                                       { return Error(RcfError_UnknownSerializationProtocol, numberToString(protocol)); }

    inline Error _SfError_NoCtor()                          { return Error(SfError_NoCtor); }
    inline Error _SfError_RefMismatch()                     { return Error(SfError_RefMismatch); }
    inline Error _SfError_DataFormat()                      { return Error(SfError_DataFormat); }
    inline Error _SfError_ReadFailure()                     { return Error(SfError_ReadFailure); }
    inline Error _SfError_WriteFailure()                    { return Error(SfError_WriteFailure); }

    inline Error _SfError_BaseDerivedRegistration(
        const std::string & baseType, 
        const std::string & derivedType)                    { return Error(SfError_BaseDerivedRegistration, baseType, derivedType); }

    inline Error _SfError_TypeRegistration(
        const std::string & typeName)                       { return Error(SfError_TypeRegistration, typeName); }

    inline Error _RcfError_NonStdException()                { return Error(RcfError_NonStdException); }

    inline Error _RcfError_SocketBind(
        const std::string & networkInterface, 
        int port)                                           { return Error(RcfError_SocketBind, networkInterface, numberToString(port)); }

    inline Error _RcfError_Decoding()                       { return Error(RcfError_Decoding); }
    inline Error _RcfError_Encoding()                       { return Error(RcfError_Encoding); }
    inline Error _RcfError_TokenRequestFailed()             { return Error(RcfError_TokenRequestFailed); }
    inline Error _RcfError_ObjectFactoryNotFound()          { return Error(RcfError_ObjectFactoryNotFound); }

    inline Error _RcfError_PortInUse(
        const std::string & networkInterface, 
        int port)                                           { return Error(RcfError_PortInUse, networkInterface, numberToString(port)); }

    inline Error _RcfError_DynamicObjectNotFound(
        int tokenId)                                        { return Error(RcfError_DynamicObjectNotFound, numberToString(tokenId)); }

    inline Error _RcfError_VersionMismatch()                { return Error(RcfError_VersionMismatch); }
    inline Error _RcfError_SslCertVerification()            { return Error(RcfError_SslCertVerification); }
    inline Error _RcfError_FiltersLocked()                  { return Error(RcfError_FiltersLocked); }
    inline Error _RcfError_Pipe()                           { return Error(RcfError_Pipe); }

    inline Error _RcfError_AnySerializerNotFound(
        const std::string & typeName)                       { return Error(RcfError_AnySerializerNotFound, typeName); }

    inline Error _RcfError_ConnectionLimitExceeded()        { return Error(RcfError_ConnectionLimitExceeded); }
    inline Error _RcfError_DeserializationNullPointer()     { return Error(RcfError_DeserializationNullPointer); }

    inline Error _RcfError_PipeNameTooLong(
        const std::string & pipeName, 
        unsigned int max)                                   { return Error(RcfError_PipeNameTooLong, pipeName, numberToString(max)); }

    inline Error _RcfError_PingBack()                       { return Error(RcfError_PingBack); }
    inline Error _RcfError_NoPingBackService()              { return Error(RcfError_NoPingBackService); }
    inline Error _RcfError_NoDownload()                     { return Error(RcfError_NoDownload); }
    inline Error _RcfError_FileOffset()                     { return Error(RcfError_FileOffset); }
    inline Error _RcfError_NoUpload()                       { return Error(RcfError_NoUpload); }
    inline Error _RcfError_FileOpen()                       { return Error(RcfError_FileOpen); }
    inline Error _RcfError_FileRead()                       { return Error(RcfError_FileRead); }
    inline Error _RcfError_FileWrite()                      { return Error(RcfError_FileWrite); }
    inline Error _RcfError_UploadFailed()                   { return Error(RcfError_UploadFailed); }
    inline Error _RcfError_UploadInProgress()               { return Error(RcfError_UploadInProgress); }
    inline Error _RcfError_ConcurrentUpload()               { return Error(RcfError_ConcurrentUpload); }
    inline Error _RcfError_UploadFileSize()                 { return Error(RcfError_UploadFileSize); }
    inline Error _RcfError_AccessDenied()                   { return Error(RcfError_AccessDenied); }

    inline Error _RcfError_PingBackTimeout(
        unsigned int pingBackIntervalMs)                    { return Error(RcfError_PingBackTimeout, numberToString(pingBackIntervalMs)); }

    inline Error _RcfError_AllThreadsBusy()                 { return Error(RcfError_AllThreadsBusy); }

    inline Error _RcfError_UnsupportedRuntimeVersion(
        int requestedVersion,
        int maxVersion)                                     { return Error(RcfError_UnsupportedRuntimeVersion, numberToString(requestedVersion), numberToString(maxVersion)); }

    inline Error _RcfError_FdSetSize(unsigned int max)      { return Error(RcfError_FdSetSize); }
    inline Error _RcfError_DnsLookup(const std::string & ip){ return Error(RcfError_DnsLookup); }
    inline Error _RcfError_SspiHandshakeExtraData()         { return Error(RcfError_SspiHandshakeExtraData); }

    inline Error _RcfError_ProtobufWrite(
        const std::string & typeName)                       { return Error(RcfError_ProtobufWrite, typeName); }

    inline Error _RcfError_ProtobufRead(
        const std::string & typeName)                       { return Error(RcfError_ProtobufRead, typeName); }

    inline Error _RcfError_ExtractSlice(
        std::size_t pos, 
        std::size_t len, 
        std::size_t max)                                    { return Error(RcfError_ExtractSlice, numberToString(pos), numberToString(len), numberToString(max)); }

    inline Error _RcfError_ServerStubExpired()              { return Error(RcfError_ServerStubExpired); }

    inline Error _RcfError_VariantDeserialization(
        int index, 
        int max)                                            { return Error(RcfError_VariantDeserialization, numberToString(index), numberToString(max)); }

    inline Error _RcfError_SspiAuthFailServer()             { return Error(RcfError_SspiAuthFailServer); }
    inline Error _RcfError_SspiAuthFailClient()             { return Error(RcfError_SspiAuthFailClient); }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    // RCF subsystem identifiers
    static const int RcfSubsystem_None                          = 0;
    static const int RcfSubsystem_Os                            = 1;
    static const int RcfSubsystem_Zlib                          = 2;
    static const int RcfSubsystem_OpenSsl                       = 3;
    static const int RcfSubsystem_Asio                          = 4;

    RCF_EXPORT std::string getErrorString(int rcfError);
    RCF_EXPORT std::string getSubSystemName(int subSystem);
    RCF_EXPORT std::string getOsErrorString(int osError);

    /// Base class of all exceptions thrown by RCF.
    class RCF_EXPORT Exception : public std::runtime_error
    {
    public:
        Exception();

        Exception(
            const std::string &     what, 
            const std::string &     context = "");

        Exception(
            Error error,
            const std::string &     what = "",
            const std::string &     context = "");

        Exception(
            Error                   error,
            int                     subSystemError,
            int                     subSystem = RcfSubsystem_Os,
            const std::string &     what = "",
            const std::string &     context = "");

        ~Exception() throw();

        virtual std::auto_ptr<Exception> clone() const
        {
            return std::auto_ptr<Exception>(
                new Exception(*this));
        }

        bool good() const;
        bool bad() const;

        const char *    what()                  const throw();
        const Error &   getError()              const;
        int             getErrorId()            const;
        std::string     getErrorString()        const;
        int             getSubSystemError()     const;
        int             getSubSystem()          const;
        std::string     getSubSystemName()      const;
        std::string     getContext()            const;
        std::string     getWhat()               const;

        void            setContext(const std::string &context);
        void            setWhat(const std::string &what);

        virtual void    throwSelf() const;

    protected:

        std::string     translate()             const;

        // protected to make serialization of RemoteException simpler
    protected:

        std::string             mWhat;
        std::string             mContext;
        Error                   mError;
        int                     mSubSystemError;
        int                     mSubSystem;

        mutable std::string     mTranslatedWhat;
    };

    typedef boost::shared_ptr<Exception> ExceptionPtr;

    class RCF_EXPORT RemoteException : public Exception
    {
    public:
        RemoteException();

        RemoteException(
            Error                   remoteError,
            const std::string &     remoteWhat = "",
            const std::string &     remoteContext = "",
            const std::string &     remoteExceptionType = "");

        RemoteException(
            Error                   remoteError,
            int                     remoteSubSystemError,
            int                     remoteSubSystem,
            const std::string &     remoteWhat = "",
            const std::string &     remoteContext = "",
            const std::string &     remoteExceptionType = "");

        ~RemoteException() throw();

        const char *what() const throw();

        std::string getRemoteExceptionType() const;

#ifdef RCF_USE_SF_SERIALIZATION

        void serialize(SF::Archive & ar);

#endif

#ifdef RCF_USE_BOOST_SERIALIZATION

        template<typename Archive>
        void serialize(Archive &ar, const unsigned int)
        {
            // TODO: update this to pass error arguments, not just id. Will
            // require use of BSer version parameter.

            int errorId = mError.getErrorId();

            ar
                & boost::serialization::make_nvp("What", mWhat)
                & boost::serialization::make_nvp("Context", mContext)
                & boost::serialization::make_nvp("Error", errorId)
                & boost::serialization::make_nvp("Subsystem Error", mSubSystemError)
                & boost::serialization::make_nvp("Subsystem", mSubSystem)
                & boost::serialization::make_nvp("Remote Exception Type", mRemoteExceptionType);

            bool isLoading = boost::is_base_and_derived<
                boost::archive::detail::basic_iarchive, Archive>::value;

            if (isLoading)
            {
                mError.setErrorId(errorId);
            }
        }

#endif

        std::auto_ptr<Exception> clone() const
        {
            return std::auto_ptr<Exception>(
                new RemoteException(*this));
        }

        void throwSelf() const;

    private:
        std::string mRemoteExceptionType;
    };

#define RCF_DEFINE_EXCEPTION(E, PE)                             \
    class E : public PE                                         \
    {                                                           \
    public:                                                     \
        E(                                                      \
            const std::string &what = "") :                     \
                PE(_RcfError_Unspecified(), what)               \
        {}                                                      \
        E(                                                      \
            Error error,                                        \
            const std::string &what = "") :                     \
                PE(error, what)                                 \
        {}                                                      \
        E(                                                      \
            Error error,                                        \
            int subSystemError,                                 \
            int subSystem,                                      \
            const std::string &what = "") :                     \
                PE(error, subSystemError, subSystem, what)      \
        {}                                                      \
        std::auto_ptr<Exception> clone() const                  \
        {                                                       \
            return std::auto_ptr<Exception>(                    \
                new E(*this));                                  \
        }                                                       \
        void throwSelf() const                                  \
        {                                                       \
            RCF_THROW((*this));                                 \
        }                                                       \
        ~E() throw()                                            \
        {}                                                      \
    };

    RCF_DEFINE_EXCEPTION(SerializationException,        Exception)
    RCF_DEFINE_EXCEPTION(AssertionFailureException,     Exception)
    RCF_DEFINE_EXCEPTION(FilterException,               Exception)

    class RCF_EXPORT VersioningException : public RemoteException
    {
    public:
        VersioningException(int version);
        ~VersioningException() throw();
        int getVersion() const;

        std::auto_ptr<Exception> clone() const
        {
            return std::auto_ptr<Exception>(
                new VersioningException(*this));
        }

        void throwSelf() const
        {
            RCF_THROW((*this));
        }

    private:
        int mVersion;
    };

#undef RCF_DEFINE_EXCEPTION

} // namespace RCF

#include <memory>
#include <boost/type_traits.hpp>
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION( RCF::Error );
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION( std::vector<std::string> );
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION( RCF::RemoteException )
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION( std::auto_ptr<RCF::RemoteException> )

#endif // ! INCLUDE_RCF_EXCEPTION_HPP
