
#include <string>
#include <strstream>

#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/AsyncFilter.hpp>
#include <RCF/FilterService.hpp>
#include <RCF/Idl.hpp>
#include <RCF/OpenSslEncryptionFilter.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/ZlibCompressionFilter.hpp>

#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Profile.hpp>

#include <SF/AdlWorkaround.hpp>

#ifdef BOOST_WINDOWS
#include <RCF/SspiFilter.hpp>
#endif

namespace Test_ZeroAllocation {

    class ContainsByteBuffer
    {
    public:
        RCF::ByteBuffer mByteBuffer;

        void serialize(SF::Archive &archive)
        {
            archive & mByteBuffer;
        }

        template<typename Archive>
        void serialize(Archive &, const unsigned int)
        {
            RCF_ASSERT(0);
        }
    };

    class Echo
    {
    public:
        std::string echo(const std::string &s)
        {
            return s;
        }

        RCF::ByteBuffer echo(RCF::ByteBuffer byteBuffer1, const std::string &s, RCF::ByteBuffer byteBuffer2)
        {
            void *pv1 = byteBuffer1.getPtr() ;
            std::size_t pvlen1 = byteBuffer1.getLength() ;

            void *pv2 = byteBuffer2.getPtr() ;
            std::size_t pvlen2 = byteBuffer2.getLength() ;

            return byteBuffer2;
        }

        RCF::ByteBuffer echo(RCF::ByteBuffer byteBuffer)
        {
            return byteBuffer;
        }

        ContainsByteBuffer echo(ContainsByteBuffer c)
        {
            return c;
        }

    };

    bool gInstrumented = false;
    bool gExpectAllocations = true;
    std::size_t gnAllocations = 0;

} // namespace Test_ZeroAllocation

// User-defined operator new.
void *operator new(size_t bytes)
{
    if (Test_ZeroAllocation::gInstrumented)
    {
        RCF_CHECK(Test_ZeroAllocation::gExpectAllocations);
        if (!Test_ZeroAllocation::gExpectAllocations)
        {
            std::cout << "Unexpected memory allocation." << std::endl;
        }
        ++Test_ZeroAllocation::gnAllocations;
    }
    return malloc(bytes);
}

// User-defined operator delete.
void operator delete(void *pv) throw()
{
    free(pv);
}

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_ZeroAllocation::ContainsByteBuffer)

namespace Test_ZeroAllocation {

    RCF_BEGIN(I_Echo, "I_Echo")
        RCF_METHOD_R1(std::string, echo, const std::string &)
        RCF_METHOD_R3(RCF::ByteBuffer, echo, RCF::ByteBuffer, std::string, RCF::ByteBuffer)
        RCF_METHOD_R1(RCF::ByteBuffer, echo, RCF::ByteBuffer)
        RCF_METHOD_R1(ContainsByteBuffer, echo, ContainsByteBuffer)
    RCF_END(I_Echo)

} // namespace Test_ZeroAllocation

int test_main(int argc, char **argv)
{

    Test_ZeroAllocation::gInstrumented = true;

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_ZeroAllocation;

    util::CommandLineOption<std::string>    clScert("scert", RCF_TEMP_DIR "ssCert2.pem", "OpenSSL server certificate");
    util::CommandLineOption<std::string>    clSpwd("spwd", "mt2316", "OpenSSL server certificate password");
    util::CommandLineOption<std::string>    clCcert("ccert", RCF_TEMP_DIR "ssCert1.pem", "OpenSSL client certificate");
    util::CommandLineOption<std::string>    clCpwd("cpwd", "mt2316", "OpenSSL client certificate password");
    util::CommandLine::getSingleton().parse(argc, argv);

#if !defined(BOOST_WINDOWS)
    RCF_CHECK(1 == 0 && "Zero-allocation not yet supported with asio transports");
    return boost::exit_success;
#endif

#if defined(BOOST_WINDOWS) && defined(__MINGW32__) && __GNUC__ == 3 && __GNUC_MINOR__ == 4
    RCF_CHECK(1 == 0 && "Zero-allocation not working in gcc 3.4 for unknown reasons");
    return boost::exit_success;
#endif

    for (unsigned int i=0; i<RCF::getTransportFactories().size(); ++i)
    {

        RCF::TransportFactoryPtr transportFactoryPtr( RCF::getTransportFactories()[i] );
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );
        bool transportFiltersSupported = transportFactoryPtr->supportsTransportFilters();

#ifdef RCF_USE_BOOST_ASIO
        if (typeid(*serverTransportPtr) == typeid(RCF::TcpAsioServerTransport))
        {
            // still got some work to do on zero allocation
            continue;
        }
#endif

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);
        std::string transportDesc = "Transport " + boost::lexical_cast<std::string>(i) + ": ";

        serverTransportPtr->setMaxMessageLength(-1);
        clientTransportAutoPtr->setMaxMessageLength(-1);

        Echo echo;
        RCF::RcfServer server(serverTransportPtr);
        server.bind( (I_Echo*) 0, echo);

        RCF::FilterServicePtr filterServicePtr(new RCF::FilterService());
        filterServicePtr->addFilterFactory( RCF::FilterFactoryPtr( new RCF::IdentityFilterFactory()));
        filterServicePtr->addFilterFactory( RCF::FilterFactoryPtr( new RCF::XorFilterFactory()));
