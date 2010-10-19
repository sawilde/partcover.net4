
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/SspiFilter.hpp>

#include <boost/multi_index/detail/scope_guard.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/CurrentSession.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/ObjectPool.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/Schannel.hpp>
#include <RCF/ServerTransport.hpp>
#include <RCF/Tools.hpp>

#include <tchar.h>

#ifdef _UNICODE

#define INIT_SEC_INTERFACE_NAME       "InitSecurityInterfaceW"
typedef unsigned short UTCHAR;

#else

#define INIT_SEC_INTERFACE_NAME       "InitSecurityInterfaceA"
typedef unsigned char UTCHAR;

#endif

// spelling mistake in mingw headers!
#if defined(__MINGW32__) && __GNUC__ == 3 && __GNUC_MINOR__ <= 2
#define cbMaxSignature cbMaxSIgnature
#endif

// missing stuff in mingw headers
#ifdef __MINGW32__
#ifndef SEC_WINNT_AUTH_IDENTITY_VERSION
#define SEC_WINNT_AUTH_IDENTITY_VERSION 0x200

#ifndef SEC_WINNT_AUTH_IDENTITY_ANSI
#define SEC_WINNT_AUTH_IDENTITY_ANSI    0x1
#endif

#ifndef SEC_WINNT_AUTH_IDENTITY_UNICODE
#define SEC_WINNT_AUTH_IDENTITY_UNICODE 0x2
#endif

typedef struct _SEC_WINNT_AUTH_IDENTITY_EXW {
    unsigned long Version;
    unsigned long Length;
    unsigned short SEC_FAR *User;
    unsigned long UserLength;
    unsigned short SEC_FAR *Domain;
    unsigned long DomainLength;
    unsigned short SEC_FAR *Password;
    unsigned long PasswordLength;
    unsigned long Flags;
    unsigned short SEC_FAR * PackageList;
    unsigned long PackageListLength;
} SEC_WINNT_AUTH_IDENTITY_EXW, *PSEC_WINNT_AUTH_IDENTITY_EXW;

// end_ntifs

typedef struct _SEC_WINNT_AUTH_IDENTITY_EXA {
    unsigned long Version;
    unsigned long Length;
    unsigned char SEC_FAR *User;
    unsigned long UserLength;
    unsigned char SEC_FAR *Domain;
    unsigned long DomainLength;
    unsigned char SEC_FAR *Password;
    unsigned long PasswordLength;
    unsigned long Flags;
    unsigned char SEC_FAR * PackageList;
    unsigned long PackageListLength;
} SEC_WINNT_AUTH_IDENTITY_EXA, *PSEC_WINNT_AUTH_IDENTITY_EXA;

#ifdef UNICODE
#define SEC_WINNT_AUTH_IDENTITY_EX  SEC_WINNT_AUTH_IDENTITY_EXW    // ntifs
#define PSEC_WINNT_AUTH_IDENTITY_EX PSEC_WINNT_AUTH_IDENTITY_EXW   // ntifs
#else
#define SEC_WINNT_AUTH_IDENTITY_EX  SEC_WINNT_AUTH_IDENTITY_EXA
#endif

// begin_ntifs
#endif // SEC_WINNT_AUTH_IDENTITY_VERSION      

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum
    {
        NameUnknown = 0,
        NameFullyQualifiedDN = 1,
        NameSamCompatible = 2,
        NameDisplay = 3,
        NameUniqueId = 6,
        NameCanonical = 7,
        NameUserPrincipal = 8,
        NameCanonicalEx = 9,
        NameServicePrincipal = 10,
        NameDnsDomain = 12
    } EXTENDED_NAME_FORMAT, * PEXTENDED_NAME_FORMAT ;

#ifdef __cplusplus
}
#endif

#define SP_PROT_NONE                    0

#endif // __MINGW__

#if defined(_MSC_VER) && _MSC_VER == 1200
#include "rpc.h" // SEC_WINNT_AUTH_IDENTITY
#endif

#include <sspi.h>

namespace RCF {

    PSecurityFunctionTable getSft();

#ifdef UNICODE
    LPCSTR GetUserNameExName = "GetUserNameExW";
#else
    LPCSTR GetUserNameExName = "GetUserNameExA";
#endif

    typedef BOOLEAN (WINAPI *PfnGetUserNameEx)(EXTENDED_NAME_FORMAT, LPTSTR, PULONG);
    HMODULE hModuleSecur32 = 0;
    PfnGetUserNameEx pfnGetUserNameEx = NULL;
   

    tstring getMyUserName()
    {
        std::vector<TCHAR> vec;
        DWORD len = 0;
        BOOL ret = GetUserName(NULL, &len);
        BOOL err = 0;
        vec.resize(len);
        ret = GetUserName(&vec[0], &len);
        err = GetLastError();
        RCF_VERIFY(
            ret,
            Exception(
                _RcfError_Sspi(),
                err,
                RcfSubsystem_Os,
                "GetUserName() failed"));
        return tstring(&vec[0]);
    }

