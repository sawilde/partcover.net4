
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_SSPIFILTER_HPP
#define INCLUDE_RCF_SSPIFILTER_HPP

#include <memory>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/AsyncFilter.hpp>
#include <RCF/RecursionLimiter.hpp>
#include <RCF/Export.hpp>
#include <RCF/RecursionLimiter.hpp>
#include <RCF/Tools.hpp>

#include <RCF/util/Tchar.hpp>

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif

#include <security.h>
#include <WinCrypt.h>
#include <tchar.h>

#if defined(_MSC_VER) && _MSC_VER == 1200

#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0400
#error Schannel support not available in Visual C++ 6 platform headers, when _WIN32_WINNT < 0x0400.
#endif

#endif

namespace RCF {

    static const bool BoolClient = false;
    static const bool BoolServer = true;

    static const bool BoolSchannel = true;

    typedef util::tstring tstring;

    RCF_EXPORT tstring getMyUserName();
    RCF_EXPORT tstring getMyDomain();
    RCF_EXPORT tstring getMyMachineName();

    class SspiFilter;

    typedef boost::shared_ptr<SspiFilter> SspiFilterPtr;

    class RCF_EXPORT SspiImpersonator
    {
    public:
        SspiImpersonator(SspiFilterPtr sspiFilterPtr);
        ~SspiImpersonator();

        bool impersonate();
        void revertToSelf() const;
    private:
        SspiFilterPtr mSspiFilterPtr;
    };

    static const ULONG DefaultSspiContextRequirements =
        ISC_REQ_REPLAY_DETECT   |
        ISC_REQ_SEQUENCE_DETECT |
        ISC_REQ_CONFIDENTIALITY |
        ISC_REQ_INTEGRITY       |
        ISC_REQ_DELEGATE        |
        ISC_REQ_MUTUAL_AUTH;

    class SchannelClientFilter;
    typedef SchannelClientFilter SchannelFilter;

    class RCF_EXPORT SspiFilter : public Filter
    {
    public:

        ~SspiFilter();

        enum QualityOfProtection
        {
            None,
            Encryption,
            Integrity
        };

        QualityOfProtection getQop();

        typedef SspiImpersonator Impersonator;

        typedef boost::function1<void, SchannelFilter &> CertValidationCallback;

    protected:

        friend class SspiImpersonator;

        SspiFilter(
            const tstring &         packageName,
            const tstring &         packageList,
            bool                    server,
            bool                    schannel);

        SspiFilter(
            const tstring &         target,
            QualityOfProtection     qop,
            ULONG                   contextRequirements,
            const tstring &         packageName,
            const tstring &         packageList,
            bool                    server,
            bool                    schannel);

        SspiFilter(
            const tstring &         userName,
            const tstring &         password,
            const tstring &         domain,
            const tstring &         target,
            QualityOfProtection     qop,
            ULONG                   contextRequirements,
            const tstring &         packageName,
            const tstring &         packageList,
            bool                    server);

        enum Event
        {
            ReadIssued,
            WriteIssued,
            ReadCompleted,
            WriteCompleted
        };

        enum ContextState
        {
            AuthContinue,
            AuthOk,
            AuthOkAck,
            AuthFailed
        };

        enum State
        {
            Ready,
            Reading,
            Writing
        };

        void            setupCredentials(
                            const tstring &userName,
                            const tstring &password,
                            const tstring &domain);

        void            setupCredentialsSchannel();

        void            acquireCredentials(
                            const tstring &userName = RCF_T(""),
                            const tstring &password = RCF_T(""),
                            const tstring &domain = RCF_T(""));

        void            freeCredentials();

        void            init();
        void            deinit();
        void            reset();

        void            read(
                            const ByteBuffer &byteBuffer, 
                            std::size_t bytesRequested);

        void            write(const std::vector<ByteBuffer> &byteBuffers);

        void            onReadCompleted(const ByteBuffer &byteBuffer);
        void            onWriteCompleted(std::size_t bytesTransferred);

        void            handleEvent(Event event);
        void            readBuffer();
        void            writeBuffer();
        
        void            encryptWriteBuffer();
        bool            decryptReadBuffer();

        void            encryptWriteBufferSchannel();
        bool            decryptReadBufferSchannel();
        
