
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_SESSIONTIMEOUTSERVICE_HPP
#define INCLUDE_RCF_SESSIONTIMEOUTSERVICE_HPP

#include <set>

#include <RCF/Export.hpp>
#include <RCF/Service.hpp>

namespace RCF {

    class RcfSession;
    typedef boost::shared_ptr<RcfSession> RcfSessionPtr;
    typedef boost::weak_ptr<RcfSession> RcfSessionWeakPtr;

    class RCF_EXPORT SessionTimeoutService : public I_Service
    {
    public:
        SessionTimeoutService(
            boost::uint32_t sessionTimeoutMs,
            boost::uint32_t reapingIntervalMs = 30*1000);

    private:

        void onServiceAdded(RcfServer &server);
        void onServiceRemoved(RcfServer &server);

        void stop();

        bool cycle(
            int timeoutMs,
            const volatile bool &stopFlag);

    private:

        std::vector<RcfSessionWeakPtr>  mSessionsTemp;

        boost::uint32_t                 mSessionTimeoutMs;
        boost::uint32_t                 mLastRunTimeMs;
        boost::uint32_t                 mReapingIntervalMs;

        bool                            mStopFlag;

        RcfServer *                     mpRcfServer;

    };

    typedef boost::shared_ptr<SessionTimeoutService> SessionTimeoutServicePtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_SESSIONTIMEOUTSERVICE_HPP
