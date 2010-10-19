
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_PERFORMANCEDATA_HPP
#define INCLUDE_RCF_PERFORMANCEDATA_HPP

#include <RCF/Export.hpp>
#include <RCF/ThreadLibrary.hpp>

#include <boost/cstdint.hpp>

namespace RCF {

    class RCF_EXPORT PerformanceData
    {
    public:
        PerformanceData() : mRcfSessions(0), mBufferCount(0), mTotalBufferSize(0)
        {
        }

        void collect();

        Mutex           mMutex;
        boost::uint32_t mRcfSessions;
        boost::uint32_t mBufferCount;
        boost::uint32_t mTotalBufferSize;
    };

    RCF_EXPORT PerformanceData & getPerformanceData();

} // namespace RCF

#endif // ! INCLUDE_RCF_PERFORMANCEDATA_HPP
