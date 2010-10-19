
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/Schannel.hpp>

#include <RCF/Exception.hpp>
#include <RCF/SspiFilter.hpp>
#include <RCF/Tools.hpp>

#include <wincrypt.h>
#include <schnlsp.h>

#ifdef __MINGW32__
#ifndef CERT_STORE_ADD_USE_EXISTING
#define CERT_STORE_ADD_USE_EXISTING                         2
#endif
#endif // __MINGW32__

#if defined(_MSC_VER) && _MSC_VER < 1310
#ifndef SEC_E_UNTRUSTED_ROOT
#define SEC_E_UNTRUSTED_ROOT             _HRESULT_TYPEDEF_(0x80090325L)
#endif
#endif

namespace RCF {

    PSecurityFunctionTable getSft();

    void SspiFilter::encryptWriteBufferSchannel()
    {
        // encrypt the pre buffer to the write buffer

        RCF_ASSERT(mContextState == AuthOkAck)(mContextState);

        SecPkgContext_Sizes sizes;
        getSft()->QueryContextAttributes(
            &mContext,
            SECPKG_ATTR_SIZES,
            &sizes);

        SecPkgContext_StreamSizes streamSizes;
        getSft()->QueryContextAttributes(
            &mContext,
            SECPKG_ATTR_STREAM_SIZES,
            &streamSizes);

        DWORD cbHeader          = streamSizes.cbHeader;
        DWORD cbMsg             = static_cast<DWORD>(mWriteByteBufferOrig.getLength());
        DWORD cbTrailer         = streamSizes.cbTrailer;
        DWORD cbPacket          = cbHeader + cbMsg + cbTrailer;

        resizeWriteBuffer(cbPacket);
        
        memcpy(
            mWriteBuffer+cbHeader,
            mWriteByteBufferOrig.getPtr(),
            mWriteByteBufferOrig.getLength());

        BYTE *pEncryptedMsg     =((BYTE *) mWriteBuffer);

        SecBuffer rgsb[4]       = {0};
        rgsb[0].cbBuffer        = cbHeader;
        rgsb[0].BufferType      = SECBUFFER_STREAM_HEADER;
        rgsb[0].pvBuffer        = pEncryptedMsg;

        rgsb[1].cbBuffer        = cbMsg;
        rgsb[1].BufferType      = SECBUFFER_DATA;
        rgsb[1].pvBuffer        = pEncryptedMsg + cbHeader;

        rgsb[2].cbBuffer        = cbTrailer;
        rgsb[2].BufferType      = SECBUFFER_STREAM_TRAILER;
        rgsb[2].pvBuffer        = pEncryptedMsg + cbHeader + cbMsg;

        rgsb[3].cbBuffer        = 0;
        rgsb[3].BufferType      = SECBUFFER_EMPTY;
        rgsb[3].pvBuffer        = NULL;

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

        RCF_ASSERT(rgsb[0].cbBuffer == cbHeader);
        RCF_ASSERT(rgsb[1].cbBuffer == cbMsg);
        RCF_ASSERT(rgsb[2].cbBuffer <= cbTrailer);

        cbTrailer               = rgsb[2].cbBuffer;
        cbPacket                = cbHeader + cbMsg + cbTrailer;
        resizeWriteBuffer(cbPacket);
    }

