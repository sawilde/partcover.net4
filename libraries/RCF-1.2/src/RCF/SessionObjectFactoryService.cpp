
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/SessionObjectFactoryService.hpp>

#include <RCF/ServerInterfaces.hpp>

#ifdef RCF_USE_PROTOBUF
#include <RCF/protobuf/RcfMessages.pb.h>
#endif

namespace RCF {

#ifdef RCF_USE_PROTOBUF

    class SessionObjectFactoryServicePb
    {
    public:

        SessionObjectFactoryServicePb(SessionObjectFactoryService & sofs) : mSofs(sofs)
        {

        }

        void createSessionObject(const CreateSessionObject & request)
        {
            int error = mSofs.createSessionObject(request.objectname());

            if (error != RCF::RcfError_Ok)
            {
                RCF_THROW( RemoteException( Error(error) ) );
            }
        }

        void deleteSessionObject(const DeleteSessionObject & request)
        {
            RCF_UNUSED_VARIABLE(request);

            int error = mSofs.deleteSessionObject();

            if (error != RCF::RcfError_Ok)
            {
                RCF_THROW( RemoteException( Error(error) ) );
            }
        }


    private:
        SessionObjectFactoryService & mSofs;
    };

    void onServiceAddedProto(SessionObjectFactoryService & sofs, RcfServer & server)
    {
        boost::shared_ptr<SessionObjectFactoryServicePb> sofsPbPtr(
            new SessionObjectFactoryServicePb(sofs));

        server.bind((I_SessionObjectFactoryPb *) NULL, sofsPbPtr);
    }

    void onServiceRemovedProto(SessionObjectFactoryService & sofs, RcfServer & server)
    {
        server.unbind( (I_SessionObjectFactoryPb *) NULL);
    }

#else

    void onServiceAddedProto(SessionObjectFactoryService &, RcfServer &)
    {
    }

    void onServiceRemovedProto(SessionObjectFactoryService &, RcfServer &)
    {
    }

#endif // RCF_USE_PROTOBUF

    void SessionObjectFactoryService::onServiceAdded(RcfServer &server)
    {
        server.bind((I_SessionObjectFactory *) NULL, *this);

        onServiceAddedProto(*this, server);
    }

    void SessionObjectFactoryService::onServiceRemoved(RcfServer &server)
    {
        server.unbind( (I_SessionObjectFactory *) NULL);

        onServiceRemovedProto(*this, server);
    }

} // namespace RCF
