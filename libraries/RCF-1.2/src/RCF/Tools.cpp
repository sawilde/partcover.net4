
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/Tools.hpp>

#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>

#include <RCF/util/Platform/OS/GetCurrentTime.hpp>

#ifndef __BORLANDC__
namespace std {
#endif

    // Trace type_info
    std::ostream &operator<<(std::ostream &os, const std::type_info &ti)
    {
        return os << ti.name();
    }

    // Trace exception
    std::ostream &operator<<(std::ostream &os, const std::exception &e)
    {
        return os
            << "Type: " << typeid(e).name()
            << ", What: " << e.what();
    }

    // Trace exception
    std::ostream &operator<<(std::ostream &os, const RCF::Exception &e)
    {
        os << "Type: " << typeid(e).name();
        os << ", What: " << e.what();

        os << ", RCF:" << e.getErrorId();
        os << ": " << e.getError().getErrorString();

        if (e.getSubSystem() == RCF::RcfSubsystem_Os)
        {
            os << ", OS:" << e.getSubSystemError();
            os << ": " << RCF::getOsErrorString(e.getSubSystemError());
        }
        else if (e.getSubSystem() == RCF::RcfSubsystem_Asio)
        {
            os << ", Asio:" << e.getSubSystemError();
        }
        else if (e.getSubSystem() == RCF::RcfSubsystem_Zlib)
        {
            os << ", Zlib:" << e.getSubSystemError();
        }
        else if (e.getSubSystem() == RCF::RcfSubsystem_OpenSsl)
        {
            os << ", OpenSSSL:" << e.getSubSystemError();
        }

        return os;
    }

#ifndef __BORLANDC__
} // namespace std
#endif

#if defined(_MSC_VER) && _MSC_VER == 1200

namespace std {

    std::ostream &operator<<(std::ostream &os, __int64)
    {
        // TODO
        RCF_ASSERT(0);
        return os;
    }

    std::ostream &operator<<(std::ostream &os, unsigned __int64)
    {
        // TODO
        RCF_ASSERT(0);
        return os;
    }

    std::istream &operator>>(std::istream &os, __int64 &)
    {
        // TODO
        RCF_ASSERT(0);
        return os;
    }

    std::istream &operator>>(std::istream &os, unsigned __int64 &)
    {
        // TODO
        RCF_ASSERT(0);
        return os;
    }

}

#endif


namespace RCF {

    std::string toString(const std::exception &e)
    {
        std::ostringstream os;

        const RCF::Exception *pE = dynamic_cast<const RCF::Exception *>(&e);
        if (pE)
        {
            int err = pE->getErrorId();
            std::string errMsg = pE->getError().getErrorString();
            os << "[RCF:" << err << "] " << errMsg << std::endl;
            if (pE->getSubSystem() == RCF::RcfSubsystem_Os)
            {
                err = pE->getSubSystemError();
                errMsg = Platform::OS::GetErrorString(err);
                os << "[OS:" << err << "] " << errMsg << std::endl;
            }
            os << "[Context] " << pE->getContext() << std::endl;
        }

        os << "[What] " << e.what() << std::endl;
        os << "[Exception type] " << typeid(e).name() << std::endl;
        return os.str();
    }

    util::TraceChannel *pTraceChannels[10];

    util::TraceChannel **getTraceChannels()
    {
        return pTraceChannels;
    }

    // 32 bit millisecond counter. Turns over every 49 days or so.
    unsigned int getCurrentTimeMs()
    {
        return Platform::OS::getCurrentTimeMs();
    }

    // Generate a timeout value for the given ending time.
    // Returns zero if endTime <= current time <= endTime+10% of timer resolution,
    // otherwise returns a nonzero duration in ms.
    // Timer resolution as above (49 days).
    unsigned int generateTimeoutMs(unsigned int endTimeMs)
    {
        // 90% of the timer interval
        static const unsigned int maxTimeoutMs = (((unsigned int)-1)/10)*9;
        unsigned int currentTimeMs = getCurrentTimeMs();
        unsigned int timeoutMs = endTimeMs - currentTimeMs;
        return (timeoutMs < maxTimeoutMs) ? timeoutMs : 0;
    }

    void deinitRcfTraceChannels()
    {
        for (int i=0; i<10; ++i)
        {
            delete pTraceChannels[i];
            pTraceChannels[i] = NULL;
        }
    }

    void initRcfTraceChannels()
    {
        deinitRcfTraceChannels();
        for (int i=0; i<10; ++i)
        {
            std::string name = "RCF";
            if (i > 0)
            {
                name += '0' + static_cast<char>(i);
            }
            pTraceChannels[i] = new util::TraceChannel(name);
        }

#ifndef NDEBUG
        pTraceChannels[9]->setTraceTarget("ODS");
#endif

    }

    RCF_ON_INIT_DEINIT_NAMED(
        initRcfTraceChannels(),
        deinitRcfTraceChannels(),
        InitRcfTraceChannels);

    boost::uint64_t fileSize(const std::string & path)
    {
        // TODO: this may not work for files larger than 32 bits, on some 32 bit
        // STL implementations. msvc for instance.

        std::ifstream fin ( path.c_str() );
        RCF_VERIFY(fin, Exception("Could not open file."));
        std::size_t begin = fin.tellg();
        fin.seekg (0, std::ios::end);
        std::size_t end = fin.tellg();
        fin.close();
        return end - begin;
    }

} // namespace RCF
