
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_CURRENTSESSION_HPP
#define INCLUDE_RCF_CURRENTSESSION_HPP

#include <boost/shared_ptr.hpp>

#include <RCF/RcfSession.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    class SetCurrentSessionGuard
    {
    public:
        SetCurrentSessionGuard(const RcfSessionPtr &sessionPtr)
        {
            setCurrentRcfSessionPtr(sessionPtr);
        }

        SetCurrentSessionGuard(const SessionPtr &sessionPtr)
        {
            setCurrentRcfSessionPtr( boost::static_pointer_cast<RcfSession>(sessionPtr) );
        }

        ~SetCurrentSessionGuard()
        {
            setCurrentRcfSessionPtr();
        }
    };

    inline SessionPtr getCurrentSessionPtr()
    {
        return SessionPtr(getCurrentRcfSessionPtr());
    }

} // namespace RCF

#endif // ! INCLUDE_RCF_CURRENTSESSION_HPP
