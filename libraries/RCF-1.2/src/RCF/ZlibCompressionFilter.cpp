
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/ZlibCompressionFilter.hpp>

#include "zlib.h"

#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/RecursionLimiter.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    class ZlibCompressionReadFilter
    {
    public:
        ZlibCompressionReadFilter(
            ZlibCompressionFilterBase &filter,
            int bufferSize,
            bool serverSide);

        ~ZlibCompressionReadFilter();
        void reset();
        void read(const ByteBuffer &byteBuffer, std::size_t bytesRequested);
        void onReadCompleted(const ByteBuffer &byteBuffer);

        void clearPreBuffer();

    private:
        void resetDecompressionState();
        bool decompress();

        ZlibCompressionFilterBase & mFilter;
        z_stream                    mDstream;
        std::size_t                 mBytesRequested;
        ByteBuffer                  mPreBuffer;
        ByteBuffer                  mPostBuffer;
        int                         mZerr;
        bool                        mDecompressionStateInited;

        ByteBuffer                              mOrigBuffer;
        boost::shared_ptr<std::vector<char> >   mVecPtr;
    };

    class ZlibCompressionWriteFilter
    {
    public:
        ZlibCompressionWriteFilter(
            ZlibCompressionFilterBase &filter,
            int bufferSize,
            bool stateful);

        ~ZlibCompressionWriteFilter();
        void reset();
        void write(const std::vector<ByteBuffer> &byteBuffers);
        void onWriteCompleted(std::size_t bytesTransferred);

    private:
        void resetCompressionState();
        void compress();

        ZlibCompressionFilterBase & mFilter;
        z_stream                    mCstream;
        std::size_t                 mTotalBytesIn;
        std::size_t                 mTotalBytesOut;
        int                         mZerr;
        bool                        mCompressionStateInited;
        const bool                  mStateful;

        std::vector<ByteBuffer>     mPostBuffers;
        std::vector<ByteBuffer>     mPreBuffers;

        boost::shared_ptr<std::vector<char> > mVecPtr;
    };

    ZlibCompressionReadFilter::ZlibCompressionReadFilter(
        ZlibCompressionFilterBase &filter,
        int bufferSize,
        bool serverSide) :
            mFilter(filter),
            mDstream(),
            mBytesRequested(RCF_DEFAULT_INIT),
            mZerr(Z_OK),
            mDecompressionStateInited(RCF_DEFAULT_INIT)
    {
        RCF_UNUSED_VARIABLE(serverSide);

        memset(&mDstream, 0, sizeof(mDstream));

        // TODO: buffer size
        RCF_UNUSED_VARIABLE(bufferSize);
        resetDecompressionState();
    }

    ZlibCompressionReadFilter::~ZlibCompressionReadFilter()
    {
        RCF_DTOR_BEGIN
            if (mDecompressionStateInited)
            {
                mZerr = inflateEnd(&mDstream);
                RCF_VERIFY(
                    mZerr == Z_OK,
                    FilterException(
                        _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                        "inflateEnd() failed"))(mZerr);
                mDecompressionStateInited = false;
            }
        RCF_DTOR_END
    }

    void ZlibCompressionReadFilter::clearPreBuffer()
    {
        mPreBuffer.clear();
    }

    void ZlibCompressionReadFilter::reset()
    {
        resetDecompressionState();
    }

    void ZlibCompressionReadFilter::resetDecompressionState()
    {
        if (mDecompressionStateInited)
        {
            mZerr = inflateEnd(&mDstream);

            RCF_VERIFY(
                mZerr == Z_OK,
                FilterException(
                    _RcfError_Zlib(),
                    mZerr,
                    RcfSubsystem_Zlib,
                    "inflateEnd() failed"))
                (mZerr);

            mDecompressionStateInited = false;
        }
        mDstream.zalloc = NULL;
        mDstream.zfree = NULL;
        mDstream.opaque = NULL;
        mZerr = inflateInit(&mDstream);
        RCF_VERIFY(
            mZerr == Z_OK,
            FilterException(
                _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                "inflateInit() failed"))(mZerr);
        mDecompressionStateInited = true;
    }

    // TODO: do the right thing with the byteBuffer parameter
    void ZlibCompressionReadFilter::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        if (byteBuffer.isEmpty() && bytesRequested == 0)
        {
            if (mPostBuffer.getLength() > 0)
            {
                mFilter.getPreFilter().onReadCompleted(byteBuffer);
            }
            else
            {
                mBytesRequested = bytesRequested;
                mFilter.getPostFilter().read(ByteBuffer(), 0);
            }
        }
        else
        {

            mOrigBuffer = ByteBuffer(
                byteBuffer,
                0,
                RCF_MIN(byteBuffer.getLength(), bytesRequested));

            if (mPostBuffer.getLength() > 0)
            {
                onReadCompleted(mPostBuffer);
            }
            else
            {
                mPostBuffer = ByteBuffer();

                mBytesRequested = bytesRequested;
                // TODO:  a dedicated buffer for reading, so we can read a lot at a time
                // TODO: buffer size
                std::size_t bytesToRead = 4096;
                mFilter.getPostFilter().read(ByteBuffer(), bytesToRead);
            }
        }
    }

    void ZlibCompressionReadFilter::onReadCompleted(
        const ByteBuffer &byteBuffer)
    {
        if (mBytesRequested == 0)
        {
            RCF_ASSERT(byteBuffer.isEmpty());
            mFilter.getPreFilter().onReadCompleted(ByteBuffer());
        }
        else 
        {
            if (mOrigBuffer.getLength() > 0)
            {
                mPreBuffer = mOrigBuffer;
            }
            else
            {
                if (mVecPtr.get() == NULL || !mVecPtr.unique())
                {
                    mVecPtr.reset( new std::vector<char>());
                }
                mVecPtr->resize(mBytesRequested);
                mPreBuffer = ByteBuffer(mVecPtr);
                mOrigBuffer = mPreBuffer;
            }
            mPostBuffer = byteBuffer;
            if (decompress())
            {
                mOrigBuffer.clear();
                mFilter.getPreFilter().onReadCompleted(mPreBuffer);
            }
            else
            {
                mPreBuffer.clear();
                read(mOrigBuffer, mBytesRequested);
            }
        }
    }

    bool ZlibCompressionReadFilter::decompress()
    {
        mDstream.next_in = (Bytef*) mPostBuffer.getPtr();
        mDstream.avail_in = static_cast<uInt>(mPostBuffer.getLength());
        mDstream.next_out = (Bytef*) mPreBuffer.getPtr();
        mDstream.avail_out = static_cast<uInt>(mPreBuffer.getLength());
        mZerr = inflate(&mDstream, Z_SYNC_FLUSH);
        RCF_VERIFY(
            mZerr == Z_OK || mZerr == Z_STREAM_END || mZerr == Z_BUF_ERROR,
            FilterException(
                _RcfError_ZlibInflate(), mZerr, RcfSubsystem_Zlib,
                "inflate() failed"))
            (mZerr)(mPreBuffer.getLength())(mPostBuffer.getLength());
        if (mZerr == Z_STREAM_END)
        {
            resetDecompressionState();
        }

        mPreBuffer = ByteBuffer(
            mPreBuffer,
            0,
            mPreBuffer.getLength() - mDstream.avail_out);

        mPostBuffer = ByteBuffer(
            mPostBuffer,
            mPostBuffer.getLength() - mDstream.avail_in);

        if (mPostBuffer.getLength() == 0)
        {
            mPostBuffer.clear();
        }

        return mPreBuffer.getLength() > 0;
    }

    ZlibCompressionWriteFilter::ZlibCompressionWriteFilter(
        ZlibCompressionFilterBase &filter,
        int bufferSize,
        bool stateful) :
            mFilter(filter),
            mCstream(),
            mTotalBytesIn(RCF_DEFAULT_INIT),
            mTotalBytesOut(RCF_DEFAULT_INIT),
            mZerr(Z_OK),
            mCompressionStateInited(RCF_DEFAULT_INIT),
            mStateful(stateful)
    {
        memset(&mCstream, 0, sizeof(mCstream));

        // TODO: buffer size
        RCF_UNUSED_VARIABLE(bufferSize);
    }

    ZlibCompressionWriteFilter::~ZlibCompressionWriteFilter()
    {
        RCF_DTOR_BEGIN
            if (mCompressionStateInited)
            {
                mZerr = deflateEnd(&mCstream);
                RCF_VERIFY(
                    mZerr == Z_OK || mZerr == Z_DATA_ERROR,
                    FilterException(
                        _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                        "deflateEnd() failed"))(mZerr)(mStateful);
                mCompressionStateInited = false;
            }
        RCF_DTOR_END
    }

    void ZlibCompressionWriteFilter::reset()
    {
        resetCompressionState();
    }

    void ZlibCompressionWriteFilter::resetCompressionState()
    {
        if (mCompressionStateInited)
        {
            mZerr = deflateEnd(&mCstream);
            RCF_VERIFY(
                mZerr == Z_OK || mZerr == Z_DATA_ERROR,
                FilterException(
                    _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                    "deflateEnd() failed"))(mZerr);
            mCompressionStateInited = false;
        }
        mCstream.zalloc = NULL;
        mCstream.zfree = NULL;
        mCstream.opaque = NULL;
        mZerr = deflateInit(&mCstream, Z_DEFAULT_COMPRESSION);
        RCF_VERIFY(
            mZerr == Z_OK,
            FilterException(
                _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                "deflateInit() failed"))(mZerr);
        mCompressionStateInited = true;
    }

    void ZlibCompressionWriteFilter::write(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        if (mStateful == false || mCompressionStateInited == false)
        {
            resetCompressionState();
        }
        mPreBuffers.resize(0);

        std::copy(
            byteBuffers.begin(),
            byteBuffers.end(),
            std::back_inserter(mPreBuffers));

        compress();
        mFilter.getPostFilter().write(mPostBuffers);
    }

    void ZlibCompressionWriteFilter::onWriteCompleted(
        std::size_t bytesTransferred)
    {
        // 1. if partial buffer was written -> write remaining part of buffer
        // 2. if whole buffer was written -> check if any more compression or writing is needed
        // 3. if no more compression or writing needed, notify previous filter of completion

        RCF_ASSERT(
            bytesTransferred <= lengthByteBuffers(mPostBuffers))
            (bytesTransferred)(lengthByteBuffers(mPostBuffers));

        if (bytesTransferred < lengthByteBuffers(mPostBuffers))
        {
            // TODO: optimize
            std::vector<ByteBuffer> slicedBuffers;
            sliceByteBuffers(slicedBuffers, mPostBuffers, bytesTransferred);
            mPostBuffers = slicedBuffers;
            mFilter.getPostFilter().write(mPostBuffers);
        }
        else
        {
            mPreBuffers.resize(0);
            mPostBuffers.resize(0);
            mFilter.getPreFilter().onWriteCompleted(mTotalBytesIn);
        }
    }

    void ZlibCompressionWriteFilter::compress()
    {
        mPostBuffers.resize(0);

        // TODO: buffer size
        std::size_t bufferSize = 2*(lengthByteBuffers(mPreBuffers)+7+7);
        std::size_t leftMargin = mPreBuffers.front().getLeftMargin();

        if (mVecPtr.get() == NULL || !mVecPtr.unique())
        {
            mVecPtr.reset( new std::vector<char>());
        }
        mVecPtr->resize(leftMargin + bufferSize);

        if (leftMargin > 0)
        {
            mPostBuffers.push_back(
                ByteBuffer(
                    &mVecPtr->front()+leftMargin,
                    mVecPtr->size()-leftMargin,
                    leftMargin,
                    mVecPtr));
        }
        else
        {
            mPostBuffers.push_back(
                ByteBuffer(
                    &mVecPtr->front(),
                    mVecPtr->size(),
                    mVecPtr));
        }

        ByteBuffer &outBuffer = mPostBuffers.back();
        std::size_t outPos = 0;
        std::size_t outRemaining = outBuffer.getLength() - outPos;
        mTotalBytesIn = 0;
        mTotalBytesOut = 0;

        for (std::size_t i=0; i<mPreBuffers.size(); ++i)
        {
            RCF_ASSERT(
                outPos < outBuffer.getLength())
                (outPos)(outBuffer.getLength());

            ByteBuffer &inBuffer = mPreBuffers[i];
            mCstream.next_in = (Bytef*) inBuffer.getPtr();
            mCstream.avail_in = static_cast<uInt>(inBuffer.getLength());
            mCstream.next_out = (Bytef*) &outBuffer.getPtr()[outPos];
            mCstream.avail_out = static_cast<uInt>(outRemaining);

            mZerr = (i<mPreBuffers.size()-1) ?
                deflate(&mCstream, Z_NO_FLUSH) :
                deflate(&mCstream, Z_SYNC_FLUSH);

            RCF_VERIFY(
                mZerr == Z_OK || mZerr == Z_BUF_ERROR,
                FilterException(
                    _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                    "deflate() failed"))
                (mZerr)(inBuffer.getLength())(outBuffer.getLength());

            RCF_ASSERT(mCstream.avail_out >= 0)(mCstream.avail_out);

            std::size_t bytesIn = inBuffer.getLength() - mCstream.avail_in;
            std::size_t bytesOut = outRemaining - mCstream.avail_out;
            mTotalBytesIn += bytesIn;
            mTotalBytesOut += bytesOut;
            outPos += bytesOut;
            outRemaining -= bytesOut;
        }

        if (!mStateful)
        {
            mCstream.next_in = NULL;
            mCstream.avail_in = 0;
            mCstream.next_out = (Bytef*) &outBuffer.getPtr()[outPos];
            mCstream.avail_out = static_cast<uInt>(outRemaining);

            mZerr = deflate(&mCstream, Z_FINISH);
            RCF_VERIFY(
                mZerr == Z_BUF_ERROR || mZerr == Z_STREAM_END,
                FilterException(
                    _RcfError_Zlib(), mZerr, RcfSubsystem_Zlib,
                    "deflate() failed"))
                    (mZerr)(outPos)(outRemaining);

            RCF_ASSERT(mCstream.avail_out > 0)(mCstream.avail_out);

            std::size_t bytesOut = outRemaining - mCstream.avail_out;
            mTotalBytesOut += bytesOut;
            outPos += bytesOut;
            outRemaining -= bytesOut;
        }

        mPreBuffers.resize(0);
        outBuffer = ByteBuffer(outBuffer, 0, mTotalBytesOut);
    }

    const FilterDescription & ZlibStatelessCompressionFilter::sGetFilterDescription()
    {
        return *spFilterDescription;
    }

    const FilterDescription & ZlibStatefulCompressionFilter::sGetFilterDescription()
    {
        return *spFilterDescription;
    }

    const FilterDescription *ZlibStatelessCompressionFilter::spFilterDescription = NULL;
    const FilterDescription *ZlibStatefulCompressionFilter::spFilterDescription = NULL;

    static void initZlibCompressionFilterDescriptions()
    {
        RCF_ASSERT(!ZlibStatelessCompressionFilter::spFilterDescription);
        RCF_ASSERT(!ZlibStatefulCompressionFilter::spFilterDescription);

        ZlibStatelessCompressionFilter::spFilterDescription =
            new FilterDescription(
                "Zlib stateless compression filter",
                RcfFilter_ZlibCompressionStateless,
                false);

        ZlibStatefulCompressionFilter::spFilterDescription =
            new FilterDescription(
                "Zlib stateful compression filter",
                RcfFilter_ZlibCompressionStateful,
                false);
    }

    static void deinitZlibCompressionFilterDescriptions()
    {
        delete ZlibStatelessCompressionFilter::spFilterDescription;
        ZlibStatelessCompressionFilter::spFilterDescription = NULL;

        delete ZlibStatefulCompressionFilter::spFilterDescription;
        ZlibStatefulCompressionFilter::spFilterDescription = NULL;
    }

    RCF_ON_INIT_DEINIT(
        initZlibCompressionFilterDescriptions(),
        deinitZlibCompressionFilterDescriptions())

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4355 )  // warning C4355: 'this' : used in base member initializer list
#endif

    ZlibCompressionFilterBase::ZlibCompressionFilterBase(int bufferSize, bool stateful, bool serverSide) :
        mPreState(Ready),
        mReadFilter( new ZlibCompressionReadFilter(*this, bufferSize, serverSide) ),
        mWriteFilter( new ZlibCompressionWriteFilter(*this, bufferSize, stateful) )
    {}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    const FilterDescription & ZlibStatelessCompressionFilter::getFilterDescription() const
    {
        return sGetFilterDescription();
    }

    const FilterDescription & ZlibStatefulCompressionFilter::getFilterDescription() const
    {
        return sGetFilterDescription();
    }

    void ZlibCompressionFilterBase::reset()
    {
        mPreState = Ready;
        mReadFilter->reset();
        mWriteFilter->reset();
    }

    void ZlibCompressionFilterBase::read(
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested)
    {
        mReadFilter->clearPreBuffer();

        mPreState = Reading;
        mReadFilter->read(byteBuffer, bytesRequested);
    }

    void ZlibCompressionFilterBase::write(
        const std::vector<ByteBuffer> &byteBuffers)
    {
        mReadFilter->clearPreBuffer();

        mPreState = Writing;
        mWriteFilter->write(byteBuffers);
    }

    void ZlibCompressionFilterBase::onReadCompleted(
        const ByteBuffer &byteBuffer)
    {
        mReadFilter->onReadCompleted(byteBuffer);
    }

    void ZlibCompressionFilterBase::onWriteCompleted(
        std::size_t bytesTransferred)
    {
        mWriteFilter->onWriteCompleted(bytesTransferred);
    }

    ZlibStatelessCompressionFilterFactory::ZlibStatelessCompressionFilterFactory(
        int bufferSize) :
            mBufferSize(bufferSize)
    {}

    FilterPtr ZlibStatelessCompressionFilterFactory::createFilter()
    {
        return FilterPtr( new ZlibStatelessCompressionFilter(
            (ServerSide *) NULL,
            mBufferSize));
    }

    const FilterDescription & ZlibStatelessCompressionFilterFactory::getFilterDescription()
    {
        return ZlibStatelessCompressionFilter::sGetFilterDescription();
    }

    ZlibStatefulCompressionFilterFactory::ZlibStatefulCompressionFilterFactory(
        int bufferSize) :
            mBufferSize(bufferSize)
    {}

    FilterPtr ZlibStatefulCompressionFilterFactory::createFilter()
    {
        return FilterPtr( new ZlibStatefulCompressionFilter( 
            (ServerSide *) NULL, 
            mBufferSize));
    }

    const FilterDescription & ZlibStatefulCompressionFilterFactory::getFilterDescription()
    {
        return ZlibStatefulCompressionFilter::sGetFilterDescription();
    }

} // namespace RCF
