
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_SESSIONOBJECTFACTORYSERVICE_HPP
#define INCLUDE_RCF_SESSIONOBJECTFACTORYSERVICE_HPP

#include <RCF/ObjectFactoryService.hpp>

namespace RCF {

    class RCF_EXPORT SessionObjectFactoryService :
        public I_Service,
        public StubFactoryRegistry,
        boost::noncopyable
    {
    public:
        boost::int32_t createSessionObject(const std::string &objectName);
        boost::int32_t deleteSessionObject();

    private:
        void onServiceAdded(RcfServer &server);
        void onServiceRemoved(RcfServer &server);
    };

    typedef boost::shared_ptr<SessionObjectFactoryService> 
        SessionObjectFactoryServicePtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_SESSIONOBJECTFACTORYSERVICE_HPP