    tstring getMyDomain()
    {
        if (pfnGetUserNameEx)
        {
            ULONG count = 0;
            pfnGetUserNameEx(NameSamCompatible, NULL, &count);
            std::vector<TCHAR> vec(count);
            BOOLEAN ok = pfnGetUserNameEx(NameSamCompatible, &vec[0], &count);
            DWORD dwErr = GetLastError();

            RCF_VERIFY(
                ok,
                Exception(
                _RcfError_SspiCredentials(),
                dwErr,
                RcfSubsystem_Os,
                "GetUserNameEx() failed"))(dwErr);

            tstring domainAndUser(&vec[0]);
            tstring domain = domainAndUser.substr(
                0,
                domainAndUser.find('\\'));
            return domain;
        }
        else
        {
            // GetUserNameEx() is not available on older Windows versions, so
            // here's the alternative.

            // This code may fail if we are impersonating another user, and our
            // Windows privileges aren't appropriately enabled. OpenThreadToken()
            // fails with "Access denied".

            using namespace boost::multi_index::detail;

            // obtain current token
            HANDLE hToken;
            BOOL ok = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken);
            DWORD dwErr1 = GetLastError();
            DWORD dwErr2 = 0;
            if (!ok)
            {
                ok = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
                dwErr2 = GetLastError();
            }

            RCF_VERIFY(
                ok,
                Exception(
                    _RcfError_SspiCredentials(),
                    dwErr2,
                    RcfSubsystem_Os,
                    "OpenProcessToken() failed"))(dwErr1)(dwErr2);

            scope_guard guard = make_guard(&CloseHandle, hToken);
            RCF_UNUSED_VARIABLE(guard);

            PTOKEN_USER ptiUser     = NULL;
            DWORD       cbti        = 0;

            // find the length of the token information buffer
            GetTokenInformation(hToken, TokenUser, NULL, 0, &cbti);

            // allocate buffer for token information
            std::vector<char> vec(cbti);
            ptiUser = (PTOKEN_USER) &vec[0];

            // obtain token information
            GetTokenInformation(hToken, TokenUser, ptiUser, cbti, &cbti);

            // extract domain and username
            TCHAR    szDomain[256];           
            DWORD    szDomainLen = sizeof(szDomain)/sizeof(szDomain[0]);

            TCHAR    szUsername[256];           
            DWORD    szUsernameLen = sizeof(szUsername)/sizeof(szUsername[0]);

            SID_NAME_USE snu;

            ok = LookupAccountSid(
                NULL, ptiUser->User.Sid,
                szUsername, &szUsernameLen,
                szDomain, &szDomainLen,
                &snu);
            DWORD err = GetLastError();
            RCF_VERIFY(
                ok,
                Exception(
                    _RcfError_SspiCredentials(),
                    err,
                    RcfSubsystem_Os,
                    "LookupAccountSid() failed"));

            return szDomain;
        }
    }

    tstring getMyMachineName()
    {
        const int BufferSize = MAX_COMPUTERNAME_LENGTH + 1;
        TCHAR buffer[BufferSize];
        DWORD dwSize = sizeof(buffer)/sizeof(buffer[0]);
        BOOL ok = GetComputerName(buffer, &dwSize);
        RCF_ASSERT(ok);
        return tstring(&buffer[0]);
    }

    SspiFilter::SspiFilter(
        const tstring &packageName,
        const tstring &packageList,
        bool server,
        bool schannel) :
            mPackageName(packageName),
            mPackageList(packageList),
            mQop(None),
            mContextRequirements(RCF_DEFAULT_INIT),
            mServer(server),
            mPreState(Ready),
            mBytesRequestedOrig(RCF_DEFAULT_INIT),
            mWriteBuffer(RCF_DEFAULT_INIT),
            mWriteBufferPos(RCF_DEFAULT_INIT),
            mWriteBufferLen(RCF_DEFAULT_INIT),
            mReadBuffer(RCF_DEFAULT_INIT),
            mReadBufferPos(RCF_DEFAULT_INIT),
            mReadBufferLen(RCF_DEFAULT_INIT),
            mPostState(Ready),
            mHaveContext(RCF_DEFAULT_INIT),
            mHaveCredentials(RCF_DEFAULT_INIT),
            mImplicitCredentials(true),
            mContext(),
            mCredentials(),
            mTarget(),
            mContextState(AuthContinue),
            mEvent(ReadIssued),
            mLimitRecursion(!server),
            mSchannel(schannel),
            mMaxMessageLength(RCF_DEFAULT_INIT),
            mReadAheadChunkSize(schannel ? 0x10000 : 4),
            mRemainingDataPos(RCF_DEFAULT_INIT)
    {

#if defined(_MSC_VER) && _MSC_VER < 1310
        memset(&mContext, 0, sizeof(mContext));
        memset(&mCredentials, 0, sizeof(mCredentials));
#endif

        mPkgInfo.Name = NULL;
        mPkgInfo.Comment = NULL;

        init();
    }

    SspiFilter::SspiFilter(
        const tstring &target,
        QualityOfProtection qop,
        ULONG contextRequirements,
        const tstring &packageName,
        const tstring &packageList,
        bool server,
        bool schannel) :
            mPackageName(packageName),
            mPackageList(packageList),
            mQop(qop),
            mContextRequirements(contextRequirements),
            mServer(server),
            mPreState(Ready),
            mBytesRequestedOrig(RCF_DEFAULT_INIT),
            mWriteBuffer(RCF_DEFAULT_INIT),
            mWriteBufferPos(RCF_DEFAULT_INIT),
            mWriteBufferLen(RCF_DEFAULT_INIT),
            mReadBuffer(RCF_DEFAULT_INIT),
            mReadBufferPos(RCF_DEFAULT_INIT),
            mReadBufferLen(RCF_DEFAULT_INIT),
            mPostState(Ready),
            mHaveContext(RCF_DEFAULT_INIT),
            mHaveCredentials(RCF_DEFAULT_INIT),
            mImplicitCredentials(true),
            mContext(),
            mCredentials(),
            mTarget(target),
            mContextState(AuthContinue),
            mEvent(ReadIssued),
            mLimitRecursion(!server),
            mSchannel(schannel),
            mMaxMessageLength(RCF_DEFAULT_INIT),
            mReadAheadChunkSize(schannel ? 0x10000 : 4),
            mRemainingDataPos(RCF_DEFAULT_INIT)
    {

#if defined(_MSC_VER) && _MSC_VER < 1310
        memset(&mContext, 0, sizeof(mContext));
        memset(&mCredentials, 0, sizeof(mCredentials));
#endif

        mPkgInfo.Name = NULL;
        mPkgInfo.Comment = NULL;

        init();
    }

    // client mode ctor, accessible to the public
    SspiFilter::SspiFilter(
        const tstring &userName,
        const tstring &password,
        const tstring &domain,
        const tstring &target,
        QualityOfProtection qop,
        ULONG contextRequirements,
        const tstring &packageName,
        const tstring &packageList,
        bool server) :
            mPackageName(packageName),
            mPackageList(packageList),
            mQop(qop),
            mContextRequirements(contextRequirements),
            mServer(server),
            mPreState(Ready),
            mBytesRequestedOrig(RCF_DEFAULT_INIT),
            mWriteBuffer(RCF_DEFAULT_INIT),
            mWriteBufferPos(RCF_DEFAULT_INIT),
            mWriteBufferLen(RCF_DEFAULT_INIT),
            mReadBuffer(RCF_DEFAULT_INIT),
            mReadBufferPos(RCF_DEFAULT_INIT),
            mReadBufferLen(RCF_DEFAULT_INIT),
            mPostState(Ready),
            mHaveContext(RCF_DEFAULT_INIT),
            mHaveCredentials(RCF_DEFAULT_INIT),
            mImplicitCredentials(RCF_DEFAULT_INIT),
            mContext(),
            mCredentials(),
            mTarget(target),
            mContextState(AuthContinue),
            mEvent(ReadIssued),
            mLimitRecursion(!server),
            mSchannel(false),
            mMaxMessageLength(RCF_DEFAULT_INIT),
            mReadAheadChunkSize(4),
            mRemainingDataPos(RCF_DEFAULT_INIT)
    {

#if defined(_MSC_VER) && _MSC_VER < 1310
        memset(&mContext, 0, sizeof(mContext));
        memset(&mCredentials, 0, sizeof(mCredentials));
#endif

        mPkgInfo.Name = NULL;
        mPkgInfo.Comment = NULL;

        acquireCredentials(userName, password, domain);
        init();
    }

    SspiFilter::~SspiFilter()
    {
        RCF_DTOR_BEGIN
            deinit();
            freeCredentials();
        RCF_DTOR_END
    }

    SspiFilter::QualityOfProtection SspiFilter::getQop()
    {
        return mQop;
    }

#if defined(_MSC_VER) && _MSC_VER == 1200
#define FreeCredentialsHandle FreeCredentialHandle
#endif

    void SspiFilter::freeCredentials()
    {
        if (mHaveCredentials)
        {
            SECURITY_STATUS status = 0;
            status = getSft()->FreeCredentialsHandle(&mCredentials);
            RCF_VERIFY(
                status == SEC_E_OK || status == SEC_E_INVALID_HANDLE,
                FilterException(
                    _RcfError_Sspi(),
                    status,
                    RcfSubsystem_Os,
                    "FreeCredentialsHandle() failed"));
        }

        if (mPkgInfo.Name)
        {
            delete [] mPkgInfo.Name;
        }

        if (mPkgInfo.Comment)
        {
            delete [] mPkgInfo.Comment;
        }

    }