        bool            completeReadBlock();
        bool            completeWriteBlock();
        bool            completeBlock();
        void            resumeUserIo();
        void            resizeReadBuffer(std::size_t newSize);
        void            resizeWriteBuffer(std::size_t newSize);

        void            shiftReadBuffer();
        void            trimReadBuffer();

        virtual void    handleHandshakeEvent() = 0;

    protected:

        const tstring                           mPackageName;
        const tstring                           mPackageList;
        tstring                                 mTarget;
        QualityOfProtection                     mQop;
        ULONG                                   mContextRequirements;

        bool                                    mHaveContext;
        bool                                    mHaveCredentials;
        bool                                    mImplicitCredentials;
        SecPkgInfo                              mPkgInfo;
        CtxtHandle                              mContext;
        CredHandle                              mCredentials;

        ContextState                            mContextState;
        State                                   mPreState;
        State                                   mPostState;
        Event                                   mEvent;
        const bool                              mServer;

        ByteBuffer                              mReadByteBufferOrig;
        ByteBuffer                              mWriteByteBufferOrig;
        std::size_t                             mBytesRequestedOrig;

        ByteBuffer                              mReadByteBuffer;
        boost::shared_ptr<std::vector<char> >   mReadBufferVectorPtr;
        char *                                  mReadBuffer;
        std::size_t                             mReadBufferPos;
        std::size_t                             mReadBufferLen;

        ByteBuffer                              mWriteByteBuffer;
        boost::shared_ptr<std::vector<char> >   mWriteBufferVectorPtr;
        char *                                  mWriteBuffer;
        std::size_t                             mWriteBufferPos;
        std::size_t                             mWriteBufferLen;

        std::vector<ByteBuffer>                 mByteBuffers;
        ByteBuffer                              mTempByteBuffer;

        const bool                              mSchannel;

        std::size_t                             mMaxMessageLength;
 
        // Schannel-specific members.
        boost::shared_ptr<const CERT_CONTEXT>   mLocalCert;
        boost::shared_ptr<const CERT_CONTEXT>   mRemoteCert;
        CertValidationCallback                  mCertValidationCallback;
        DWORD                                   mEnabledProtocols;
        const std::size_t                       mReadAheadChunkSize;
        std::size_t                             mRemainingDataPos;

    private:
        bool                                    mLimitRecursion;
        RecursionState<ByteBuffer, int>         mRecursionStateRead;
        RecursionState<std::size_t, int>        mRecursionStateWrite;

        void onReadCompleted_(const ByteBuffer &byteBuffer);
        void onWriteCompleted_(std::size_t bytesTransferred);
    };

    // server filters

    class RCF_EXPORT SspiServerFilter : public SspiFilter
    {
    public:
        SspiServerFilter(
            const tstring &packageName, 
            const tstring &packageList,
            bool schannel = false);

    private:
        bool doHandshakeSchannel();
        bool doHandshake();
        void handleHandshakeEvent();
    };

    class RCF_EXPORT NtlmServerFilter : public SspiServerFilter
    {
    public:
        NtlmServerFilter();
        const FilterDescription & getFilterDescription() const;
    };

    class RCF_EXPORT KerberosServerFilter : public SspiServerFilter
    {
    public:
        KerberosServerFilter();
        const FilterDescription & getFilterDescription() const;
    };

    class RCF_EXPORT NegotiateServerFilter : public SspiServerFilter
    {
    public:
        NegotiateServerFilter(const tstring &packageList);
        const FilterDescription & getFilterDescription() const;
    };

    // filter factories

    class RCF_EXPORT NtlmFilterFactory : public FilterFactory
    {
    public:
        FilterPtr createFilter();
        const FilterDescription & getFilterDescription();
    };

    class RCF_EXPORT KerberosFilterFactory : public FilterFactory
    {
    public:
        FilterPtr createFilter();
        const FilterDescription & getFilterDescription();
    };

    class RCF_EXPORT NegotiateFilterFactory : public FilterFactory
    {
    public:
        NegotiateFilterFactory(const tstring &packageList = RCF_T("Kerberos, NTLM"));
        FilterPtr createFilter();
        const FilterDescription & getFilterDescription();
    private:
        tstring mPackageList;
    };

    // client filters

