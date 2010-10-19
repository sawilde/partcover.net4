
#include <RCF/test/TestMinimal.hpp>

#include <RCF/ClientTransport.hpp>
#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/EndpointBrokerService.hpp>
#include <RCF/EndpointServerService.hpp>
#include <RCF/ServerInterfaces.hpp>

#include <RCF/test/TransportFactories.hpp>
#include <RCF/test/ThreadGroup.hpp>

#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Platform/OS/Sleep.hpp>

#include "Test_Endpoint.hpp"

namespace Test_Endpoint {

    void endpointClient(
        const RCF::I_ClientTransport &clientTransport, 
        const std::string &mEndpointName, 
        int mReps)
    {
        try
        {
            for (int i=0; i<mReps; ++i)
            {
                
                RCF::RcfClient<RCF::I_EndpointBroker> broker(clientTransport.clone());

                int tries = 0;
                int err = RCF::RcfError_EndpointRetry;
                while (tries < 20 && err == RCF::RcfError_EndpointRetry)
                {
                    err = broker.connectToEndpoint(RCF::Twoway, mEndpointName, "");
                    ++tries;
                    Platform::OS::SleepMs(100);
                }

                RCF_CHECK(err == RCF::RcfError_Ok);
                MyRcfClient client( broker.getClientStub().releaseTransport() );
                std::string name = client.getEndpointName(RCF::Twoway);
                RCF_CHECK(name == mEndpointName);
            }
        }
        catch(const RCF::Exception &e)
        {
            RCF_CHECK(1==0);
            std::cout << e.what() << std::endl;
            RCF_TRACE("")(e);
        }
    }

} // namespace Test_Endpoint

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_Endpoint;

    util::CommandLine::getSingleton().parse(argc, argv);

    for (int i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports;

        transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr brokerServerTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr brokerClientTransportAutoPtr( *transports.second );

        if (NULL == dynamic_cast<RCF::I_ServerTransportEx *>(brokerServerTransportPtr.get()))
        {
            continue;
        }

        RCF::writeTransportTypes(std::cout, *brokerServerTransportPtr, *brokerClientTransportAutoPtr);

        transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtrA( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtrA( *transports.second );

        transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtrB( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtrB( *transports.second );

        transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtrC( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtrC( *transports.second );

        std::string endpointNameA = "EndpointA";
        std::string endpointNameB = "EndpointB";
        std::string endpointNameC = "EndpointC";

        RCF::RcfServer broker(brokerServerTransportPtr);
        broker.addService( RCF::EndpointBrokerServicePtr( new RCF::EndpointBrokerService ) );
        broker.start();

        for (int j=0; j<3; ++j)
        {
            typedef RCF::EndpointServerService::EndpointId EndpointId;
            EndpointId endpointId;

            // TODO: run a server transport w/o acceptor fd
            // TODO: ok to call openEndpoint(...) w/o explicitly starting the server

            boost::shared_ptr<GetEndpointName> implPtr;

            // server A
            RCF::RcfServer serverA(serverTransportPtrA);
            implPtr.reset( new GetEndpointName(endpointNameA));
            serverA.bind( (MyInterface *) 0, implPtr);
            serverA.start();

            RCF::EndpointPtr brokerClientEndpointPtr( brokerClientTransportAutoPtr->getEndpointPtr() );

            RCF::EndpointServerServicePtr endpointServerServiceAPtr( new RCF::EndpointServerService );
            serverA.addService(endpointServerServiceAPtr);
            endpointId = endpointServerServiceAPtr->openEndpoint(*brokerClientEndpointPtr, endpointNameA);
            RCF_CHECK(endpointId != EndpointId());

            // server B
            RCF::RcfServer serverB(serverTransportPtrB);
            implPtr.reset( new GetEndpointName(endpointNameB));
            serverB.bind( (MyInterface*) 0, implPtr);
            serverB.start();

            RCF::EndpointServerServicePtr endpointServerServiceBPtr( new RCF::EndpointServerService );
            serverB.addService(endpointServerServiceBPtr);
            endpointId = endpointServerServiceBPtr->openEndpoint(*brokerClientEndpointPtr, endpointNameB);
            RCF_CHECK(endpointId != EndpointId());

            // server C
            RCF::RcfServer serverC(serverTransportPtrC);
            implPtr.reset( new GetEndpointName(endpointNameC));
            serverC.bind( (MyInterface*) 0, implPtr);
            serverC.start();

            RCF::EndpointServerServicePtr endpointServerServiceCPtr( new RCF::EndpointServerService );
            serverC.addService(endpointServerServiceCPtr);
            endpointId = endpointServerServiceCPtr->openEndpoint(*brokerClientEndpointPtr, endpointNameC);
            RCF_CHECK(endpointId != EndpointId());

            Platform::OS::Sleep(1);

            // off we go
            ThreadGroup endpointClients;

            endpointClients.push_back( ThreadPtr( new Thread(
                boost::bind( &endpointClient, boost::ref(*brokerClientTransportAutoPtr), endpointNameA, 3) ) ) );

            endpointClients.push_back( ThreadPtr( new Thread(
                boost::bind( &endpointClient, boost::ref(*brokerClientTransportAutoPtr), endpointNameB, 3) ) ) );

            endpointClients.push_back( ThreadPtr( new Thread(
                boost::bind( &endpointClient, boost::ref(*brokerClientTransportAutoPtr), endpointNameC, 3) ) ) );

            joinThreadGroup(endpointClients);
        }
    }

    return boost::exit_success;
}






