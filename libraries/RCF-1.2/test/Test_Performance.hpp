
// DEPRECATED
//#error deprecated

#ifndef INCLUDE_TEST_PERFORMANCE_HPP
#define INCLUDE_TEST_PERFORMANCE_HPP

#include <string>

#include <boost/lexical_cast.hpp>

#include <RCF/ClientTransport.hpp>
#include <RCF/FilterService.hpp>
#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/ServerTransport.hpp>
#include <RCF/util/Profile.hpp>

#include <RCF/test/WithStopServer.hpp>

namespace Test_Performance {

    class X : public RCF::WithStopServer
    {
    public:
        void f0() {}
        int f1(int) { return 0; }
        std::string f2(const std::string &) { return ""; }
        std::string echo(const std::string &s) { return s; }
        RCF::ByteBuffer echo(RCF::ByteBuffer byteBuffer) { return byteBuffer; }
    };

    RCF_BEGIN( I_X, "I_X" )
        RCF_METHOD_V0(void, f0)
        RCF_METHOD_R1(int, f1, int)
        RCF_METHOD_R1(std::string, f2, const std::string &)
        RCF_METHOD_R1(std::string, echo, const std::string &)
        RCF_METHOD_R1(RCF::ByteBuffer, echo, RCF::ByteBuffer)
        RCF_METHOD_V0(void, stopServer)
    RCF_END( I_X )

    bool gServer = true;
    bool gClient = true;

    void runPerformanceTest(
                            const std::string &title,
                            RCF::ClientTransportAutoPtr clientTransportAutoPtr,
                            RCF::ServerTransportPtr serverTransportPtr,
                            RCF::RemoteCallSemantics rcs,
                            const std::string &s0,
                            int serializationProtocol,
                            const std::vector<RCF::FilterFactoryPtr> &filterFactories,
                            const std::vector<RCF::FilterPtr> &payloadFilters,
                            const std::vector<RCF::FilterPtr> &transportFilters,
                            int calls,
                            unsigned int timeoutMs = 1000*10,
                            bool precall = true)
    {
        std::string serverTransportDesc = typeid(*serverTransportPtr).name();
        std::string clientTransportDesc = typeid(*clientTransportAutoPtr).name();
        std::string transportDesc = "(" + serverTransportDesc + ", " + clientTransportDesc + ")";

        RCF::FilterServicePtr filterServicePtr( new RCF::FilterService() );
        for (unsigned int i=0; i<filterFactories.size(); ++i)
        {
            filterServicePtr->addFilterFactory( filterFactories[i] );
        }

        std::string payloadFiltersDesc = "<empty>";
        if (!payloadFilters.empty())
        {
            payloadFiltersDesc = "";;
            for (unsigned int i=0; i<payloadFilters.size(); ++i)
            {
                payloadFiltersDesc += "<" + payloadFilters[i]->getFilterDescription().getName() + ">";
            }
        }

        std::string transportFiltersDesc = "<empty>";
        if (!transportFilters.empty())
        {
            transportFiltersDesc = "";
            for (unsigned int i=0; i<transportFilters.size(); ++i)
            {
                transportFiltersDesc += "<" + transportFilters[i]->getFilterDescription().getName() + ">";
            }
        }

        std::string serializationProtocolDesc =
            boost::lexical_cast<std::string>(serializationProtocol)
            + " (" + RCF::getSerializationProtocolName(serializationProtocol) + ")";

        RCF_ASSERT(rcs == RCF::Oneway || rcs == RCF::Twoway);
        std::string directionDesc = (rcs == RCF::Oneway) ? "one-way" : "two-way";

        std::string profileDesc = title
            + " Transport: " + transportDesc
            + ", Semantics: " + "std::string echo(const std::string &)"
            + ", Direction: " + directionDesc
            + ", String length: " + boost::lexical_cast<std::string>(s0.length())
            + ", Ser. protocol: " + serializationProtocolDesc
            + ", Payload filters: " + payloadFiltersDesc
            + ", Transport filters: " + transportFiltersDesc
            + ", Calls: " + boost::lexical_cast<std::string>(calls);

        X x;
        RCF::RcfServer server(serverTransportPtr);
        server.bind( (I_X*) 0, x);
        server.addService(filterServicePtr);
        if (gServer)
        {
            server.start();
        }
        else
        {
            Platform::OS::SleepMs(1000);
        }

        if (gClient)
        {
            // all calls on the same connection
            RcfClient<I_X> client(clientTransportAutoPtr->clone());
            client.getClientStub().setRemoteCallTimeoutMs(timeoutMs);
            RCF::SerializationProtocol sp = RCF::SerializationProtocol(serializationProtocol);
            client.getClientStub().setSerializationProtocol(sp);
            if (!payloadFilters.empty())
            {
                client.getClientStub().setMessageFilters(payloadFilters);
            }
            if (!transportFilters.empty())
            {
                client.getClientStub().requestTransportFilters(transportFilters);
            }

            if (precall)
            {
                std::string s = client.echo(rcs, s0);
            }
            {
                std::cout << profileDesc << std::endl;
                util::Profile profile(profileDesc);
                for (int i=0; i<calls; ++i)
                {
                    //if (s0.length() < 1024*1024)
                    //{
                    //    std::string s = client.echo(rcs, s0);
                    //}
                    //else
                    {
                        RCF::ByteBuffer byteBuffer = client.echo(
                            rcs,
                            RCF::ByteBuffer( (char*) s0.c_str(), s0.length()));
                    }
                }
            }

            // a timeout here, to let the server catch its breath, esp. after oneway calls
            Platform::OS::SleepMs(300);

            RcfClient<I_X>(clientTransportAutoPtr->clone()).stopServer();
        }

        if (gServer)
        {
            x.wait();
            server.stop();
        }

    }

} // namespace Test_Performance

#endif // ! INCLUDE_TEST_PERFORMANCE_HPP
