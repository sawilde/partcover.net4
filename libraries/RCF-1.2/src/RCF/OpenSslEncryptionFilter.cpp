
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/OpenSslEncryptionFilter.hpp>

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/Tools.hpp>
#include <RCF/UsingOpenSsl.hpp>

namespace RCF {
/*
    void printErrors(SSL * pSsl, int result)
    {
        if (result <= 0)
        {
            int error = SSL_get_error(pSsl, result);

            switch(error)
            {
            case SSL_ERROR_ZERO_RETURN:
            case SSL_ERROR_NONE: 
            case SSL_ERROR_WANT_READ :

                break;

            default :
                {

                    char buffer[256];

                    while (error != 0)
                    {
                        ERR_error_string_n(error, buffer, sizeof(buffer));

                        std::cout << "Error: " << error << " - " << buffer << std::endl;

                        error = ERR_get_error();

                        printf("Error = %s\n",ERR_reason_error_string(error));
                    }
                }
                break;
            }
        }
    }
*/
    class OpenSslEncryptionFilterImpl
    {
    public:
        OpenSslEncryptionFilterImpl(
            OpenSslEncryptionFilter &   openSslEncryptionFilter,
            SslRole                     sslRole,
            const std::string &         certificateFile,
            const std::string &         certificateFilePassword,
            const std::string &         caCertificate,
            const std::string &         ciphers,
            VerifyFunctor               verifyFunctor,
            unsigned int                bioBufferSize);

        void reset();

        void read(
            const ByteBuffer &          byteBuffer, 
            std::size_t                 bytesRequested);

        void write(
            const std::vector<ByteBuffer> &byteBuffers);

        void onReadWriteCompleted(
            std::size_t                 bytesTransferred);

        SSL * getSSL();
        SSL_CTX * getCTX();

    private:
        void init();

        bool loadCertificate(
            boost::shared_ptr<SSL_CTX>  ctx,
            const std::string &         file,
            const std::string &         password);

        bool loadCaCertificate(
            boost::shared_ptr<SSL_CTX>  ctx,
            const std::string &         file);

        void readWrite();
        void transferData();
        void onDataTransferred(std::size_t bytesTransferred);
        void retryReadWrite();

        std::size_t                     mPos;
        std::size_t                     mReadRequested;
        ByteBuffer                      mPreByteBuffer;
        ByteBuffer                      mPostByteBuffer;
        std::vector<ByteBuffer>         mByteBuffers;

        boost::shared_ptr< std::vector<char> > 
                                        mVecPtr;

        enum IoState
        {
            Ready,
            Reading,
            Writing
        };

        SslRole                         mSslRole;
        std::string                     mCertificateFile;
        std::string                     mCertificateFilePassword;
        std::string                     mCaCertificate;
        std::string                     mCiphers;
        IoState                         mPreState;
        IoState                         mPostState;
        bool                            mRetry;
        bool                            mHandshakeOk;
        char *                          mPreBuffer;
        char *                          mPostBuffer;
        std::size_t                     mPostBufferLen;
        std::size_t                     mPostBufferRequested;
        VerifyFunctor                   mVerifyFunctor;
        int                             mErr;

        // OpenSSL members
        // NB: using shared_ptr instead of auto_ptr, since we need custom deleters
        boost::shared_ptr<SSL_CTX>      mSslCtx;
        boost::shared_ptr<SSL>          mSsl;
        boost::shared_ptr<BIO>          mBio;
        boost::shared_ptr<BIO>          mIoBio;
        boost::shared_ptr<BIO>          mSslBio;

        unsigned int                    mBioBufferSize;

        OpenSslEncryptionFilter &       mOpenSslEncryptionFilter;
    };

    const FilterDescription & OpenSslEncryptionFilter::sGetFilterDescription()
    {
        return *spFilterDescription;
    }

    const FilterDescription *OpenSslEncryptionFilter::spFilterDescription = NULL;

    static void initOpenSslEncryptionFilterDescription()
    {
        RCF_ASSERT(!OpenSslEncryptionFilter::spFilterDescription);
        OpenSslEncryptionFilter::spFilterDescription =
            new FilterDescription(
                "OpenSSL encryption filter",
                RcfFilter_OpenSsl,
                true);
    }

