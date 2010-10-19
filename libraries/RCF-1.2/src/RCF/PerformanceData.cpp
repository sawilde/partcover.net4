
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/PerformanceData.hpp>

#include <RCF/InitDeinit.hpp>
#include <RCF/ObjectPool.hpp>

#include <numeric>

namespace RCF {

    PerformanceData *gpPerformanceData = NULL;

    RCF_ON_INIT( gpPerformanceData = new PerformanceData() );

    RCF_ON_DEINIT( delete gpPerformanceData; gpPerformanceData = NULL; );

    PerformanceData & getPerformanceData()
    {
        return *gpPerformanceData;
    }

    void PerformanceData::collect()
    {
        std::vector<std::size_t> inBufferSizes;
        std::vector<std::size_t> outBufferSizes;

        getObjectPool().enumerateBuffers(inBufferSizes);
        getObjectPool().enumerateOstrstreams(outBufferSizes);

        std::size_t inBufferSize = 
            std::accumulate(inBufferSizes.begin(), inBufferSizes.end(), std::size_t(0));

        std::size_t outBufferSize = 
            std::accumulate(outBufferSizes.begin(), outBufferSizes.end(), std::size_t(0));

        Lock lock(mMutex);
        
        mBufferCount = static_cast<boost::uint32_t>(
            inBufferSizes.size() + outBufferSizes.size());

        mTotalBufferSize = static_cast<boost::uint32_t>(
            inBufferSize + outBufferSize);
    }

} // namespace RCF