#if defined(_MSC_VER) && _MSC_VER == 1200
#undef FreeCredentialsHandle
#endif

    void SspiFilter::reset()
    {
        init();
    }

    void SspiFilter::deinit()
    {
        if (mHaveContext)
        {
            SECURITY_STATUS status = 0;       
            status = getSft()->DeleteSecurityContext(&mContext);
            RCF_VERIFY(
                status == SEC_E_OK || status == SEC_E_INVALID_HANDLE,
                FilterException(
                    _RcfError_Sspi(),
                    status,
                    RcfSubsystem_Os,
                    "DeleteSecurityContext() failed"));
            mHaveContext = false;
        }

        if (mReadBufferVectorPtr.get() && mReadBufferVectorPtr.unique())
        {
            getObjectPool().put(mReadBufferVectorPtr);
        }

        if (mWriteBufferVectorPtr.get() && mWriteBufferVectorPtr.unique())
        {
            getObjectPool().put(mWriteBufferVectorPtr);
        }
    }

    void SspiFilter::init()
    {
        deinit();

        mPreState = Ready;
        mPostState = Ready;
        mContextState = AuthContinue;
        mEvent = ReadIssued;

        resizeReadBuffer(0);
        resizeWriteBuffer(0);

    }

    void SspiFilter::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        // Client-side - cache the current max message length setting.
        if (!mServer)
        {
            ClientStubPtr clientStubPtr = getCurrentClientStubPtr();
            if (clientStubPtr)
            {
                I_ClientTransport & clientTransport = clientStubPtr->getTransport();
                mMaxMessageLength = clientTransport.getMaxMessageLength();
            }
        }

        if (byteBuffer.isEmpty() && bytesRequested == 0)
        {
            // If we have some data, issue a read completion, otherwise return
            // our buffers to the pool and issue a zero byte read on the next
            // filter.

            if (mReadBufferPos < mReadBufferLen)
            {
                mpPreFilter->onReadCompleted(byteBuffer);
            }
            else if (mReadBufferPos == mReadBufferLen && mRemainingDataPos > 0)
            {
                mpPreFilter->onReadCompleted(byteBuffer);
            }
            else
            {
                RCF_ASSERT(mReadBufferPos == mReadBufferLen);

                if (mReadBufferVectorPtr)
                {
                    mReadBufferVectorPtr->resize(0);
                }
                if (mWriteBufferVectorPtr)
                {
                    mWriteBufferVectorPtr->resize(0);
                }
                
                mTempByteBuffer.clear();
                mReadByteBuffer.clear();
                mWriteByteBuffer.clear();

                if (mReadBufferVectorPtr)
                {
                    getObjectPool().put(mReadBufferVectorPtr);
                }
                if (mWriteBufferVectorPtr)
                {
                    getObjectPool().put(mWriteBufferVectorPtr);
                }

                // Forward the zero-byte read to the next filter.
                mReadByteBufferOrig = byteBuffer;
                mBytesRequestedOrig = bytesRequested;
                mPreState = Reading;
                mpPostFilter->read(ByteBuffer(), 0);
            }
        }
        else
        {
            mReadByteBufferOrig = byteBuffer;
            mBytesRequestedOrig = bytesRequested;
            mPreState = Reading;
            handleEvent(ReadIssued);
        }
    }

    void SspiFilter::write(const std::vector<ByteBuffer> &byteBuffers)
    {
        // Client-side - cache the current max message length setting.
        if (!mServer)
        {
            ClientStubPtr clientStubPtr = getCurrentClientStubPtr();
            if (clientStubPtr)
            {
                I_ClientTransport & clientTransport = clientStubPtr->getTransport();
                mMaxMessageLength = clientTransport.getMaxMessageLength();
            }
        }

        // TODO: write many buffers in one go
        mWriteByteBufferOrig = byteBuffers.front();
        mPreState = Writing;
        handleEvent(WriteIssued);
    }

    void SspiFilter::onReadCompleted_(
        const ByteBuffer &byteBuffer)
    {
        if (mPreState == Reading && mBytesRequestedOrig == 0)
        {
            RCF_ASSERT(byteBuffer.isEmpty());
            mpPreFilter->onReadCompleted(ByteBuffer());
        }
        else
        {
            RCF_ASSERT(
                mReadBuffer + mReadBufferPos == byteBuffer.getPtr())
                (mReadBuffer)(mReadBufferPos)(byteBuffer.getPtr());

            mReadBufferPos += byteBuffer.getLength();

            RCF_ASSERT(
                mReadBufferPos <= mReadBufferLen)
                (mReadBufferPos)(mReadBufferLen);

            const_cast<ByteBuffer &>(byteBuffer).clear();
            handleEvent(ReadCompleted);
        }
    }

    // Recursion limiter can only be used on synchronous filter stacks, and
    // avoids excessive recursion when reading or writing data in small pieces.
    // On asynchronous filter stacks, it would introduce a race condition by setting
    // filter state _after_ invoking downstream async read/write operations.
    void SspiFilter::onReadCompleted(
        const ByteBuffer &byteBuffer)
    {
        if (mLimitRecursion)
        {
            applyRecursionLimiter(
                mRecursionStateRead,
                &SspiFilter::onReadCompleted_,
                *this,
                byteBuffer);
        }
        else
        {
            onReadCompleted_(byteBuffer);
        }
    }

    void SspiFilter::onWriteCompleted_(
        std::size_t bytesTransferred)
    {
        mByteBuffers.resize(0);
        mWriteBufferPos += bytesTransferred;
       
        RCF_ASSERT(
            mWriteBufferPos <= mWriteBufferLen)
            (mWriteBufferPos)(mWriteBufferLen);

        handleEvent(WriteCompleted);
    }

    void SspiFilter::onWriteCompleted(
        std::size_t bytesTransferred)
    {
        if (mLimitRecursion)
        {
            applyRecursionLimiter(
                mRecursionStateWrite,
                &SspiFilter::onWriteCompleted_,
                *this,
                bytesTransferred);
        }
        else
        {
            onWriteCompleted_(bytesTransferred);
        }
    }

    void SspiFilter::handleEvent(Event event)
    {
        RCF_ASSERT(
            event == ReadIssued || event == WriteIssued ||
            event == ReadCompleted || event == WriteCompleted)(event);

        mTempByteBuffer.clear();

        mEvent = event;
        if ((mEvent == ReadIssued || mEvent == WriteIssued) || completeBlock())
        {
            if (mContextState != AuthOkAck)
            {
                handleHandshakeEvent();
            }
            else
            {
                switch (mEvent)
                {
                case ReadIssued:

                    if (0 < mReadBufferPos && mReadBufferPos < mReadBufferLen)
                    {
                        // read from currently decrypted  block
                        std::size_t bytesAvail = mReadBufferLen - mReadBufferPos;

                        std::size_t bytesToRead =
                            RCF_MIN(bytesAvail, mBytesRequestedOrig);

                        if (mReadByteBufferOrig.getLength() > 0)
                        {
                            memcpy(
                                mReadByteBufferOrig.getPtr(),
                                mReadBuffer+mReadBufferPos,
                                bytesToRead);

                            mTempByteBuffer = ByteBuffer(
                                mReadByteBufferOrig,
                                0,
                                bytesToRead);
                        }
                        else
                        {
                            mTempByteBuffer = ByteBuffer(
                                mReadByteBuffer,
                                mReadBufferPos,
                                bytesToRead);
                        }
                       
                        mReadBufferPos += bytesToRead;
                        mReadByteBufferOrig = ByteBuffer();

                        mpPreFilter->onReadCompleted(mTempByteBuffer);
                    }
                    else if (mRemainingDataPos)
                    {
                        RCF_ASSERT(mSchannel);
                        shiftReadBuffer();
                        handleEvent(ReadCompleted);
                    }
                    else
                    {
                        // read in a new block
                        resizeReadBuffer(mReadAheadChunkSize);
                        readBuffer();
                    }
                    break;

                case WriteIssued:

                    mSchannel ?
                        encryptWriteBufferSchannel() :
                        encryptWriteBuffer();

                    writeBuffer();
                    break;

                case ReadCompleted:

                    {
                        bool ok = mSchannel ?
                            decryptReadBufferSchannel() :
                            decryptReadBuffer();

                        if (ok)
                        {
                            handleEvent(ReadIssued);
                        }
                    }
                    
                    break;

                case WriteCompleted:

                    {
                        std::size_t bytesTransferred =
                            mWriteByteBufferOrig.getLength();

                        mWriteByteBufferOrig = ByteBuffer();
                        mpPreFilter->onWriteCompleted(bytesTransferred);
                    }
                   
                    break;

                default:
                    RCF_ASSERT(0);
                }
            }
        }
    }

    void SspiFilter::readBuffer()
    {
        RCF_ASSERT(
            0 <= mReadBufferPos && mReadBufferPos <= mReadBufferLen)
            (mReadBufferPos)(mReadBufferLen);

        mPostState = Reading;
        mTempByteBuffer = ByteBuffer(mReadByteBuffer, mReadBufferPos);
        mpPostFilter->read(mTempByteBuffer, mReadBufferLen-mReadBufferPos);
    }

    void SspiFilter::writeBuffer()
    {
        RCF_ASSERT(
            0 <= mWriteBufferPos && mWriteBufferPos <= mWriteBufferLen)
            (mWriteBufferPos)(mWriteBufferLen);

        mPostState = Writing;
       
        mByteBuffers.resize(0);
        mByteBuffers.push_back( ByteBuffer(mWriteByteBuffer, mWriteBufferPos));
        mpPostFilter->write(mByteBuffers);
    }

    bool SspiFilter::completeReadBlock()
    {
        if (mSchannel)
        {
            return true;
        }

        RCF_ASSERT(
            0 <= mReadBufferPos && mReadBufferPos <= mReadBufferLen )
            (mReadBufferPos)(mReadBufferLen);

        if (mReadBufferPos == mReadBufferLen && mReadBufferLen == 4)
        {
            // Got the 4 byte length field, now read the rest of the block.
            BOOST_STATIC_ASSERT( sizeof(unsigned int) == 4 );
            BOOST_STATIC_ASSERT( sizeof(DWORD) == 4 );

            unsigned int len = * (unsigned int *) mReadBuffer;
            bool integrity = (len & (1<<30)) ? true : false;
            bool encryption = (len & (1<<31)) ? true : false;
            len = len & ~(1<<30);
            len = len & ~(1<<31);

            RCF_ASSERT(mMaxMessageLength);

            // Check the length against the max message length.
            if (len > mMaxMessageLength)
            {
                int rcfError = mServer ? 
                    RcfError_ServerMessageLength : 
                    RcfError_ClientMessageLength;

                RCF_THROW( Exception( Error(rcfError) ) )(mMaxMessageLength)(len);
            }

            * (unsigned int *) mReadBuffer = len;

            RCF_VERIFY(
                !(integrity && encryption),
                FilterException(_RcfError_Sspi(), "both integrity and encryption requested"));
            if (mServer)
            {
                if (integrity)
                {
                    mQop = Integrity;
                }
                else if (encryption)
                {
                    mQop = Encryption;
                }
                else
                {
                    mQop = None;
                }
            }

            resizeReadBuffer(4+len);
            mReadBufferPos = 4;
            readBuffer();
            return false;
        }

        return (mReadBufferPos < mReadBufferLen) ?
            readBuffer(), false :
            true;
    }

    bool SspiFilter::completeWriteBlock()
    {
        RCF_ASSERT(
            0 <= mWriteBufferPos && mWriteBufferPos <= mWriteBufferLen )
            (mWriteBufferPos)(mWriteBufferLen);

        return (mWriteBufferPos < mWriteBufferLen) ?
            writeBuffer(), false :
            true;
    }

    bool SspiFilter::completeBlock()
    {
        // check to see if a whole block was read or written
        // if not, issue another read or write
        RCF_ASSERT(
            mPostState == Reading || mPostState == Writing )
            (mPostState);

        return
            mPostState == Reading ?
                completeReadBlock() :
                completeWriteBlock();
    }

    void SspiFilter::resizeReadBuffer(std::size_t newSize)
    {
        mTempByteBuffer.clear();
        mReadByteBuffer.clear();
        if (!mReadBufferVectorPtr)
        {
            getObjectPool().get(mReadBufferVectorPtr);
        }

        std::size_t newSize_ = newSize == 0 ? 1 : newSize;
        mReadBufferVectorPtr->resize(newSize_);
        mReadByteBuffer = ByteBuffer(mReadBufferVectorPtr);
        mReadBuffer = mReadByteBuffer.getPtr();
        mReadBufferPos = 0;
        mReadBufferLen = mReadByteBuffer.getLength();
        mReadBufferLen = (mReadBufferLen == 1) ? 0 : mReadBufferLen;

        RCF_ASSERT(mReadBufferLen == newSize)(mReadBufferLen)(newSize);
    }

    void SspiFilter::resizeWriteBuffer(std::size_t newSize)
    {
        mWriteByteBuffer.clear();
        if (!mWriteBufferVectorPtr)
        {
            getObjectPool().get(mWriteBufferVectorPtr);
        }

        std::size_t newSize_ = newSize == 0 ? 1 : newSize;
        mWriteBufferVectorPtr->resize(newSize_);
        mWriteByteBuffer = ByteBuffer(mWriteBufferVectorPtr);
        mWriteBuffer = mWriteByteBuffer.getPtr();
        mWriteBufferPos = 0;
        mWriteBufferLen = mWriteByteBuffer.getLength();
        mWriteBufferLen = mWriteBufferLen == 1 ? 0 : mWriteBufferLen;
        RCF_ASSERT(mWriteBufferLen == newSize)(mWriteBufferLen)(newSize);
    }

    void SspiFilter::shiftReadBuffer()
    {
        RCF_ASSERT(     0 < mRemainingDataPos 
                    &&  mRemainingDataPos < mReadBufferVectorPtr->size());

        mReadBufferPos = mReadBufferVectorPtr->size();
        std::size_t bytesToMove = mReadBufferPos - mRemainingDataPos;
        char * pchFrom = mReadBuffer + mRemainingDataPos;
        char * pchTo = mReadBuffer;
        memmove(pchTo, pchFrom, bytesToMove);
        mReadBufferPos = bytesToMove;
        mRemainingDataPos = 0;
        trimReadBuffer();
    }

    void SspiFilter::trimReadBuffer()
    {
        mReadBufferVectorPtr->resize(mReadBufferPos);
        mReadByteBuffer = ByteBuffer(mReadBufferVectorPtr);
        mReadBuffer = mReadByteBuffer.getPtr();
        mReadBufferLen = mReadByteBuffer.getLength();
        mReadBufferLen = (mReadBufferLen == 1) ? 0 : mReadBufferLen;
    }

    void SspiFilter::encryptWriteBuffer()
    {
        // encrypt the pre buffer to the write buffer

        RCF_ASSERT(mContextState == AuthOkAck)(mContextState);

        if (mQop == Integrity)
        {
            SecPkgContext_Sizes sizes;
            getSft()->QueryContextAttributes(
                &mContext,
                SECPKG_ATTR_SIZES,
                &sizes);

            DWORD cbPacketLength    = 4;
            DWORD cbMsgLength       = 4;
            DWORD cbMsg             = static_cast<DWORD>(mWriteByteBufferOrig.getLength());
            DWORD cbSignature       = sizes.cbMaxSignature;
            DWORD cbPacket            = cbMsgLength + cbMsg + cbSignature;

            resizeWriteBuffer(cbPacketLength + cbPacket);
            memcpy(mWriteBuffer+cbPacketLength, &cbMsg, cbMsgLength);
            memcpy(
                mWriteBuffer+cbPacketLength+cbMsgLength,
                mWriteByteBufferOrig.getPtr(),
                mWriteByteBufferOrig.getLength());

            char *pMsg              = &mWriteBuffer[4];
            SecBuffer rgsb[2]       = {0,0};
            rgsb[0].cbBuffer        = cbMsg;
            rgsb[0].BufferType      = SECBUFFER_DATA;
            rgsb[0].pvBuffer        = pMsg + cbMsgLength;
            rgsb[1].cbBuffer        = cbSignature;
            rgsb[1].BufferType      = SECBUFFER_TOKEN;
            rgsb[1].pvBuffer        = pMsg + cbMsgLength + cbMsg;
            SecBufferDesc sbd       = {0};
            sbd.ulVersion           = SECBUFFER_VERSION;
            sbd.cBuffers            = sizeof(rgsb)/sizeof(*rgsb);
            sbd.pBuffers            = rgsb;

            SECURITY_STATUS status = getSft()->MakeSignature(
                &mContext,
                0,
                &sbd,
                0);

            RCF_VERIFY(
                status == SEC_E_OK,
                FilterException(
                    _RcfError_SspiEncrypt(),
                    status,
                    RcfSubsystem_Os,
                    "MakeSignature() failed"))(status);

            cbSignature                = rgsb[1].cbBuffer;
            cbPacket                = cbMsgLength + cbMsg + cbSignature;
            resizeWriteBuffer(cbPacketLength + cbPacket);
            DWORD encodedLength        = cbPacket;
            RCF_ASSERT(encodedLength < (1<<30))(encodedLength);
            encodedLength            = encodedLength | (1<<30);
            * (DWORD*) mWriteBuffer = encodedLength;
        }
        else if (mQop == Encryption)
        {
            SecPkgContext_Sizes sizes;
            getSft()->QueryContextAttributes(
                &mContext,
                SECPKG_ATTR_SIZES,
                &sizes);

            DWORD cbPacketLength    = 4;
            DWORD cbMsgLength       = 4;
            DWORD cbMsg             = static_cast<DWORD>(mWriteByteBufferOrig.getLength());
            DWORD cbTrailer         = sizes.cbSecurityTrailer;
            DWORD cbPacket            = cbMsgLength + cbMsg + cbTrailer;

            resizeWriteBuffer(cbPacketLength + cbPacket);
            memcpy(mWriteBuffer+cbPacketLength, &cbMsg, cbMsgLength);
            memcpy(
                mWriteBuffer+cbPacketLength+cbMsgLength,
                mWriteByteBufferOrig.getPtr(),
                mWriteByteBufferOrig.getLength());

            BYTE *pEncryptedMsg     =((BYTE *) mWriteBuffer) + 4;
            SecBuffer rgsb[2]       = {0,0};
            rgsb[0].cbBuffer        = cbMsg;
            rgsb[0].BufferType      = SECBUFFER_DATA;
            rgsb[0].pvBuffer        = pEncryptedMsg + cbMsgLength;
            rgsb[1].cbBuffer        = cbTrailer;
            rgsb[1].BufferType      = SECBUFFER_TOKEN;
            rgsb[1].pvBuffer        = pEncryptedMsg + cbMsgLength + cbMsg;
            SecBufferDesc sbd       = {0};
            sbd.ulVersion           = SECBUFFER_VERSION;
            sbd.cBuffers            = sizeof(rgsb)/sizeof(*rgsb);
            sbd.pBuffers            = rgsb;

            SECURITY_STATUS status = getSft()->EncryptMessage(
                &mContext,
                0,
                &sbd,
                0);

            RCF_VERIFY(
                status == SEC_E_OK,
                FilterException(
                    _RcfError_SspiEncrypt(),
                    status,
                    RcfSubsystem_Os,
                    "EncryptMessage() failed"))(status);

            cbTrailer               = rgsb[1].cbBuffer;
            cbPacket                = cbMsgLength + cbMsg + cbTrailer;
            resizeWriteBuffer(cbPacketLength + cbPacket);
            DWORD encodedLength     = cbPacket;
            RCF_ASSERT(encodedLength < (1<<30))(encodedLength);
            encodedLength           = encodedLength | (1<<31);
            * (DWORD*) mWriteBuffer = encodedLength;
        }
        else
        {
            RCF_ASSERT(mQop == None)(mQop);
            RCF_ASSERT(
                mWriteByteBufferOrig.getLength() < (1<<31))
                (mWriteByteBufferOrig.getLength());

            resizeWriteBuffer(mWriteByteBufferOrig.getLength()+4);
            memcpy(
                mWriteBuffer+4,
                mWriteByteBufferOrig.getPtr(),
                mWriteByteBufferOrig.getLength());

            DWORD dw = static_cast<DWORD>(mWriteByteBufferOrig.getLength());
            *(DWORD*) mWriteBuffer  = dw;
        }

    }

    bool SspiFilter::decryptReadBuffer()
    {
        // decrypt read buffer in place

        RCF_ASSERT(mContextState == AuthOkAck)(mContextState);

        if (mQop == Integrity)
        {
            BYTE *pMsg              = ((BYTE *) mReadBuffer) + 4;
            DWORD cbPacketLength    = 4;
            DWORD cbPacket          = *(DWORD*) mReadBuffer;
            DWORD cbMsgLength       = 4;
            DWORD cbMsg             = *(DWORD*) pMsg;
            DWORD cbSignature       = cbPacket - cbMsgLength - cbMsg;
            SecBuffer rgsb[2]       = {0,0};
            rgsb[0].cbBuffer        = cbMsg;
            rgsb[0].BufferType      = SECBUFFER_DATA;
            rgsb[0].pvBuffer        = pMsg + cbMsgLength;
            rgsb[1].cbBuffer        = cbSignature;
            rgsb[1].BufferType      = SECBUFFER_TOKEN;
            rgsb[1].pvBuffer        = pMsg + cbMsgLength + cbMsg;
            SecBufferDesc sbd       = {0};
            sbd.ulVersion           = SECBUFFER_VERSION;
            sbd.cBuffers            = sizeof(rgsb)/sizeof(*rgsb);
            sbd.pBuffers            = rgsb;
            ULONG qop               = 0;
            SECURITY_STATUS status  = getSft()->VerifySignature(
                &mContext,
                &sbd,
                0,
                &qop);

            RCF_VERIFY(
                status == SEC_E_OK,
                FilterException(
                    _RcfError_SspiDecrypt(),
                    status,
                    RcfSubsystem_Os,
                    "VerifySignature() failed"))(status);

            resizeReadBuffer(cbPacketLength + cbMsgLength + cbMsg);
            mReadBufferPos          = cbPacketLength + cbMsgLength;
        }
        else if (mQop == Encryption)
        {
            BYTE *pMsg              = ((BYTE *) mReadBuffer) + 4;
            DWORD cbPacketLength    = 4;
            DWORD cbPacket          = *(DWORD*)mReadBuffer;
            DWORD cbMsgLength       = 4;
            DWORD cbMsg             = *(DWORD*) pMsg;
            DWORD cbTrailer         = (cbPacket - cbMsgLength) - cbMsg;
            SecBuffer rgsb[2]       = {0,0};
            rgsb[0].cbBuffer        = cbMsg;
            rgsb[0].BufferType      = SECBUFFER_DATA;
            rgsb[0].pvBuffer        = pMsg + cbMsgLength;
            rgsb[1].cbBuffer        = cbTrailer;
            rgsb[1].BufferType      = SECBUFFER_TOKEN;
            rgsb[1].pvBuffer        = pMsg + cbMsgLength + cbMsg;
            SecBufferDesc sbd       = {0};
            sbd.ulVersion           = SECBUFFER_VERSION;
            sbd.cBuffers            = sizeof(rgsb)/sizeof(*rgsb);
            sbd.pBuffers            = rgsb;
            ULONG qop               = 0;

            SECURITY_STATUS status  = getSft()->DecryptMessage(
                &mContext,
                &sbd,
                0,
                &qop);

            RCF_VERIFY(
                status == SEC_E_OK,
                FilterException(
                    _RcfError_SspiDecrypt(),
                    status,
                    RcfSubsystem_Os,
                    "DecryptMessage() failed"))(status);

            resizeReadBuffer(cbPacketLength + cbMsgLength + cbMsg);
            mReadBufferPos          = cbPacketLength + cbMsgLength;
        }
        else
        {
            RCF_ASSERT(mQop == None)(mQop);
            mReadBufferPos = 4;
        }

        return true;
    }

    void SspiFilter::resumeUserIo()
    {
        RCF_ASSERT( mPreState == Reading || mPreState == Writing )(mPreState);
        handleEvent( mPreState == Reading ? ReadIssued : WriteIssued );
    }

    SspiImpersonator::SspiImpersonator(SspiFilterPtr sspiFilterPtr) :
        mSspiFilterPtr(sspiFilterPtr)
    {
    }

    SspiImpersonator::~SspiImpersonator()
    {
        RCF_DTOR_BEGIN
            revertToSelf();
        RCF_DTOR_END
    }

    bool SspiImpersonator::impersonate()
    {
        if (mSspiFilterPtr)
        {
            RCF_ASSERT(
                mSspiFilterPtr->mContextState == SspiFilter::AuthOkAck )
                (mSspiFilterPtr->mContextState);

            SECURITY_STATUS status = 
                getSft()->ImpersonateSecurityContext(&mSspiFilterPtr->mContext);

            RCF_VERIFY(
                status == SEC_E_OK,
                FilterException(
                    _RcfError_SspiImpersonation(), status, RcfSubsystem_Os,
                    "ImpersonateSecurityContext() failed"))(status);

            return true;
        }
        else
        {
            return false;
        }
    }

    void SspiImpersonator::revertToSelf() const
    {
        if (mSspiFilterPtr)
        {
            RCF_ASSERT( mSspiFilterPtr->mContextState == SspiFilter::AuthOkAck );

            SECURITY_STATUS status = 
                getSft()->RevertSecurityContext(&mSspiFilterPtr->mContext);

            RCF_VERIFY(
                status == SEC_E_OK,
                FilterException(
                    _RcfError_SspiImpersonation(), status, RcfSubsystem_Os,
                    "RevertSecurityContext() failed"));
        }
    }
   
    bool SspiServerFilter::doHandshake()
    {
        // use the block in the read buffer to proceed through the handshake procedure

        // lazy acquiring of implicit credentials
        if (mImplicitCredentials && !mHaveCredentials)
        {
            acquireCredentials();
        }

        DWORD cbPacket          = mPkgInfo.cbMaxToken;
        DWORD cbPacketLength    = 4;

        std::vector<char> vec(cbPacketLength + cbPacket);

        BYTE *pPacket           = (BYTE*) &vec[0];
        SecBuffer ob            = {0};
        ob.BufferType           = SECBUFFER_TOKEN;
        ob.cbBuffer             = cbPacket;
        ob.pvBuffer             = pPacket+cbPacketLength;
        SecBufferDesc obd       = {0};
        obd.cBuffers            = 1;
        obd.ulVersion           = SECBUFFER_VERSION;
        obd.pBuffers            = &ob;

        RCF_ASSERT(
            mReadBufferLen == 0 || mReadBufferLen > 4)
            (mReadBufferLen);

        RCF_ASSERT(
            !mServer || (mServer && mReadBufferLen > 4))
            (mServer)(mReadBufferLen);

        SecBufferDesc ibd       = {0};
        SecBuffer ib            = {0};
        if (mReadBufferLen > 4)
        {
            ib.BufferType       = SECBUFFER_TOKEN;
            ib.cbBuffer         = *(DWORD *)mReadBuffer;
            ib.pvBuffer         = mReadBuffer+cbPacketLength;
            ibd.cBuffers        = 1;
            ibd.ulVersion       = SECBUFFER_VERSION;
            ibd.pBuffers        = &ib;
        }

        DWORD   CtxtAttr        = 0;
        TimeStamp Expiration    = {0};
        SECURITY_STATUS status  = getSft()->AcceptSecurityContext(
            &mCredentials,
            mHaveContext ? &mContext : NULL,
            &ibd,
            mContextRequirements,
            SECURITY_NATIVE_DREP,
            &mContext,
            &obd,
            &CtxtAttr,
            &Expiration);

        switch (status)
        {
        case SEC_E_OK:
        case SEC_I_CONTINUE_NEEDED:
        case SEC_I_COMPLETE_NEEDED:
        case SEC_I_COMPLETE_AND_CONTINUE:
        case SEC_E_INCOMPLETE_MESSAGE:
            mHaveContext = true;
            break;
        default:
            break;
        }

        cbPacket = ob.cbBuffer;

        // We only support NTLM, Kerberos and Negotiate SSP's, so there's never
        // a need to call CompleteAuthToken()
        RCF_ASSERT(
            status != SEC_I_COMPLETE_AND_CONTINUE &&
            status != SEC_I_COMPLETE_NEEDED)
            (status);

        if (status == SEC_I_CONTINUE_NEEDED)
        {
            // authorization ok so far, copy outbound data to write buffer
            mContextState = AuthContinue;
            *(DWORD *) pPacket = cbPacket;
            resizeWriteBuffer(cbPacketLength + cbPacket);
            memcpy(mWriteBuffer, pPacket, cbPacketLength + cbPacket);
        }
        else if (status == SEC_E_OK)
        {
            // authorization ok, send a special block of our own to notify client
            mContextState = AuthOk;
            if (cbPacket > 0)
            {
                *(DWORD *) pPacket = cbPacket;
                resizeWriteBuffer(cbPacketLength + cbPacket);
                memcpy(mWriteBuffer, pPacket, cbPacketLength + cbPacket);
            }
            else
            {
                resizeWriteBuffer(4+4+4);
                *(DWORD*) mWriteBuffer = 8;
                *(DWORD*) (mWriteBuffer+4) = RcfError_Ok;
                *(DWORD*) (mWriteBuffer+8) = 0;
            }
        }
        else
        {
            // authorization failed, send a special block of our own to notify client
            mContextState = AuthFailed;
            resizeWriteBuffer(4+4+4);
            *(DWORD*) mWriteBuffer = 8;
            *(DWORD*) (mWriteBuffer+4) = RcfError_SspiAuthFailServer;
            *(DWORD*) (mWriteBuffer+8) = status;
        }

        return true;
    }

    void SspiServerFilter::handleHandshakeEvent()
    {
        // take another step through the handshake process

        switch (mEvent)
        {
        case ReadIssued:
        case WriteIssued:

            // read first block from client
            RCF_ASSERT(mEvent == ReadIssued)(mEvent);
            resizeReadBuffer(mReadAheadChunkSize);
            readBuffer();
            break;

        case ReadCompleted:
           
            // process inbound block and write outbound block
            {
                bool written = mSchannel ? 
                    doHandshakeSchannel() : 
                    doHandshake();

                if (written)
                {
                    writeBuffer();
                }
            }
            break;

        case WriteCompleted:

            switch (mContextState)
            {
            case AuthOk:
                mContextState = AuthOkAck;
                resumeUserIo();
                break;

            case AuthFailed:
                RCF_THROW(FilterException(_RcfError_SspiAuthFailServer()));
                break;

            default:
                resizeReadBuffer(mReadAheadChunkSize);
                readBuffer();
            }
            break;
        default:
            RCF_ASSERT(0);
        }
    }

    bool SspiClientFilter::doHandshake()
    {
        // use the block in the read buffer to proceed through the handshake procedure

        // lazy acquiring of implicit credentials
        if (mImplicitCredentials && !mHaveCredentials)
        {
            acquireCredentials();
        }

        if (mContextState == AuthOk)
        {
            if (mReadBufferLen == 12)
            {
                DWORD rcfErr = *(DWORD*) &mReadBuffer[4];
                DWORD osErr = *(DWORD*) &mReadBuffer[8];
                if (rcfErr == RcfError_Ok)
                {
                    mContextState = AuthOkAck;
                    resumeUserIo();
                    return false;
                }
                else
                {
                    RCF_THROW(RemoteException( Error(rcfErr), osErr, RcfSubsystem_Os));
                }
            }
            else
            {
                RCF_THROW(Exception(_RcfError_SspiAuthFailServer()));
            }
        }
       
        DWORD cbPacketLength    = 4;
        DWORD cbPacket          = mPkgInfo.cbMaxToken;
        std::vector<char> vec(cbPacket + cbPacketLength);

        BYTE *pPacket           = (BYTE*) &vec[0];
        SecBuffer ob            = {0};
        ob.BufferType           = SECBUFFER_TOKEN;
        ob.cbBuffer             = cbPacket;
        ob.pvBuffer             = pPacket + cbPacketLength;
        SecBufferDesc obd       = {0};
        obd.cBuffers            = 1;
        obd.ulVersion           = SECBUFFER_VERSION;
        obd.pBuffers            = &ob;

        RCF_ASSERT(
            mReadBufferLen == 0 || mReadBufferLen > 4)
            (mReadBufferLen);

        RCF_ASSERT(
            !mServer || (mServer && mReadBufferLen > 4))
            (mServer)(mReadBufferLen);

        SecBuffer ib            = {0};
        SecBufferDesc ibd       = {0};

        if (mReadBufferLen > 4)
        {
            ib.BufferType       = SECBUFFER_TOKEN;
            ib.cbBuffer         = *(DWORD *) mReadBuffer;
            ib.pvBuffer         = mReadBuffer + cbPacketLength;
            ibd.cBuffers        = 1;
            ibd.ulVersion       = SECBUFFER_VERSION;
            ibd.pBuffers        = &ib;
        }

        const TCHAR *target = mTarget.empty() ? RCF_T("") : mTarget.c_str();

        DWORD CtxtAttr          = 0;
        TimeStamp Expiration    = {0};
        ULONG CtxtReq =  mContextRequirements;

        SECURITY_STATUS status  = getSft()->InitializeSecurityContext(
            &mCredentials,
            mHaveContext ? &mContext : NULL,
            (TCHAR *) target,
            CtxtReq,
            0,
            SECURITY_NATIVE_DREP,
            (mHaveContext && mReadBufferLen > 4) ? &ibd : NULL,
            0,
            &mContext,
            &obd,
            &CtxtAttr,
            &Expiration);

        switch (status)
        {
        case SEC_E_OK:
        case SEC_I_CONTINUE_NEEDED:
        case SEC_I_COMPLETE_NEEDED:
        case SEC_I_COMPLETE_AND_CONTINUE:
        case SEC_E_INCOMPLETE_MESSAGE:
        case SEC_I_INCOMPLETE_CREDENTIALS:
            mHaveContext = true;
            break;
        default:
            break;
        }

        RCF_ASSERT(
            status != SEC_I_COMPLETE_NEEDED &&
            status != SEC_I_COMPLETE_AND_CONTINUE)
            (status);
       
        cbPacket                = ob.cbBuffer;
        if (cbPacket > 0)
        {
            *(DWORD *)pPacket   = cbPacket;
            mContextState       =
                (status == SEC_E_OK) ?
                    AuthOk :
                    (status == SEC_I_CONTINUE_NEEDED) ?
                        AuthContinue :
                        AuthFailed;

            RCF_VERIFY(
                mContextState != AuthFailed,
                Exception(
                    _RcfError_SspiAuthFailClient(),
                    status,
                    RcfSubsystem_Os,
                    "InitializeSecurityContext() failed"))(status);

            resizeWriteBuffer(cbPacketLength + cbPacket);
            memcpy(mWriteBuffer, pPacket, cbPacketLength + cbPacket);
            return true;
        }
        else
        {
            mContextState = AuthOkAck;
            resumeUserIo();
            return false;
        }

    }

    void SspiClientFilter::handleHandshakeEvent()
    {
        // take another step through the handshake process

        switch (mEvent)
        {
        case ReadIssued:
        case WriteIssued:
           
            // create first block to send to server
            //resizeReadBuffer(0);
            mSchannel ? 
                doHandshakeSchannel() : 
                doHandshake();

            writeBuffer();
            break;

        case ReadCompleted:

            // process a block, and send any emitted output block
            {
            bool written = mSchannel ? 
                doHandshakeSchannel() : 
                doHandshake();

            if (written)
            {
                writeBuffer();
            }
            }
            break;

        case WriteCompleted:

            // issue a read for the next block from the server
            resizeReadBuffer(mReadAheadChunkSize);
            readBuffer();
            break;

        default:
            RCF_ASSERT(0);
        }
    }

    void SspiFilter::setupCredentials(
        const tstring &userName,
        const tstring &password,
        const tstring &domain)
    {

#if defined(_MSC_VER) && _MSC_VER == 1200
        SEC_WINNT_AUTH_IDENTITY identity        = {0};
#else
        SEC_WINNT_AUTH_IDENTITY_EX identity     = {0};
#endif

        UTCHAR *pDomain = reinterpret_cast<UTCHAR*>(const_cast<TCHAR*>(domain.c_str()));
        unsigned long pDomainLen = static_cast<unsigned long>(domain.length());

        UTCHAR *pUsername = reinterpret_cast<UTCHAR*>(const_cast<TCHAR*>(userName.c_str()));
        unsigned long pUsernameLen = static_cast<unsigned long>(userName.length());

        UTCHAR *pPassword = reinterpret_cast<UTCHAR*>(const_cast<TCHAR*>(password.c_str()));
        unsigned long pPasswordLen = static_cast<unsigned long>(password.length());

        UTCHAR *pPackages = reinterpret_cast<UTCHAR*>(const_cast<TCHAR*>(mPackageList.c_str()));
        unsigned long pPackagesLen = static_cast<unsigned long>(mPackageList.length());

        if (!userName.empty())
        {
            if (!domain.empty())
            {
                identity.Domain                 = pDomain;
                identity.DomainLength           = pDomainLen;
            }
            if (!userName.empty())
            {
                identity.User                   = pUsername;
                identity.UserLength             = pUsernameLen;
            }
            if (!password.empty())
            {
                identity.Password               = pPassword;
                identity.PasswordLength         = pPasswordLen;
            }
        }

#ifdef _UNICODE
        identity.Flags                          = SEC_WINNT_AUTH_IDENTITY_UNICODE;
#else
        identity.Flags                          = SEC_WINNT_AUTH_IDENTITY_ANSI;
#endif

#if defined(_MSC_VER) && _MSC_VER == 1200
        void *pIdentity = &identity;
#else
        identity.Version                        = SEC_WINNT_AUTH_IDENTITY_VERSION;
        identity.Length                         = sizeof(identity);
        if (!mPackageList.empty())
        {
            identity.PackageList                = pPackages;
            identity.PackageListLength          = pPackagesLen;
        }
        SEC_WINNT_AUTH_IDENTITY_EX *pIdentity = &identity;
#endif

        TimeStamp Expiration                    = {0};

        SECURITY_STATUS status = getSft()->AcquireCredentialsHandle(
            NULL,
            mPkgInfo.Name,
            mServer ? SECPKG_CRED_INBOUND : SECPKG_CRED_OUTBOUND ,
            NULL,
            pIdentity,
            NULL, NULL,
            &mCredentials,
            &Expiration);

        if (status != SEC_E_OK)
        {
            RCF_THROW(
                FilterException(
                _RcfError_Sspi(), status, RcfSubsystem_Os,
                "AcquireCredentialsHandle() failed"))
                (mPkgInfo.Name)(userName)(domain)(status);
        }

        mHaveCredentials = true;
    }

    void SspiFilter::acquireCredentials(
        const tstring &userName,
        const tstring &password,
        const tstring &domain)
    {
        // acquire credentials, implicitly (currently logged on user),
        // or explicitly (supply username and password)

        RCF_ASSERT(!mHaveCredentials);

        // TODO: whats with copying pPackage here?

        // setup security package
        SecPkgInfo *pPackage = NULL;
       
        SECURITY_STATUS status = getSft()->QuerySecurityPackageInfo(
            (TCHAR*) mPackageName.c_str(),
            &pPackage);

        if ( status != SEC_E_OK )
        {
            RCF_THROW(
                FilterException(
                    _RcfError_Sspi(), status, RcfSubsystem_Os,
                    "QuerySecurityPackageInfo() failed"))
                (mPackageName.c_str())(status);
        }

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4995 )  // warning C4995: '...': name was marked as #pragma deprecated
#pragma warning( disable : 4996 )  // warning C4996: '...' was declared deprecated
#endif

        TCHAR *pName = new TCHAR[ _tcslen(pPackage->Name) + 1 ];
        _tcscpy(pName, pPackage->Name);

        TCHAR *pComment = new TCHAR[ _tcslen(pPackage->Comment) + 1 ];
        _tcscpy(pComment, pPackage->Comment);