    class RCF_EXPORT SspiClientFilter : public SspiFilter
    {
    public:
        SspiClientFilter(
            const tstring &         userName,
            const tstring &         password,
            const tstring &         domain,
            const tstring &         targetName,
            QualityOfProtection     qop,
            ULONG                   contextRequirements,
            const tstring &         packageName,
            const tstring &         packageList) :
                SspiFilter(
                    userName, 
                    password, 
                    domain,
                    targetName, 
                    qop, 
                    contextRequirements, 
                    packageName, 
                    packageList,
                    BoolClient)
        {}

        SspiClientFilter(
            const tstring &         targetName,
            QualityOfProtection     qop,
            ULONG                   contextRequirements,
            const tstring &         packageName,
            const tstring &         packageList,
            bool                    schannel) :
                SspiFilter(
                    targetName, 
                    qop, 
                    contextRequirements, 
                    packageName, 
                    packageList,
                    BoolClient,
                    schannel)
        {}

    private:
        bool doHandshakeSchannel();
        bool doHandshake();
        void handleHandshakeEvent();
    };

    class RCF_EXPORT NtlmClientFilter : public SspiClientFilter
    {
    public:
        NtlmClientFilter(
            const tstring &         userName,
            const tstring &         password,
            const tstring &         domain,
            QualityOfProtection     qop = SspiFilter::Encryption,
            ULONG                   contextRequirements 
                                        = DefaultSspiContextRequirements);

        NtlmClientFilter(
            QualityOfProtection     qop = SspiFilter::Encryption,
            ULONG                   contextRequirements 
                                        = DefaultSspiContextRequirements);

        const FilterDescription &           getFilterDescription() const;
    };

    class RCF_EXPORT KerberosClientFilter : public SspiClientFilter
    {
    public:
        KerberosClientFilter(
            const tstring &         userName,
            const tstring &         password,
            const tstring &         domain,
            const tstring &         targetName,
            QualityOfProtection     qop = SspiFilter::Encryption,
            ULONG                   contextRequirements 
                                        = DefaultSspiContextRequirements);

        KerberosClientFilter(
            const tstring &         targetName,
            QualityOfProtection     qop = SspiFilter::Encryption,
            ULONG                   contextRequirements 
                                        = DefaultSspiContextRequirements);

        const FilterDescription &           getFilterDescription() const;
    };

    class RCF_EXPORT NegotiateClientFilter : public SspiClientFilter
    {
    public:
        NegotiateClientFilter(
            const tstring &         userName,
            const tstring &         password,
            const tstring &         domain,
            const tstring &         targetName,
            QualityOfProtection     qop = SspiFilter::None,
            ULONG                   contextRequirements 
                                        = DefaultSspiContextRequirements);

        NegotiateClientFilter(
            const tstring &         targetName,
            QualityOfProtection     qop = SspiFilter::Encryption,
            ULONG                   contextRequirements 
                                        = DefaultSspiContextRequirements);

        const FilterDescription &           getFilterDescription() const;
    };

    typedef NtlmClientFilter            NtlmFilter;
    typedef KerberosClientFilter        KerberosFilter;
    typedef NegotiateClientFilter       NegotiateFilter;
    

    // These SSPI-prefixed typedefs make us compatible with code written for RCF 1.0.
    typedef NtlmFilter                  SspiNtlmFilter;
    typedef KerberosFilter              SspiKerberosFilter;
    typedef NegotiateFilter             SspiNegotiateFilter;

    typedef NtlmServerFilter            SspiNtlmServerFilter;
    typedef KerberosServerFilter        SspiKerberosServerFilter;
    typedef NegotiateServerFilter       SspiNegotiateServerFilter;
    typedef NtlmFilterFactory           SspiNtlmFilterFactory;
    typedef KerberosFilterFactory       SspiKerberosFilterFactory;
    typedef NegotiateFilterFactory      SspiNegotiateFilterFactory;
    typedef NtlmClientFilter            SspiNtlmClientFilter;
    typedef KerberosClientFilter        SspiKerberosClientFilter;
    typedef NegotiateClientFilter       SspiNegotiateClientFilter;

    typedef SspiFilter                  SspiFilterBase;
    typedef SspiFilterPtr               SspiFilterBasePtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_SSPIFILTER_HPP