    bool SspiFilter::decryptReadBufferSchannel()
    {
        // decrypt read buffer in place

        RCF_ASSERT(mContextState == AuthOkAck)(mContextState);

        BYTE *pMsg              = ((BYTE *) mReadBuffer);
        DWORD cbMsg             = static_cast<DWORD>(mReadBufferPos);
        SecBuffer rgsb[4]       = {0};
        
        rgsb[0].cbBuffer        = cbMsg;
        rgsb[0].BufferType      = SECBUFFER_DATA;
        rgsb[0].pvBuffer        = pMsg;

        rgsb[1].cbBuffer        = 0;
        rgsb[1].BufferType      = SECBUFFER_EMPTY;
        rgsb[1].pvBuffer        = NULL;

        rgsb[2].cbBuffer        = 0;
        rgsb[2].BufferType      = SECBUFFER_EMPTY;
        rgsb[2].pvBuffer        = NULL;

        rgsb[3].cbBuffer        = 0;
        rgsb[3].BufferType      = SECBUFFER_EMPTY;
        rgsb[3].pvBuffer        = NULL;

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

        if (status == SEC_E_INCOMPLETE_MESSAGE)
        {
            // Not enough data.
            std::size_t readBufferPos = mReadBufferPos;
            resizeReadBuffer(mReadBufferPos + mReadAheadChunkSize);
            mReadBufferPos = readBufferPos;
            readBuffer();
            return false;
        }
        else
        {
            for (int i=1; i<4; ++i)
            {
                if (rgsb[i].BufferType == SECBUFFER_EXTRA)
                {
                    // Found extra data - set a marker where it begins.
                    char * pRemainingData = (char *) rgsb[i].pvBuffer;
                    mRemainingDataPos = pRemainingData - mReadBuffer;
                    RCF_ASSERT(0 < mRemainingDataPos && mRemainingDataPos < mReadBufferPos);
                    break;
                }
            }            
        }

        trimReadBuffer();

        RCF_VERIFY(
            status == SEC_E_OK,
            FilterException(
                _RcfError_SspiDecrypt(),
                status,
                RcfSubsystem_Os,
                "DecryptMessage() failed"))(status);

        RCF_ASSERT(rgsb[0].BufferType == SECBUFFER_STREAM_HEADER);
        RCF_ASSERT(rgsb[1].BufferType == SECBUFFER_DATA);
        RCF_ASSERT(rgsb[2].BufferType == SECBUFFER_STREAM_TRAILER);

        DWORD cbHeader          = rgsb[0].cbBuffer;
        DWORD cbData            = rgsb[1].cbBuffer;
        DWORD cbTrailer         = rgsb[2].cbBuffer;

        RCF_UNUSED_VARIABLE(cbTrailer);

        mReadBufferPos          = cbHeader;
        mReadBufferLen          = cbHeader + cbData;

        return true;
    }

    bool SspiServerFilter::doHandshakeSchannel()
    {
        // use the block in the read buffer to proceed through the handshake procedure

        // lazy acquiring of implicit credentials
        if (mImplicitCredentials && !mHaveCredentials)
        {
            acquireCredentials();
        }

        DWORD cbPacket          = mPkgInfo.cbMaxToken;

        SecBuffer ob            = {0};
        ob.BufferType           = SECBUFFER_TOKEN;
        ob.cbBuffer             = 0;
        ob.pvBuffer             = NULL;

        SecBufferDesc obd       = {0};
        obd.cBuffers            = 1;
        obd.ulVersion           = SECBUFFER_VERSION;
        obd.pBuffers            = &ob;
        
        SecBuffer ib[2]         = {0};
        ib[0].BufferType        = SECBUFFER_TOKEN;
        ib[0].cbBuffer          = static_cast<DWORD>(mReadBufferPos);
        ib[0].pvBuffer          = mReadBuffer;
        ib[1].BufferType        = SECBUFFER_EMPTY;
        ib[1].cbBuffer          = 0;
        ib[1].pvBuffer          = NULL;

        SecBufferDesc ibd       = {0};
        ibd.cBuffers            = 2;
        ibd.ulVersion           = SECBUFFER_VERSION;
        ibd.pBuffers            = ib;

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
            mHaveContext = true;
            break;
        default:
            break;
        }

        cbPacket = ob.cbBuffer;

        RCF_ASSERT(
            status != SEC_I_COMPLETE_AND_CONTINUE &&
            status != SEC_I_COMPLETE_NEEDED)
            (status);

        if (status == SEC_E_INCOMPLETE_MESSAGE)
        {
            // Not enough data.
            std::size_t readBufferPos = mReadBufferPos;
            resizeReadBuffer(mReadBufferPos + mReadAheadChunkSize);
            mReadBufferPos = readBufferPos;
            readBuffer();
            return false;
        }
        else if (ib[1].BufferType == SECBUFFER_EXTRA)
        {
            // We consider any extra data at this stage to be a protocol error.
            RCF_THROW(Exception(_RcfError_SspiHandshakeExtraData()));
        }

        trimReadBuffer();
        
        if (status == SEC_I_CONTINUE_NEEDED)
        {
            // Authorization ok so far, copy outbound data to write buffer.
            resizeWriteBuffer(ob.cbBuffer);
            memcpy(mWriteBuffer, ob.pvBuffer, ob.cbBuffer);
            getSft()->FreeContextBuffer(ob.pvBuffer);
        }
        else if (status == SEC_E_OK)
        {
            // Authorization ok, send last handshake block to the client.
            mContextState = AuthOk;
            RCF_ASSERT(cbPacket > 0);
            resizeWriteBuffer(cbPacket);
            memcpy(mWriteBuffer, ob.pvBuffer, ob.cbBuffer);
            getSft()->FreeContextBuffer(ob.pvBuffer);
        }
        else
        {
            // Authorization failed. Do nothing here, the connection will automatically close.
        }