#ifdef _MSC_VER
#pragma warning( pop )
#endif

        memcpy ( (void*)&mPkgInfo, (void*)pPackage, sizeof(SecPkgInfo) );
        mPkgInfo.Name = pName;
        mPkgInfo.Comment = pComment;

        getSft()->FreeContextBuffer( (void*) pPackage );

        mSchannel ?
            setupCredentialsSchannel() :
            setupCredentials(userName, password, domain);
    }

    //**************************************************************************

    const FilterDescription *gpNtlmFilterDescription = NULL;
    const FilterDescription *gpKerberosFilterDescription = NULL;
    const FilterDescription *gpNegotiateFilterDescription = NULL;
    const FilterDescription *gpSchannelFilterDescription = NULL;

    //**************************************************************************
    // Server filters.

    SspiServerFilter::SspiServerFilter(
        const tstring &packageName,
        const tstring &packageList,
        bool schannel) :
            SspiFilter(packageName, packageList, BoolServer, schannel)
    {
        RcfSessionPtr sessionPtr = RCF::getCurrentRcfSessionPtr();
        if (sessionPtr)
        {
            I_ServerTransport & serverTransport = 
                sessionPtr->getProactor().getServerTransport();

            mMaxMessageLength = serverTransport.getMaxMessageLength();
        }
    }

    // NTLM
    NtlmServerFilter::NtlmServerFilter() :
        SspiServerFilter(RCF_T("NTLM"), RCF_T(""))
    {}

    const FilterDescription &NtlmServerFilter::getFilterDescription() const
    {
        return *gpNtlmFilterDescription;
    }

    // Kerberos
    KerberosServerFilter::KerberosServerFilter() :
        SspiServerFilter(RCF_T("Kerberos"), RCF_T(""))
    {}

    const FilterDescription &KerberosServerFilter::getFilterDescription() const
    {
        return *gpKerberosFilterDescription;
    }

    // Negotiate
    NegotiateServerFilter::NegotiateServerFilter(
        const tstring &packageList) :
            SspiServerFilter(RCF_T("Negotiate"), packageList)
    {}

    const FilterDescription &NegotiateServerFilter::getFilterDescription() const
    {
        return *gpNegotiateFilterDescription;
    }

    // Schannel
    const FilterDescription &SchannelServerFilter::getFilterDescription() const
    {
        return *gpSchannelFilterDescription;
    }

    //**************************************************************************
    // Filter factories

    // NTLM
    FilterPtr NtlmFilterFactory::createFilter()
    {
        return FilterPtr( new NtlmServerFilter() );
    }
    const FilterDescription &NtlmFilterFactory::getFilterDescription()
    {
        return *gpNtlmFilterDescription;
    }

    // Kerberos
    FilterPtr KerberosFilterFactory::createFilter()
    {
        return FilterPtr( new KerberosServerFilter() );
    }
    const FilterDescription &KerberosFilterFactory::getFilterDescription()
    {
        return *gpKerberosFilterDescription;
    }

    // Negotiate
    NegotiateFilterFactory::NegotiateFilterFactory(
        const tstring &packageList) :
            mPackageList(packageList)
    {}

    FilterPtr NegotiateFilterFactory::createFilter()
    {
        return FilterPtr( new NegotiateServerFilter(mPackageList) );
    }
    const FilterDescription &NegotiateFilterFactory::getFilterDescription()
    {
        return *gpNegotiateFilterDescription;
    }

    // Schannel
    const FilterDescription &SchannelFilterFactory::getFilterDescription()
    {
        return *gpSchannelFilterDescription;
    }

    //**************************************************************************
    // Client filters

    // NTLM
    NtlmClientFilter::NtlmClientFilter(
        const tstring &         userName,
        const tstring &         password,
        const tstring &         domain,
        QualityOfProtection     qop,
        ULONG                   contextRequirements) :
            SspiClientFilter(
                userName, 
                password, 
                domain,
                RCF_T(""), 
                qop, 
                contextRequirements, 
                RCF_T("NTLM"), 
                RCF_T(""))
    {}

    NtlmClientFilter::NtlmClientFilter(
        QualityOfProtection     qop,
        ULONG                   contextRequirements) :
            SspiClientFilter(
                RCF_T(""), 
                qop, 
                contextRequirements, 
                RCF_T("NTLM"), 
                RCF_T(""),
                BoolClient)
    {}

    const FilterDescription & 
    NtlmClientFilter::getFilterDescription() const
    {
        return *gpNtlmFilterDescription;
    }

    // Kerberos
    KerberosClientFilter::KerberosClientFilter(
        const tstring &         userName,
        const tstring &         password,
        const tstring &         domain,
        const tstring &         targetName,
        QualityOfProtection     qop,
        ULONG                   contextRequirements) :
            SspiClientFilter(
                userName, 
                password, 
                domain,
                targetName, 
                qop, 
                contextRequirements, 
                RCF_T("Kerberos"), 
                RCF_T(""))
    {}

    KerberosClientFilter::KerberosClientFilter(
        const tstring &         targetName,
        QualityOfProtection     qop,
        ULONG                   contextRequirements) :
            SspiClientFilter(
                targetName,
                qop, 
                contextRequirements, 
                RCF_T("Kerberos"), 
                RCF_T(""),
                BoolClient)
    {}

    const FilterDescription & 
    KerberosClientFilter::getFilterDescription() const
    {
        return *gpKerberosFilterDescription;
    }

    // Negotiate
    NegotiateClientFilter::NegotiateClientFilter(
        const tstring &         userName,
        const tstring &         password,
        const tstring &         domain,
        const tstring &         targetName,
        QualityOfProtection     qop,
        ULONG                   contextRequirements) :
            SspiClientFilter(
                userName, 
                password, 
                domain,
                targetName, 
                qop, 
                contextRequirements, 
                RCF_T("Negotiate"), 
                RCF_T("Kerberos,NTLM"))
    {}

    NegotiateClientFilter::NegotiateClientFilter(
        const tstring &         targetName,
        QualityOfProtection     qop,
        ULONG                   contextRequirements) :
            SspiClientFilter(
                targetName,
                qop, 
                contextRequirements, 
                RCF_T("Negotiate"), 
                RCF_T("Kerberos,NTLM"),
                BoolClient)
    {}

    const FilterDescription & 
    NegotiateClientFilter::getFilterDescription() const
    {
        return *gpNegotiateFilterDescription;
    }

    // Schannel
    const FilterDescription & 
    SchannelClientFilter::getFilterDescription() const
    {
        return *gpSchannelFilterDescription;
    }

    //************************************************************************

    // Global init and deinit.

    HINSTANCE               ghProvider          = NULL;      // provider dll's instance
    PSecurityFunctionTable  gpSecurityInterface = NULL;      // security interface table

    PSecurityFunctionTable getSft()
    {
        return gpSecurityInterface;
    }

    void SspiInitialize()
    {
        // load the provider dll
        ghProvider = LoadLibrary ( RCF_T("security.dll") );
        if (ghProvider == NULL)
        {
            int err = GetLastError();
            RCF_THROW(
                FilterException(
                    _RcfError_SspiInit(),
                    err,
                    RcfSubsystem_Os,
                    "LoadLibrary(\"security.dll\") failed"));
        }

        INIT_SECURITY_INTERFACE InitSecurityInterface;

        InitSecurityInterface = reinterpret_cast<INIT_SECURITY_INTERFACE> (
            GetProcAddress(ghProvider, INIT_SEC_INTERFACE_NAME));

        if (InitSecurityInterface == NULL)
        {
            int err = GetLastError();
            RCF_THROW(
                FilterException(_RcfError_SspiInit(), err, RcfSubsystem_Os,
                "GetProcAddress() failed to retrieve address of InitSecurityInterface())"));
        }

        gpSecurityInterface = InitSecurityInterface();
        if (gpSecurityInterface == NULL)
        {
            int err = GetLastError();
            RCF_THROW(
                FilterException(_RcfError_SspiInit(), err, RcfSubsystem_Os,
                "InitSecurityInterface() failed"));
        }

        // try to load the GetUserNameEx() function, if we can
        hModuleSecur32 = LoadLibrary( RCF_T("secur32.dll"));
        if (hModuleSecur32)
        {
            pfnGetUserNameEx = (PfnGetUserNameEx) GetProcAddress(hModuleSecur32, GetUserNameExName);
        }

    }

    void SspiUninitialize()
    {
        FreeLibrary (ghProvider);
        ghProvider = NULL;
        gpSecurityInterface = NULL;

        if (hModuleSecur32)   
        {
            FreeLibrary(hModuleSecur32);
            hModuleSecur32 = 0;
            pfnGetUserNameEx = NULL;
        }
    }

    RCF_ON_INIT_DEINIT( SspiInitialize(), SspiUninitialize() )

    static void initSspiFilterDescriptions()
    {
        RCF_ASSERT(!gpNtlmFilterDescription);
        RCF_ASSERT(!gpKerberosFilterDescription);
        RCF_ASSERT(!gpNegotiateFilterDescription);
        RCF_ASSERT(!gpSchannelFilterDescription);

        gpNtlmFilterDescription =
            new FilterDescription(
                "sspi ntlm filter",
                RcfFilter_SspiNtlm,
                true);
       
        gpKerberosFilterDescription =
            new FilterDescription(
                "sspi kerberos filter",
                RcfFilter_SspiKerberos,
                true);
       
        gpNegotiateFilterDescription =
            new FilterDescription(
                "sspi negotiate filter",
                RcfFilter_SspiNegotiate,
                true);

        gpSchannelFilterDescription =
            new FilterDescription(
                "sspi schannel filter",
                RcfFilter_SspiSchannel,
                true);
    }

    static void deinitSspiFilterDescriptions()
    {
        delete gpNtlmFilterDescription;
        gpNtlmFilterDescription = NULL;

        delete gpKerberosFilterDescription;
        gpKerberosFilterDescription = NULL;

        delete gpNegotiateFilterDescription;
        gpNegotiateFilterDescription = NULL;

        delete gpSchannelFilterDescription;
        gpSchannelFilterDescription = NULL;
    }

    RCF_ON_INIT_DEINIT(
        initSspiFilterDescriptions(),
        deinitSspiFilterDescriptions())

} // namespace RCF
