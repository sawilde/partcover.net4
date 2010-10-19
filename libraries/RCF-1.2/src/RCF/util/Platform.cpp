
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <boost/config.hpp>
#include <boost/cstdint.hpp>

#include <RCF/InitDeinit.hpp>

// In debug builds, the tick counter rolls over after 15 seconds.
#ifdef NDEBUG
static const boost::uint32_t OffsetMs = 15*1000;
#else
static const boost::uint32_t OffsetMs = 0;
#endif

#ifdef BOOST_WINDOWS

// Windows implementation, using GetTickCount().
#include <Windows.h>

namespace Platform {
    namespace OS {
        boost::uint32_t getCurrentTimeMs()
        {
            static boost::uint32_t BaseTickCountMs = GetTickCount();
            boost::uint32_t timeMs = GetTickCount();
            return  timeMs - BaseTickCountMs - OffsetMs;
        }
    }
}

#else

// Non-Windows implementation, using gettimeofday().
#include <sys/time.h>

namespace Platform {
    namespace OS {

        // TODO: any issues with monotonicity of gettimeofday()?
        boost::uint32_t getCurrentTimeMs()
        {
            static struct timeval start = {0};
            static bool init = false;
            if (!init)
            {
                init = true;
                gettimeofday(&start, NULL);
            }

            struct timeval now;
            gettimeofday(&now, NULL);

            long seconds =  now.tv_sec - start.tv_sec;
            long microseconds = now.tv_usec - start.tv_usec;
            boost::uint64_t timeMs = boost::uint64_t(seconds)*1000 + microseconds/1000;
            timeMs = timeMs & 0xFFFFFFFF;
            return static_cast<boost::uint32_t>(timeMs) - OffsetMs;
        }

    }
}

#endif

RCF_ON_INIT_NAMED(Platform::OS::getCurrentTimeMs(), InitGetCurrentTime)