    static void deinitOpenSslEncryptionFilterDescription()
    {
        delete OpenSslEncryptionFilter::spFilterDescription;
        OpenSslEncryptionFilter::spFilterDescription = NULL;
    }

    RCF_ON_INIT_DEINIT(
        initOpenSslEncryptionFilterDescription(),
        deinitOpenSslEncryptionFilterDescription())

#ifdef _MSC_VER
#pragma warning( push )
// warning C4355: 'this' : used in base member initializer list
#pragma warning( disable : 4355 ) 
#endif

    OpenSslEncryptionFilter::OpenSslEncryptionFilter(
        const std::string &certificateFile,
        const std::string &certificateFilePassword,
        const std::string &caCertificate,
        const std::string &ciphers,
        VerifyFunctor verifyFunctor,
        SslRole sslRole,
        unsigned int bioBufferSize) :
            mImplPtr( new OpenSslEncryptionFilterImpl(
                *this,
                sslRole,
                certificateFile,
                certificateFilePassword,
                caCertificate,
                ciphers,
                verifyFunctor,
                bioBufferSize) )
    {}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    void OpenSslEncryptionFilter::reset()
    {
        mImplPtr->reset();
    }

    void OpenSslEncryptionFilter::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        mImplPtr->read(byteBuffer, bytesRequested);
    }

    void OpenSslEncryptionFilter::write(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        mImplPtr->write(byteBuffers);
    }

    void OpenSslEncryptionFilter::onReadCompleted(
        const ByteBuffer &byteBuffer)
    {
        mImplPtr->onReadWriteCompleted(byteBuffer.getLength());
    }

    void OpenSslEncryptionFilter::onWriteCompleted(
        std::size_t bytesTransferred)
    {
        mImplPtr->onReadWriteCompleted(bytesTransferred);
    }

    SSL *OpenSslEncryptionFilter::getSSL()
    {
        return mImplPtr->getSSL();
    }

    SSL_CTX *OpenSslEncryptionFilter::getCTX()
    {
        return mImplPtr->getCTX();
    }

    SSL *OpenSslEncryptionFilterImpl::getSSL()
    {
        return mSsl.get();
    }

    SSL_CTX *OpenSslEncryptionFilterImpl::getCTX()
    {
        return mSslCtx.get();
    }

    const FilterDescription & OpenSslEncryptionFilter::getFilterDescription() const
    {
        return sGetFilterDescription();
    }

    OpenSslEncryptionFilterImpl::OpenSslEncryptionFilterImpl(
        OpenSslEncryptionFilter &openSslEncryptionFilter,
        SslRole sslRole,
        const std::string &certificateFile,
        const std::string &certificateFilePassword,
        const std::string &caCertificate,
        const std::string &ciphers,
        VerifyFunctor verifyFunctor,
        unsigned int bioBufferSize) :
            mSslRole(sslRole),
            mCertificateFile(certificateFile),
            mCertificateFilePassword(certificateFilePassword),
            mCaCertificate(caCertificate),
            mCiphers(ciphers),
            mPreBuffer(RCF_DEFAULT_INIT),
            mPostBuffer(RCF_DEFAULT_INIT),
            mPostBufferLen(RCF_DEFAULT_INIT),
            mPostBufferRequested(RCF_DEFAULT_INIT),
            mPreState(Ready),
            mPostState(Ready),
            mBioBufferSize(bioBufferSize),
            mRetry(RCF_DEFAULT_INIT),
            mHandshakeOk(RCF_DEFAULT_INIT),
            mVerifyFunctor(verifyFunctor),
            mErr(RCF_DEFAULT_INIT),
            mOpenSslEncryptionFilter(openSslEncryptionFilter)
    {
        init();
    }

    void OpenSslEncryptionFilterImpl::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        if (byteBuffer.isEmpty() && bytesRequested == 0)
        {
            // If we have any data that hasn't been read, then issue a read 
            // completion, otherwise clear our buffers and issue a zero byte 
            // read on the next filter.

            if (BIO_ctrl_pending(mIoBio.get()) == 0)
            {
                RCF_ASSERT(mPreState == Ready)(mPreState);
                mPreState = Reading;
                mReadRequested = bytesRequested;
                mOpenSslEncryptionFilter.mpPostFilter->read(ByteBuffer(), 0);
            }
            else
            {
                mOpenSslEncryptionFilter.mpPreFilter->onReadCompleted(byteBuffer);
            }
        }
        else
        {
            RCF_ASSERT(mPreState == Ready || mPreState == Reading)(mPreState);            
            mPreState = Reading;
            if (byteBuffer.getLength() == 0)
            {
                if (mVecPtr.get() == NULL && !mVecPtr.unique())
                {
                    mVecPtr.reset(new std::vector<char>(bytesRequested));
                }
                mVecPtr->resize(bytesRequested);
                mPreByteBuffer = ByteBuffer(mVecPtr);
            }
            else
            {
                mPreByteBuffer = byteBuffer;
            }
            mReadRequested = bytesRequested;
            readWrite();
        }
    }

    void OpenSslEncryptionFilterImpl::write(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        RCF_ASSERT(mPreState == Ready)(mPreState);
        mPreState = Writing;
        mPreByteBuffer = byteBuffers.front();
        readWrite();
    }

    void OpenSslEncryptionFilterImpl::onReadWriteCompleted(
        std::size_t bytesTransferred)
    {
        mPostByteBuffer.clear();

        if (bytesTransferred == 0 && mReadRequested == 0 && mPreState == Reading)
        {
            mOpenSslEncryptionFilter.mpPreFilter->onReadCompleted(ByteBuffer());
        }
        else
        {

            // complete the data transfer
            onDataTransferred(bytesTransferred);

            if (mRetry)
            {
                retryReadWrite();
            }
            else
            {
                if (mPreState == Writing && BIO_ctrl_pending(mIoBio.get()) > 0)
                {
                    transferData();
                }
                else
                {
                    IoState state = mPreState;
                    mPreState = Ready;
                    if (state == Reading)
                    {
                        mPreByteBuffer = ByteBuffer(mPreByteBuffer, 0, mPos);

                        mOpenSslEncryptionFilter.mpPreFilter->onReadCompleted(
                            mPreByteBuffer);
                    }
                    else
                    {
                        mPreByteBuffer.clear();

                        mOpenSslEncryptionFilter.mpPreFilter->onWriteCompleted(
                            mPos);
                    }
                }
            }
        }
    }


    void OpenSslEncryptionFilterImpl::readWrite()
    {
        // set input parameters
        mRetry = true;
        mErr = 0;
        mPos = 0;
        retryReadWrite();
    }

    void OpenSslEncryptionFilterImpl::retryReadWrite()
    {
        RCF_ASSERT(mPreState == Reading || mPreState == Writing)(mPreState);

        int sslState = SSL_get_state(mSsl.get());
        if (!mHandshakeOk && sslState == SSL_ST_OK)
        {
            mHandshakeOk = true;
            {
                if (!mCaCertificate.empty())
                {
                    // verify the peers certificate against our CA's
                    int ret = SSL_get_verify_result(mSsl.get());
                    bool verifyOk = (ret == X509_V_OK);
                    boost::shared_ptr<X509> peerCert(
                        SSL_get_peer_certificate(mSsl.get()),
                        X509_free);
                    if (!peerCert || !verifyOk)
                    {
                        if (!mVerifyFunctor || !mVerifyFunctor(mOpenSslEncryptionFilter))
                        {
                            RCF_THROW(
                                Exception(
                                    _RcfError_SslCertVerification(), 
                                    ret, 
                                    RcfSubsystem_OpenSsl, 
                                    "Subsystem error (if any) is relative to SSL_get_verify_result()."));
                        }
                    }
                }
            }
        }
        else if (mHandshakeOk && sslState != SSL_ST_OK)
        {
            mHandshakeOk = false;
        }
        

        int bioRet = (mPreState == Reading) ?
            BIO_read(mSslBio.get(), mPreByteBuffer.getPtr(), static_cast<int>(mReadRequested)) :
            BIO_write(mSslBio.get(), mPreByteBuffer.getPtr(), static_cast<int>(mPreByteBuffer.getLength()));


        RCF_ASSERT(
            -1 <= bioRet && bioRet <= static_cast<int>(mPreByteBuffer.getLength()))
            (bioRet)(mPreByteBuffer.getLength());

        if (bioRet == -1 && BIO_should_retry(mSslBio.get()))
        {
            // initiate io requests on underlying filters
            mRetry = true;
            transferData();
        }
        else if (0 < bioRet && bioRet <= static_cast<int>(mPreByteBuffer.getLength()))
        {
            mRetry = false;
            mPos += bioRet;
            if (mPreState == Writing)
            {
                // TODO: maybe this is not always true
                RCF_ASSERT(BIO_ctrl_pending(mIoBio.get()) > 0);
                transferData();
            }
            else
            {
                mPreState = Ready;
                
                mPreByteBuffer = ByteBuffer(mPreByteBuffer, 0, mPos);

                mOpenSslEncryptionFilter.mpPreFilter->onReadCompleted(
                    mPreByteBuffer);
            }
        }
        else
        {
            mErr = -1;
        }
    }

    void OpenSslEncryptionFilterImpl::transferData()
    {
        if (BIO_ctrl_pending(mIoBio.get()) == 0)
        {
            // move data from network to mIoBio
            mPostState = Reading;
            mPostBufferRequested =
                static_cast<int>(BIO_ctrl_get_read_request(mIoBio.get()));

            mPostBufferLen = BIO_nwrite0(mIoBio.get(), &mPostBuffer);
           
            RCF_ASSERT(
                mPostBufferRequested <= mPostBufferLen)
                (mPostBufferRequested)(mPostBufferLen);

            // NB: completion routine will call BIO_nwrite(io_bio, len)
            mPostByteBuffer = ByteBuffer(mPostBuffer, mPostBufferLen);

            mOpenSslEncryptionFilter.mpPostFilter->read(
                mPostByteBuffer,
                mPostBufferRequested);
        }
        else
        {
            // move data from mIoBio to network
            mPostState = Writing;
            mPostBufferRequested = static_cast<int>(BIO_ctrl_pending(mIoBio.get()));
            mPostBufferLen = BIO_nread0(mIoBio.get(), &mPostBuffer);
            // NB: completion routine will call BIO_nread(mIoBio, postBufferLen)
            mByteBuffers.resize(0);
            mByteBuffers.push_back( ByteBuffer(mPostBuffer, mPostBufferLen));
            mOpenSslEncryptionFilter.mpPostFilter->write(mByteBuffers);
            mByteBuffers.resize(0);
        }
    }

    void OpenSslEncryptionFilterImpl::onDataTransferred(std::size_t bytesTransferred)
    {
        // complete a data transfer, in the direction that was requested

        // TODO: assert that, on read, data was actually transferred into postBuffer
        // and not somewhere else

        RCF_ASSERT(bytesTransferred > 0);
        RCF_ASSERT(
            (mPostState == Reading && bytesTransferred <= mPostBufferRequested) ||
            (mPostState == Writing && bytesTransferred <= mPostBufferLen))
            (mPostState)(bytesTransferred)(mPostBufferRequested)(mPostBufferLen);

        if (mPostState == Reading)
        {
            // return value not documented
            BIO_nwrite(
                mIoBio.get(),
                &mPostBuffer,
                static_cast<int>(bytesTransferred));

            mPostBuffer = 0;
            mPostBufferLen = 0;
            mPostState = Ready;
        }
        else if (mPostState == Writing)
        {
            // return value not documented
            BIO_nread(
                mIoBio.get(),
                &mPostBuffer,
                static_cast<int>(bytesTransferred));

            mPostBuffer = 0;
            mPostBufferLen = 0;
            mPostState = Ready;
        }
    }

    void OpenSslEncryptionFilterImpl::reset()
    {
        init();
    }

    void OpenSslEncryptionFilterImpl::init()
    {
        // TODO: sort out any OpenSSL-dependent order of destruction issues

        mSslBio = boost::shared_ptr<BIO>(
            BIO_new(BIO_f_ssl()),
            BIO_free);

        mSslCtx = boost::shared_ptr<SSL_CTX>(
            SSL_CTX_new(SSLv23_method()),
            SSL_CTX_free);

        RCF_ASSERT(mSslRole == SslServer || mSslRole == SslClient)(mSslRole);
        
        if (!mCertificateFile.empty())
        {
            loadCertificate(mSslCtx, mCertificateFile, mCertificateFilePassword);
        }

        if (!mCaCertificate.empty())
        {
            loadCaCertificate(mSslCtx, mCaCertificate);
        }

        mSsl = boost::shared_ptr<SSL>(
            SSL_new(mSslCtx.get()),
            SSL_free);

        if (mSslRole == SslServer && !mCaCertificate.empty())
        {
            SSL_set_verify(mSsl.get(), SSL_VERIFY_PEER, NULL);
        }

        BIO *bio_temp = NULL;
        BIO *io_bio_temp = NULL;
        BIO_new_bio_pair(&bio_temp, mBioBufferSize, &io_bio_temp, mBioBufferSize);
        mBio = boost::shared_ptr<BIO>(
            bio_temp,
            BIO_free);
        mIoBio = boost::shared_ptr<BIO>(
            io_bio_temp,
            BIO_free);

        RCF_ASSERT(mSslRole == SslServer || mSslRole == SslClient)(mSslRole);
        mSslRole == SslServer ?
            SSL_set_accept_state(mSsl.get()) :
            SSL_set_connect_state(mSsl.get());

        SSL_set_bio(mSsl.get(), mBio.get(), mBio.get());
        BIO_set_ssl(mSslBio.get(), mSsl.get(), BIO_NOCLOSE);

        if (
            mSslCtx.get() == NULL ||
            mSsl.get() == NULL ||
            mBio.get() == NULL ||
            mIoBio.get() == NULL)
        {
            std::string opensslErrors = getOpenSslErrors();
            RCF_THROW(Exception(_RcfError_OpenSslFilterInit(opensslErrors)))
                (opensslErrors)(mCertificateFile);
        }

    }

    bool OpenSslEncryptionFilterImpl::loadCertificate(
        boost::shared_ptr<SSL_CTX> ctx,
        const std::string &file,
        const std::string &password)
    {
        RCF_ASSERT(ctx.get());
        if (1 == SSL_CTX_use_certificate_chain_file(ctx.get(), file.c_str()))
        {
            boost::shared_ptr<BIO> bio(
                BIO_new( BIO_s_file() ),
                BIO_free );
            if (bio.get())
            {
                if (1 == BIO_read_filename(bio.get(), file.c_str()))
                {
                    boost::shared_ptr<EVP_PKEY> evp(
                        PEM_read_bio_PrivateKey(
                            bio.get(),
                            NULL,
                            NULL,
                            (void *) password.c_str()),
                        EVP_PKEY_free);
                    if (evp.get())
                    {
                        if (1 == SSL_CTX_use_PrivateKey(ctx.get(), evp.get()))
                        {
                            return true;
                        }
                    }
                }
            }
        }
        std::string opensslErrors = getOpenSslErrors();
        RCF_THROW(
            Exception(_RcfError_OpenSslLoadCert(file, opensslErrors)))
            (opensslErrors)(file)(password);
        return false;
    }

    bool OpenSslEncryptionFilterImpl::loadCaCertificate(
        boost::shared_ptr<SSL_CTX> ctx,
        const std::string &file)
    {
        RCF_ASSERT(ctx.get());

        if (SSL_CTX_load_verify_locations(ctx.get(), file.c_str(), NULL) != 1)
        {
            std::string opensslErrors = getOpenSslErrors();
            RCF_THROW(
                Exception(_RcfError_OpenSslLoadCert(file, opensslErrors)))
                (opensslErrors)(file);
        }
        return true;
    }

    OpenSslEncryptionFilterFactory::OpenSslEncryptionFilterFactory(
        const std::string &certificateFile,
        const std::string &certificateFilePassword,
        const std::string &caCertificate,
        const std::string &ciphers,
        VerifyFunctor verifyFunctor,
        bool serverRole) :
            mCertificateFile(certificateFile),
            mCertificateFilePassword(certificateFilePassword),
            mCaCertificate(caCertificate),
            mCiphers(ciphers),
            mVerifyFunctor(verifyFunctor),
            mRole(serverRole ? SslServer : SslClient)
    {}

    FilterPtr OpenSslEncryptionFilterFactory::createFilter()
    {
        return FilterPtr( new OpenSslEncryptionFilter(
            mCertificateFile,
            mCertificateFilePassword,
            mCaCertificate,
            mCiphers,
            mVerifyFunctor,
            mRole));
    }

    const FilterDescription & OpenSslEncryptionFilterFactory::getFilterDescription()
    {
        return OpenSslEncryptionFilter::sGetFilterDescription();
    }

} // namespace RCF
