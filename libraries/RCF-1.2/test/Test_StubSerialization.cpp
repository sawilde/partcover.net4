
#include <iostream>
#include <sstream>
#include <string>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/CommandLine.hpp>

#include <SF/AdlWorkaround.hpp>
#include <SF/memory.hpp>

#include <SF/ITextStream.hpp>
#include <SF/OTextStream.hpp>

#ifdef RCF_USE_BOOST_SERIALIZATION
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>
BOOST_SERIALIZATION_SHARED_PTR(RCF::TcpEndpoint)
BOOST_SERIALIZATION_SHARED_PTR(RCF::UdpEndpoint)
//BOOST_CLASS_EXPORT_GUID(RCF::UdpEndpoint, "RCF::UdpEndpoint")
//BOOST_CLASS_EXPORT_GUID(RCF::TcpEndpoint, "RCF::TcpEndpoint")
#endif

namespace Test_StubSerialization {

    RCF_BEGIN(I_X, "I_X")
        RCF_METHOD_V0(void, increment);
        RCF_METHOD_R0(int, getCount);
    RCF_END(I_X);

    class X
    {
    public:
        X() : count(RCF_DEFAULT_INIT)
        {}

        void increment()
        {
            count++;
        }

        int getCount()
        {
            return count;
        }

    private:
        int count;
    };

} // namespace Test_StubSerialization

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_StubSerialization::I_X::RcfClientT)

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_StubSerialization;

    util::CommandLine::getSingleton().parse(argc, argv);

    for (unsigned int i=0; i<RCF::getTransportFactories().size(); ++i)
    {

        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        X x;
        RCF::RcfServer server(serverTransportPtr);
        server.bind( (I_X*) 0, x);
        server.start();

        //RCF::TcpEndpoint endpoint( server.getIpServerTransport().getPort() );

        RCF::EndpointPtr endpointPtr = clientTransportAutoPtr->getEndpointPtr();
        
        RcfClient<I_X> client( *endpointPtr );
        client.increment();

        std::ostringstream os1;
        SF::OTextStream(os1) << client;
        std::string s1 = os1.str();
        RCF_TRACE("")(s1);

        {
            RcfClient<I_X> client2;
            std::istringstream is(s1);
            SF::ITextStream(is) >> client2;
            client2.increment();
        }
        {
            RcfClient<I_X> client3;
            std::istringstream is(s1);
            SF::ITextStream(is) >> client3;
            client3.increment();
        }
/*
        // Polymorphic serialization in B.Ser. is seriously broken on gcc.
        // On gcc 3.4, this test passes in debug, and crashes on release.
        // On gcc 3.2, it asserts.
#if defined(RCF_USE_BOOST_SERIALIZATION) && !defined(__GNUC__)

        {
            std::ostringstream os;
            boost::archive::text_oarchive(os) & client;
            std::string s = os.str();
            RCF_TRACE("")(s);

            RcfClient<I_X> client4;
            std::istringstream is(s);
            boost::archive::text_iarchive(is) & client4;
            client4.increment();
        }

        int count = client.getCount();
        RCF_CHECK(count == 4);

#else
*/
        int count = client.getCount();
        RCF_CHECK(count == 3);

//#endif

    }
   
    return boost::exit_success;
}