        return true;
    }

    bool SspiClientFilter::doHandshakeSchannel()
    {
        // use the block in the read buffer to proceed through the handshake procedure

        // lazy acquiring of implicit credentials
        if (mImplicitCredentials && !mHaveCredentials)
        {
            acquireCredentials();
        }
     
        SecBuffer ob            = {0};
        ob.BufferType           = SECBUFFER_TOKEN;
        ob.cbBuffer             = 0;
        ob.pvBuffer             = NULL;

        SecBufferDesc obd       = {0};
        obd.cBuffers            = 1;
        obd.ulVersion           = SECBUFFER_VERSION;
        obd.pBuffers            = &ob;

        SecBuffer ib[2]         = {0};
        
        ib[0].BufferType        = SECBUFFER_TOKEN;
        ib[0].cbBuffer          = static_cast<DWORD>(mReadBufferPos);
        ib[0].pvBuffer          = mReadBuffer;

        ib[1].BufferType        = SECBUFFER_EMPTY;
        ib[1].cbBuffer          = 0;
        ib[1].pvBuffer          = NULL;

        SecBufferDesc ibd       = {0};
        ibd.cBuffers            = 2;
        ibd.ulVersion           = SECBUFFER_VERSION;
        ibd.pBuffers            = ib;

        const TCHAR *target     = mTarget.empty() ? RCF_T("") : mTarget.c_str();
        DWORD CtxtAttr          = 0;
        TimeStamp Expiration    = {0};

        SECURITY_STATUS status  = getSft()->InitializeSecurityContext(
            &mCredentials,
            mHaveContext ? &mContext : NULL,
            (TCHAR *) target,
            mContextRequirements,
            0,
            SECURITY_NATIVE_DREP,
            mHaveContext ? &ibd : NULL,
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

        if (status == SEC_E_INCOMPLETE_MESSAGE)
        {
            // Not enough data.
            std::size_t readBufferPos = mReadBufferPos;            
            resizeReadBuffer(mReadBufferPos + mReadAheadChunkSize);
            mReadBufferPos = readBufferPos;
            readBuffer();
            return false;
        }
        else if (ib[1].BufferType == SECBUFFER_EXTRA)
        {
            // We consider any extra data at this stage to be a protocol error.
            RCF_THROW(Exception(_RcfError_SspiHandshakeExtraData()));
        }

        trimReadBuffer();
            
        if (status == SEC_I_CONTINUE_NEEDED)
        {
            // Handshake OK so far.

            RCF_ASSERT(ob.cbBuffer);
            mContextState = AuthContinue;
            resizeWriteBuffer(ob.cbBuffer);
            memcpy(mWriteBuffer, ob.pvBuffer, ob.cbBuffer);
            getSft()->FreeContextBuffer(ob.pvBuffer);
            return true;
        }
        else if (status == SEC_E_OK)
        {
            // Handshake OK.

            // Extract the server certificate.
            PCCERT_CONTEXT pRemoteCertContext = NULL;

            status = getSft()->QueryContextAttributes(
                &mContext,
                SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                (PVOID)&pRemoteCertContext);

            mRemoteCert.reset( 
                pRemoteCertContext, 
                CertFreeCertificateContext);

            // If we have a custom validation callback, call it.
            if (mCertValidationCallback)
            {
                SchannelClientFilter & rThis = 
                    static_cast<SchannelClientFilter &>(*this);

                mCertValidationCallback(rThis);
            }

            // And now back to business.
            mContextState = AuthOkAck;
            resumeUserIo();
            return false;
        }
        else
        {
            RCF_THROW(Exception(_RcfError_SspiAuthFailClient(), status));
        }
    }

    void SspiFilter::setupCredentialsSchannel()
    {
        SCHANNEL_CRED schannelCred          = {0};       
        schannelCred.dwVersion              = SCHANNEL_CRED_VERSION;
        PCCERT_CONTEXT pCertContext         = NULL;
        if(mLocalCert)
        {
            pCertContext                    = mLocalCert.get();
            schannelCred.cCreds             = 1;
            schannelCred.paCred             = &pCertContext;
        }

        schannelCred.grbitEnabledProtocols  = mEnabledProtocols;

#if !defined(_MSC_VER) || _MSC_VER > 1200

        if (mServer)
        {
            schannelCred.dwFlags            = 0;
        }
        else if (mCertValidationCallback)
        {
            schannelCred.dwFlags            = SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION;
        }
        else
        {
            schannelCred.dwFlags            = SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_AUTO_CRED_VALIDATION;

        }

#endif

        SECURITY_STATUS status = getSft()->AcquireCredentialsHandle(
            NULL,
            UNISP_NAME,
            mServer ? SECPKG_CRED_INBOUND : SECPKG_CRED_OUTBOUND ,
            NULL,
            &schannelCred,
            NULL, 
            NULL,
            &mCredentials,
            NULL);

        if (status != SEC_E_OK)
        {
            RCF_THROW(
                FilterException(
                _RcfError_Sspi(), status, RcfSubsystem_Os,
                "AcquireCredentialsHandle() failed"))
                (mPkgInfo.Name)(status);
        }

        mHaveCredentials = true;
    }

    SchannelServerFilter::SchannelServerFilter(
        boost::shared_ptr<const CERT_CONTEXT> localCert,
        DWORD enabledProtocols,
        ULONG contextRequirements) :
            SspiServerFilter(UNISP_NAME, RCF_T(""), BoolSchannel)
    {
        mLocalCert = localCert;
        mContextRequirements = contextRequirements;
        mEnabledProtocols = enabledProtocols;
    }

    SchannelFilterFactory::SchannelFilterFactory(
        PCCERT_CONTEXT cert, 
        DWORD enabledProtocols,
        ULONG contextRequirements) :
            mLocalCert(cert, NullDeleter()),
            mContextRequirements(contextRequirements),
            mEnabledProtocols(enabledProtocols)
    {
    }

    FilterPtr SchannelFilterFactory::createFilter()
    {
        return FilterPtr( new SchannelServerFilter(
            mLocalCert, 
            mEnabledProtocols,
            mContextRequirements));
    }

    SchannelClientFilter::SchannelClientFilter(
        DWORD                   enabledProtocols,
        ULONG                   contextRequirements) :
            SspiClientFilter(
                RCF_T(""),
                Encryption, 
                contextRequirements, 
                UNISP_NAME, 
                RCF_T(""),
                BoolSchannel)
    {
        mEnabledProtocols = enabledProtocols;
    }

    void SchannelClientFilter::setManualCertValidation(
        CertValidationCallback certValidationCallback)
    {
        mCertValidationCallback = certValidationCallback;
    }

    void SchannelClientFilter::setAutoCertValidation(const tstring & serverName)
    {
        mCertValidationCallback = CertValidationCallback();
        mTarget = serverName;
    }

    void SchannelClientFilter::setClientCertificate(PCCERT_CONTEXT certContext)
    {
        mLocalCert.reset(certContext, NullDeleter());
    }

    PCCERT_CONTEXT SchannelClientFilter::getServerCertificate()
    {
        return mRemoteCert.get();
    }    

#if defined(__MINGW32__) && (__GNUC__ <= 3)

    // PfxCertificate and StoreCertificate have not been implemented for mingw gcc 3.4 and earlier.
    // A number of Cert* and PFX* functions seem to be missing from the mingw headers, and there
    // are issues with std::wstring.

#elif defined(_MSC_VER) && _MSC_VER < 1310

    // Similar issues with Visual C++ 6.
#else

    // Certificate utility classes.

    PfxCertificate::PfxCertificate(
        const std::string & pathToCert, 
        const RCF::tstring & password,
        const RCF::tstring & certName,
        DWORD dwFindType) : 
            mPfxStore(NULL),
            mpCertContext(RCF_DEFAULT_INIT)
    {
        std::size_t fileSize = static_cast<std::size_t>(RCF::fileSize(pathToCert));

        std::vector<char> buffer(fileSize);
        std::ifstream fin( pathToCert.c_str() , std::ios::binary);

        fin.read( 
            &buffer[0], 
            static_cast<std::streamsize>(buffer.size()) );
        
        std::size_t bytesRead = fin.gcount();
        fin.close();
        RCF_ASSERT(bytesRead == fileSize);

        CRYPT_DATA_BLOB blob = {0};
        blob.cbData   = static_cast<DWORD>(buffer.size());
        blob.pbData   = (BYTE*) &buffer[0];

        BOOL recognizedPFX = PFXIsPFXBlob(&blob);
        DWORD dwErr = GetLastError();
        RCF_VERIFY(recognizedPFX, RCF::Exception("PFXIsPFXBlob() failed"));

        std::wstring wPassword = util::toWstring(password);

        // For Windows 98, the flag CRYPT_MACHINE_KEYSET is not valid.
        mPfxStore = PFXImportCertStore(
            &blob, 
            wPassword.c_str(),
            CRYPT_MACHINE_KEYSET | CRYPT_EXPORTABLE);

        dwErr = GetLastError();

        RCF_VERIFY(mPfxStore, RCF::Exception("PFXImportCertStore() failed"));

        std::wstring wCertName = util::toWstring(certName);

        mpCertContext = CertFindCertificateInStore(
            mPfxStore, 
            X509_ASN_ENCODING, 
            0,
            dwFindType,
            wCertName.c_str(),
            NULL);

        dwErr = GetLastError();

        RCF_VERIFY(mpCertContext, RCF::Exception("CertFindCertificateInStore() failed"));
    }

    void PfxCertificate::addToStore(
        DWORD dwFlags, 
        const std::string & storeName)
    {
        std::wstring wStoreName(storeName.begin(), storeName.end());

        HCERTSTORE hCertStore = CertOpenStore(
            (LPCSTR) CERT_STORE_PROV_SYSTEM,
            X509_ASN_ENCODING,
            0,
            dwFlags,
            wStoreName.c_str());

        DWORD dwErr = GetLastError();

        RCF_VERIFY(
            hCertStore, 
            RCF::Exception(
                _RcfError_Sspi(), 
                dwErr, 
                RCF::RcfSubsystem_Os, 
                "CertOpenStore() failed"));

        BOOL ret = CertAddCertificateContextToStore(
            hCertStore,
            mpCertContext,
            CERT_STORE_ADD_USE_EXISTING,
            NULL);

        dwErr = GetLastError();

        RCF_VERIFY(
            ret, 
            RCF::Exception(
                _RcfError_Sspi(), 
                dwErr, 
                RCF::RcfSubsystem_Os, 
                "CertAddCertificateContextToStore() failed"));

        CertCloseStore(hCertStore, 0);
    }

    PfxCertificate::~PfxCertificate()
    {
        CertFreeCertificateContext(mpCertContext);
        CertCloseStore(mPfxStore, 0);
    }

    PCCERT_CONTEXT PfxCertificate::getCertContext()
    {
        return mpCertContext;
    }

    StoreCertificate::StoreCertificate(
        DWORD dwStoreFlags, 
        const std::string & storeName,
        const tstring & certName,
        DWORD dwFindType) :
            mStore(0),
            mpCertContext(NULL)
    {
        std::wstring wStoreName(storeName.begin(), storeName.end());

        mStore = CertOpenStore(
            (LPCSTR) CERT_STORE_PROV_SYSTEM,
            X509_ASN_ENCODING,
            0,
            dwStoreFlags,
            &wStoreName[0]);

        DWORD dwErr = GetLastError();
    
        RCF_VERIFY(
            mStore, 
            RCF::Exception(
                _RcfError_Sspi(), 
                dwErr, 
                RCF::RcfSubsystem_Os, 
                "CertOpenStore() failed"));

        std::wstring wCertName(certName.begin(), certName.end());

        mpCertContext = CertFindCertificateInStore(
            mStore, 
            X509_ASN_ENCODING, 
            0,
            dwFindType,
            wCertName.c_str(),
            NULL);

        dwErr = GetLastError();

        // It's OK if mpCertContext is NULL - user will have to take appropriate action.
    }

    void StoreCertificate::removeFromStore()
    {
        if (mpCertContext)
        {
            BOOL ret = CertDeleteCertificateFromStore(mpCertContext);
            DWORD dwErr = GetLastError();

            RCF_VERIFY(
                ret, 
                RCF::Exception(
                    _RcfError_Sspi(), 
                    dwErr, 
                    RCF::RcfSubsystem_Os, 
                    "CertDeleteCertificateFromStore() failed"));

            mpCertContext = NULL;
        }
    }

    StoreCertificate::~StoreCertificate()
    {
        CertFreeCertificateContext(mpCertContext);
        CertCloseStore(mStore, 0);
    }

    PCCERT_CONTEXT StoreCertificate::getCertContext()
    {
        return mpCertContext;
    }

#endif

} // namespace RCF