#ifdef RCF_USE_ZLIB
        filterServicePtr->addFilterFactory( RCF::FilterFactoryPtr( new RCF::ZlibStatefulCompressionFilterFactory()));
        filterServicePtr->addFilterFactory( RCF::FilterFactoryPtr( new RCF::ZlibStatelessCompressionFilterFactory()));
#endif
#ifdef RCF_USE_OPENSSL
        filterServicePtr->addFilterFactory( RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(clScert, clSpwd)));
#endif
#ifdef BOOST_WINDOWS
        filterServicePtr->addFilterFactory( RCF::FilterFactoryPtr( new RCF::NtlmFilterFactory()));
#endif
        server.addService(filterServicePtr);
       
        server.start();
       
        // make sure all allocations have taken place
        Platform::OS::SleepMs(1000);

        RcfClient<I_Echo> client(clientTransportAutoPtr->clone());
       
        //client.getClientStub().setRemoteCallTimeoutMs(1000*60*60);

        {
            std::size_t nAllocations = gnAllocations;
            std::auto_ptr<int> apn(new int(17));
            apn.reset();
            RCF_CHECK(gnAllocations != nAllocations);
        }

        {
            std::string s = "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdf";
            RCF::ByteBuffer byteBuffer0( (char*) s.c_str(), s.length());

            std::vector<RCF::FilterPtr> filters;

            // with no transport or payload filters

            // prime the pump
            client.echo(byteBuffer0);
            Platform::OS::SleepMs(1000);

            {
                std::string s0 = byteBuffer0.string();
                gExpectAllocations = false;
                RCF::ByteBuffer byteBuffer1 = client.echo(byteBuffer0);
                gExpectAllocations = true;

                std::string s1 = byteBuffer1.string();
                RCF_CHECK(s0 == s1);
            }

            RCF::MarshalingProtocol mp = client.getClientStub().getMarshalingProtocol();

            client.getClientStub().setMarshalingProtocol(RCF::Mp_Rcf);
            client.echo(byteBuffer0);

            {
                util::Profile profile(transportDesc + "1000 calls, RCF marshaling, no dynamic allocations");
                gExpectAllocations = false;
                for(unsigned int i=0; i<1000; ++i)
                {
                    RCF::ByteBuffer byteBuffer1 = client.echo(byteBuffer0);
                }
                gExpectAllocations = true;
            }

#ifdef RCF_USE_PROTOBUF

            client.getClientStub().setMarshalingProtocol(RCF::Mp_Protobuf);
            client.echo(byteBuffer0);

            {
                util::Profile profile(transportDesc + "1000 calls, Protobuf marshaling, no dynamic allocations");
                gExpectAllocations = false;
                for(unsigned int i=0; i<1000; ++i)
                {
                    RCF::ByteBuffer byteBuffer1 = client.echo(byteBuffer0);
                }
                gExpectAllocations = true;
            }

#endif // RCF_USE_PROTOBUF

            client.getClientStub().setMarshalingProtocol(mp);

            // with both transport and payload filters

            if (transportFiltersSupported)
            {
                filters.clear();
                filters.push_back( RCF::FilterPtr( new RCF::XorFilter()));
                filters.push_back( RCF::FilterPtr( new RCF::XorFilter()));
                filters.push_back( RCF::FilterPtr( new RCF::XorFilter()));
                client.getClientStub().requestTransportFilters(filters);
            }

            filters.clear();
            filters.push_back( RCF::FilterPtr( new RCF::XorFilter()));
            filters.push_back( RCF::FilterPtr( new RCF::XorFilter()));
            filters.push_back( RCF::FilterPtr( new RCF::XorFilter()));
            client.getClientStub().setMessageFilters(filters);

            // prime the pump
            client.echo(byteBuffer0);
            Platform::OS::SleepMs(1000);

            for (int i=0; i<3; ++i)
            {
                // byteBuffer0 will be transformed in place, so we need to be a bit careful with the before/after comparison.
                // s0 and s1 will change on each pass.

                std::string s0 = byteBuffer0.string();
                gExpectAllocations = false;
                RCF::ByteBuffer byteBuffer1 = client.echo(byteBuffer0);
                gExpectAllocations = true;
                std::string s1 = byteBuffer1.string();
                RCF_CHECK(s0 == s1);
            }

            {
                util::Profile profile(transportDesc + "1000 calls, no dynamic allocations, 3 transport filters + 3 payload filters");
                gExpectAllocations = false;
                for(unsigned int i=0; i<1000; ++i)
                {
                    RCF::ByteBuffer byteBuffer1 = client.echo(byteBuffer0);
                }
                gExpectAllocations = true;
            }
           
            filters.clear();
#ifdef RCF_USE_ZLIB
            filters.push_back( RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
#endif
            client.getClientStub().setMessageFilters(filters);
            client.echo(byteBuffer0);

            {
                util::Profile profile(transportDesc + "1000 calls, no dynamic allocations, <zlib stateless> payload filters");
                gExpectAllocations = false;
                for(unsigned int i=0; i<1000; ++i)
                {
                    RCF::ByteBuffer byteBuffer1 = client.echo(byteBuffer0);
                }
                gExpectAllocations = true;
            }
           

            if (transportFiltersSupported)
            {
               
                filters.clear();
#ifdef RCF_USE_ZLIB
                filters.push_back( RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
#endif
#ifdef RCF_USE_OPENSSL
                filters.push_back( RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clCcert, clCpwd)));
#endif
                client.getClientStub().requestTransportFilters(filters);
               
                client.echo(byteBuffer0);

                util::Profile profile(transportDesc + "1000 calls, <zlib stateful><OpenSSL> transport filter");
                gExpectAllocations = false;
                for(unsigned int i=0; i<1000; ++i)
                {
                    RCF::ByteBuffer byteBuffer1 = client.echo(byteBuffer0);
                }
                gExpectAllocations = true;
            }

#ifdef BOOST_WINDOWS

            if (transportFiltersSupported)
            {
                filters.clear();
#ifdef RCF_USE_ZLIB
                filters.push_back( RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
#endif
                filters.push_back( RCF::FilterPtr( new RCF::NtlmFilter()));
                client.getClientStub().requestTransportFilters(filters);
                client.echo(byteBuffer0);

                util::Profile profile(transportDesc + "1000 calls, <zlib stateful><sspi ntlm> transport filter");
                gExpectAllocations = false;
                for(unsigned int i=0; i<1000; ++i)
                {
                    RCF::ByteBuffer byteBuffer1 = client.echo(byteBuffer0);
                }
                gExpectAllocations = true;
            }

#endif

            client.getClientStub().clearTransportFilters();

            {
                // try serialization (as opposed to marshaling) of ByteBuffer
                ContainsByteBuffer c1;
                c1.mByteBuffer = byteBuffer0;
                ContainsByteBuffer c2 = client.echo(c1);

                gExpectAllocations = false;
                c2.mByteBuffer.clear();
                c2 = client.echo(c1);
                gExpectAllocations = true;
            }

            // ByteBuffer serialization only supported for SF. This test crashes when run with Boost.Serialization.
            if (false)
            {
                // try serialization (not marshaling) of ByteBuffer with all serialization protocols
                for(int protocol=1; protocol<10; ++protocol)
                {
                    RCF::SerializationProtocol sp = RCF::SerializationProtocol(protocol);
                    if (RCF::isSerializationProtocolSupported(sp))
                    {
                        client.getClientStub().setSerializationProtocol(sp);

                        ContainsByteBuffer c1;
                        c1.mByteBuffer = byteBuffer0;
                        ContainsByteBuffer c2 = client.echo(c1);
                        RCF_CHECK(c2.mByteBuffer.getLength() == c1.mByteBuffer.getLength());

                        // will get memory allocations here when using boost serialization
                        c2.mByteBuffer.clear();
                        c2 = client.echo(c1);
                        RCF_CHECK(c2.mByteBuffer.getLength() == c1.mByteBuffer.getLength());
                    }
                }
            }
           
        }
        server.stop();
    }

    return boost::exit_success;
}
