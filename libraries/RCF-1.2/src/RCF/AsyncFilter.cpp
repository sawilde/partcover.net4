
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/AsyncFilter.hpp>

#include <RCF/ByteBuffer.hpp>
#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/ThreadLocalCache.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    // FilterDescription

    FilterDescription::FilterDescription(
        const std::string &filterName,
        int filterId,
        bool removable) :
            mFilterName(filterName),
            mFilterId(filterId),
            mRemovable(removable)
    {}

    const std::string &FilterDescription::getName() const
    {
        return mFilterName;
    }

    int FilterDescription::getId() const
    {
        return mFilterId;
    }

    bool FilterDescription::getRemovable() const
    {
        return mRemovable;
    }


    // Filter

    Filter::Filter() :
        mpPreFilter(RCF_DEFAULT_INIT),
        mpPostFilter(RCF_DEFAULT_INIT)
    {}

    Filter::~Filter()
    {}

    void Filter::reset()
    {}

    void Filter::setPreFilter(Filter &preFilter)
    {
        mpPreFilter = &preFilter;
    }

    void Filter::setPostFilter(Filter &postFilter)
    {
        mpPostFilter = &postFilter;
    }

    Filter &Filter::getPreFilter()
    {
        return *mpPreFilter;
    }

    Filter &Filter::getPostFilter()
    {
        return *mpPostFilter;
    }


    // IdentityFilter

    void IdentityFilter::read(const ByteBuffer &byteBuffer, std::size_t bytesRequested)
    {
        getPostFilter().read(byteBuffer, bytesRequested);
    }

    void IdentityFilter::write(const std::vector<ByteBuffer> &byteBuffers)
    {
        getPostFilter().write(byteBuffers);
    }

    void IdentityFilter::onReadCompleted(const ByteBuffer &byteBuffer)
    {
        getPreFilter().onReadCompleted(byteBuffer);
    }

    void IdentityFilter::onWriteCompleted(std::size_t bytesTransferred)
    {
        getPreFilter().onWriteCompleted(bytesTransferred);
    }

    const FilterDescription & IdentityFilter::getFilterDescription() const
    {
        return sGetFilterDescription();
    }

    const FilterDescription & IdentityFilter::sGetFilterDescription()
    {
        return *spFilterDescription;
    }

    const FilterDescription *IdentityFilter::spFilterDescription = NULL;

    RCF_ON_INIT_DEINIT(
        IdentityFilter::spFilterDescription = new FilterDescription("identity filter", RcfFilter_Identity, true),
        delete IdentityFilter::spFilterDescription; IdentityFilter::spFilterDescription = NULL )

    // IdentityFilterFactory

    FilterPtr IdentityFilterFactory::createFilter()
    {
        return FilterPtr( new IdentityFilter() );
    }

    const FilterDescription & IdentityFilterFactory::getFilterDescription()
    {
        return IdentityFilter::sGetFilterDescription();
    }

    // XorFilter

    void createNonReadOnlyByteBuffers(
        std::vector<ByteBuffer> &nonReadOnlyByteBuffers,
        const std::vector<ByteBuffer> &byteBuffers)
    {
        nonReadOnlyByteBuffers.resize(0);
        for (std::size_t i=0; i<byteBuffers.size(); ++i)
        {
            if (byteBuffers[i].getLength()  > 0)
            {
                if (byteBuffers[i].getReadOnly())
                {
                    boost::shared_ptr< std::vector<char> > spvc(
                        new std::vector<char>( byteBuffers[i].getLength()));

                    memcpy(
                        &(*spvc)[0],
                        byteBuffers[i].getPtr(),
                        byteBuffers[i].getLength() );

                    nonReadOnlyByteBuffers.push_back(
                        ByteBuffer(&(*spvc)[0], spvc->size(), spvc));
                }
                else
                {
                    nonReadOnlyByteBuffers.push_back(byteBuffers[i]);
                }
            }
        }
    }

    XorFilter::XorFilter() :
        mMask('U')
    {}
    
    void XorFilter::read(const ByteBuffer &byteBuffer, std::size_t bytesRequested)
    {
        getPostFilter().read(byteBuffer, bytesRequested);
    }

    void XorFilter::write(const std::vector<ByteBuffer> &byteBuffers)
    {
        // need to make copies of any readonly buffers before transforming
        // TODO: only do this if at least one byte buffer is non-readonly

        createNonReadOnlyByteBuffers(mByteBuffers, byteBuffers);

        // in place transformation
        for (std::size_t i=0; i<mByteBuffers.size(); ++i)
        {
            for (std::size_t j=0; j<mByteBuffers[i].getLength() ; ++j)
            {
                mByteBuffers[i].getPtr() [j] ^= mMask;
            }
        }

        getPostFilter().write(mByteBuffers);
        mByteBuffers.resize(0);
    }

    void XorFilter::onReadCompleted(const ByteBuffer &byteBuffer)
    {
        for (std::size_t i=0; i<byteBuffer.getLength() ; ++i)
        {
            byteBuffer.getPtr() [i] ^= mMask;
        }
        getPreFilter().onReadCompleted(byteBuffer);
    }

    void XorFilter::onWriteCompleted(std::size_t bytesTransferred)
    {
        getPreFilter().onWriteCompleted(bytesTransferred);
    }

    const FilterDescription & XorFilter::getFilterDescription() const
    {
        return sGetFilterDescription();
    }

    const FilterDescription & XorFilter::sGetFilterDescription()
    {
        return *spFilterDescription;
    }

    const FilterDescription *XorFilter::spFilterDescription = NULL;

    RCF_ON_INIT_DEINIT(
        XorFilter::spFilterDescription = new FilterDescription("Xor filter", RcfFilter_Xor, true); ,
        delete XorFilter::spFilterDescription; XorFilter::spFilterDescription = NULL;)


    // XorFilterFactory

    FilterPtr XorFilterFactory::createFilter()
    {
        return FilterPtr( new XorFilter() );
    }

    const FilterDescription & XorFilterFactory::getFilterDescription()
    {
        return XorFilter::sGetFilterDescription();
    }

    class ReadProxy : public IdentityFilter
    {
    public:
        ReadProxy() :
            mInByteBufferPos(RCF_DEFAULT_INIT),
            mBytesTransferred(RCF_DEFAULT_INIT)
        {}

        std::size_t getOutBytesTransferred() const
        {
            return mBytesTransferred;
        }

        ByteBuffer getOutByteBuffer()
        {
            ByteBuffer byteBuffer = mOutByteBuffer;
            mOutByteBuffer = ByteBuffer();
            return byteBuffer;
        }

        void setInByteBuffer(const ByteBuffer &byteBuffer)
        {
            mInByteBuffer = byteBuffer;
        }

        void clear()
        {
            mInByteBuffer = ByteBuffer();
            mOutByteBuffer = ByteBuffer();
            mInByteBufferPos = 0;
            mBytesTransferred = 0;
        }

    private:

        void read(const ByteBuffer &byteBuffer, std::size_t bytesRequested)
        {
            RCF_ASSERT(byteBuffer.isEmpty())(byteBuffer.getLength());

            RCF_ASSERT(
                0 <= mInByteBufferPos && mInByteBufferPos < mInByteBuffer.getLength())
                (mInByteBufferPos)(mInByteBuffer.getLength());

            std::size_t bytesRemaining = mInByteBuffer.getLength() - mInByteBufferPos;
            std::size_t bytesToRead = RCF_MIN(bytesRemaining, bytesRequested);
            ByteBuffer myByteBuffer(mInByteBuffer, mInByteBufferPos, bytesToRead);
            mInByteBufferPos += bytesToRead;
            getPreFilter().onReadCompleted(myByteBuffer);
        }

        void onReadCompleted(const ByteBuffer &byteBuffer)
        {
            mOutByteBuffer = byteBuffer;
            mBytesTransferred = byteBuffer.getLength();
        }

    private:

        std::size_t mInByteBufferPos;
        std::size_t mBytesTransferred;

        ByteBuffer mInByteBuffer;
        ByteBuffer mOutByteBuffer;
    };

    class WriteProxy : public IdentityFilter, boost::noncopyable
    {
    public:
        WriteProxy() :
            mBytesTransferred(RCF_DEFAULT_INIT),
            mTlcByteBuffers(),
            mByteBuffers(mTlcByteBuffers.get())
        {
        }

        const std::vector<ByteBuffer> &getByteBuffers() const
        {
            return mByteBuffers;
        }

        void clear()
        {
            mByteBuffers.resize(0);
            mBytesTransferred = 0;
        }

        std::size_t getBytesTransferred() const
        {
            return mBytesTransferred;
        }

    private:

        void write(const std::vector<ByteBuffer> &byteBuffers)
        {
            std::copy(
                byteBuffers.begin(),
                byteBuffers.end(),
                std::back_inserter(mByteBuffers));

            std::size_t bytesTransferred = lengthByteBuffers(byteBuffers);
            getPreFilter().onWriteCompleted(bytesTransferred);
        }

        void onWriteCompleted(std::size_t bytesTransferred)
        {
            mBytesTransferred = bytesTransferred;
        }

    private:
        std::size_t mBytesTransferred;

        RCF::ThreadLocalCached< std::vector<RCF::ByteBuffer> > mTlcByteBuffers;
        std::vector<RCF::ByteBuffer> &mByteBuffers;
    };

    bool filterData(
        const std::vector<ByteBuffer> &unfilteredData,
        std::vector<ByteBuffer> &filteredData,
        const std::vector<FilterPtr> &filters)
    {
        std::size_t bytesTransferred        = 0;
        std::size_t bytesTransferredTotal   = 0;

        WriteProxy writeProxy;
        writeProxy.setPreFilter(*filters.back());
        filters.back()->setPostFilter(writeProxy);
        filters.front()->setPreFilter(writeProxy);

        std::size_t unfilteredDataLen = lengthByteBuffers(unfilteredData);
        while (bytesTransferredTotal < unfilteredDataLen)
        {

            ThreadLocalCached< std::vector<ByteBuffer> > tlcSlicedByteBuffers;
            std::vector<ByteBuffer> &slicedByteBuffers = tlcSlicedByteBuffers.get();
            sliceByteBuffers(slicedByteBuffers, unfilteredData, bytesTransferredTotal);
            filters.front()->write(slicedByteBuffers);

            // TODO: error handling
            bytesTransferred = writeProxy.getBytesTransferred();
            bytesTransferredTotal += bytesTransferred;
        }
        RCF_ASSERT(
            bytesTransferredTotal == unfilteredDataLen)
            (bytesTransferredTotal)(unfilteredDataLen);

        filteredData.resize(0);

        std::copy(
            writeProxy.getByteBuffers().begin(),
            writeProxy.getByteBuffers().end(),
            std::back_inserter(filteredData));

        return bytesTransferredTotal == unfilteredDataLen;
    }

    bool unfilterData(
        const ByteBuffer &filteredByteBuffer,
        std::vector<ByteBuffer> &unfilteredByteBuffers,
        std::size_t unfilteredDataLen,
        const std::vector<FilterPtr> &filters)
    {
        int error                           = 0;
        std::size_t bytesTransferred        = 0;
        std::size_t bytesTransferredTotal   = 0;

        ByteBuffer byteBuffer;
        unfilteredByteBuffers.resize(0);

        ReadProxy readProxy;
        readProxy.setInByteBuffer(filteredByteBuffer);
        readProxy.setPreFilter(*filters.back());
        filters.back()->setPostFilter(readProxy);
        filters.front()->setPreFilter(readProxy);

        while (!error && bytesTransferredTotal < unfilteredDataLen)
        {
            filters.front()->read(ByteBuffer(), unfilteredDataLen - bytesTransferredTotal);
            bytesTransferred = readProxy.getOutBytesTransferred();
            byteBuffer = readProxy.getOutByteBuffer();
            // TODO: error handling
            bytesTransferredTotal += (error) ? 0 : bytesTransferred;
            unfilteredByteBuffers.push_back(byteBuffer);
        }
        return bytesTransferredTotal == unfilteredDataLen;
    }

    bool unfilterData(
        const ByteBuffer &filteredByteBuffer,
        ByteBuffer &unfilteredByteBuffer,
        std::size_t unfilteredDataLen,
        const std::vector<FilterPtr> &filters)
    {
        ThreadLocalCached< std::vector<ByteBuffer> > tlcUnfilteredByteBuffers;
        std::vector<ByteBuffer> &unfilteredByteBuffers = tlcUnfilteredByteBuffers.get();

        bool ret = unfilterData(
            filteredByteBuffer,
            unfilteredByteBuffers,
            unfilteredDataLen,
            filters);

        if (unfilteredByteBuffers.empty())
        {
            unfilteredByteBuffer = ByteBuffer();
        }
        else if (unfilteredByteBuffers.size() == 1)
        {
            unfilteredByteBuffer = unfilteredByteBuffers[0];
        }
        else
        {
            // TODO: maybe check for adjacent buffers, in which case one should not need to make a copy
            copyByteBuffers(unfilteredByteBuffers, unfilteredByteBuffer);
        }
        return ret;
    }

    void connectFilters(const std::vector<FilterPtr> &filters)
    {
        for (std::size_t i=0; i<filters.size(); ++i)
        {
            if (i>0)
            {
                filters[i]->setPreFilter(*filters[i-1]);
            }
            if (i<filters.size()-1)
            {
                filters[i]->setPostFilter(*filters[i+1]);
            }
        }
    }

} // namespace RCF
