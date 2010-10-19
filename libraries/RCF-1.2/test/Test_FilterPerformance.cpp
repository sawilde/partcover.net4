
#include <boost/lexical_cast.hpp>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/FilterService.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/OpenSslEncryptionFilter.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/ZlibCompressionFilter.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/AutoBuild.hpp>
#include <RCF/util/CommandLine.hpp>

#define Test_Performance Test_FilterPerformance
#include "Test_Performance.hpp"

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_FilterPerformance;

    util::CommandLineOption<std::string> scert("scert", RCF_TEMP_DIR "ssCert2.pem", "OpenSSL server certificate");
    util::CommandLineOption<std::string> spwd("spwd", "mt2316", "OpenSSL server certificate password");
    util::CommandLineOption<std::string> ccert("ccert", RCF_TEMP_DIR "ssCert1.pem", "OpenSSL client certificate");
    util::CommandLineOption<std::string> cpwd("cpwd", "mt2316", "OpenSSL client certificate password");
    util::CommandLineOption<int> calls( "calls", 1000, "number of calls");
    util::CommandLineOption<int> test( "test", 0, "which test to run, 0 to run them all");
    util::CommandLine::getSingleton().parse(argc, argv);

    std::string clientCertificateFile = ccert;
    std::string clientCertificateFilePassword = cpwd;
    std::string serverCertificateFile = scert;
    std::string serverCertificateFilePassword = spwd;

    int requestedTest = test;
    int currentTest = 0;

    // test all transports, against the standard serialization protocol
    for (unsigned int i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        std::string s0 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        s0 = s0 + s0;
        s0 = s0 + s0;
        s0 = s0 + s0;

        int serializationProtocol = 1;

        ++currentTest;
        if (requestedTest == 0 || requestedTest == currentTest)
        {
            std::string title = "Test " + boost::lexical_cast<std::string>(currentTest) + ": ";
            runPerformanceTest(
                title,
                clientTransportAutoPtr,
                serverTransportPtr,
                RCF::Twoway,
                s0,
                serializationProtocol,
                std::vector<RCF::FilterFactoryPtr>(),//filterFactories,
                std::vector<RCF::FilterPtr>(),
                std::vector<RCF::FilterPtr>(),
                calls);
        }
       
    }

    std::string s0 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    s0 = s0 + s0;
    s0 = s0 + s0;
    s0 = s0 + s0;

    int serializationProtocol = 1;

    // test compression and encryption filters, over TCP
    {
        std::vector<std::vector<RCF::FilterPtr> > payloadFilterChains;
        std::vector<RCF::FilterPtr> payloadFilterChain;

#ifdef RCF_USE_ZLIB
        payloadFilterChain.clear();
        payloadFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatefulCompressionFilter()) );
        payloadFilterChains.push_back(payloadFilterChain);

        payloadFilterChain.clear();
        payloadFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatelessCompressionFilter()) );
        payloadFilterChains.push_back(payloadFilterChain);
#endif

        std::vector<std::vector<RCF::FilterPtr> > transportFilterChains;
        std::vector<RCF::FilterPtr> transportFilterChain;

#ifdef RCF_USE_ZLIB
        transportFilterChain.clear();
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatefulCompressionFilter()) );
        transportFilterChains.push_back(transportFilterChain);

        transportFilterChain.clear();
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatelessCompressionFilter()) );
        transportFilterChains.push_back(transportFilterChain);
#endif
#ifdef RCF_USE_OPENSSL
        transportFilterChain.clear();
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)) );
        transportFilterChains.push_back(transportFilterChain);
#endif
#if defined(RCF_USE_ZLIB) && defined(RCF_USE_OPENSSL)
        transportFilterChain.clear();
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatelessCompressionFilter()) );
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)) );
        transportFilterChains.push_back(transportFilterChain);

        transportFilterChain.clear();
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatefulCompressionFilter()) );
        transportFilterChain.push_back( RCF::FilterPtr(new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)) );
        transportFilterChains.push_back(transportFilterChain);
#endif
        std::vector<RCF::FilterFactoryPtr> filterFactories;
#ifdef RCF_USE_ZLIB
        filterFactories.push_back(RCF::FilterFactoryPtr( new RCF::ZlibStatelessCompressionFilterFactory() ));
        filterFactories.push_back(RCF::FilterFactoryPtr( new RCF::ZlibStatefulCompressionFilterFactory() ));
#endif
#ifdef RCF_USE_OPENSSL
        filterFactories.push_back(RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(serverCertificateFile, serverCertificateFilePassword) ));
#endif

        // test payload filters
        for (unsigned int i=0; i<payloadFilterChains.size(); ++i)
        {
            RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
            RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
            RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

            ++currentTest;
            if (requestedTest == 0 || requestedTest == currentTest)
            {
                std::string title = "Test " + boost::lexical_cast<std::string>(currentTest) + ": ";
                runPerformanceTest(
                    title,
                    clientTransportAutoPtr,
                    serverTransportPtr,
                    RCF::Twoway,
                    s0,
                    serializationProtocol,
                    filterFactories,
                    payloadFilterChains[i],
                    std::vector<RCF::FilterPtr>(),
                    calls);
            }
        }

        // test transport filters
        for (unsigned int i=0; i<transportFilterChains.size(); ++i)
        {

#ifdef RCF_USE_BOOST_ASIO
            RCF::TransportFactoryPtr transportFactoryPtr( new RCF::TcpAsioTransportFactory() );
#elif defined BOOST_WINDOWS
            RCF::TransportFactoryPtr transportFactoryPtr( new RCF::TcpIocpTransportFactory() );
#else
#error no supported server transports
#endif

            RCF::TransportPair transports = transportFactoryPtr->createTransports();
            RCF::ServerTransportPtr serverTransportPtr( transports.first );
            RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

            ++currentTest;
            if (requestedTest == 0 || requestedTest == currentTest)
            {
                std::string title = "Test " + boost::lexical_cast<std::string>(currentTest) + ": ";
                runPerformanceTest(
                    title,
                    clientTransportAutoPtr,
                    serverTransportPtr,
                    RCF::Twoway,
                    s0,
                    serializationProtocol,
                    filterFactories,
                    std::vector<RCF::FilterPtr>(),
                    transportFilterChains[i],
                    calls);
            }
        }
    }

    // test oneway invocations
    {
        RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
        RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

        ++currentTest;
        if (requestedTest == 0 || requestedTest == currentTest)
        {
            std::string title = "Test " + boost::lexical_cast<std::string>(currentTest) + ": ";
            runPerformanceTest(
                title,
                clientTransportAutoPtr,
                serverTransportPtr,
                RCF::Oneway,
                s0,
                serializationProtocol,
                std::vector<RCF::FilterFactoryPtr>(),
                std::vector<RCF::FilterPtr>(),
                std::vector<RCF::FilterPtr>(),
                calls);
        }
    }

    return boost::exit_success;
}

/*
int test_main(int argc, char **argv)
{

    using namespace Test_FilterPerformance;

    std::vector<std::vector<RCF::FilterPtr> > transportFilterChains;
    std::vector<RCF::FilterPtr> transportFilterChain;

    transportFilterChain.clear();
    transportFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatefulCompressionFilter()) );
    transportFilterChains.push_back(transportFilterChain);

    transportFilterChain.clear();
    transportFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatelessCompressionFilter()) );
    transportFilterChains.push_back(transportFilterChain);

    transportFilterChain.clear();
    transportFilterChain.push_back( RCF::FilterPtr(new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)) );
    transportFilterChains.push_back(transportFilterChain);

    transportFilterChain.clear();
    transportFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatelessCompressionFilter()) );
    transportFilterChain.push_back( RCF::FilterPtr(new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)) );
    transportFilterChains.push_back(transportFilterChain);

    transportFilterChain.clear();
    transportFilterChain.push_back( RCF::FilterPtr(new RCF::ZlibStatefulCompressionFilter()) );
    transportFilterChain.push_back( RCF::FilterPtr(new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)) );
    transportFilterChains.push_back(transportFilterChain);

    std::vector<RCF::FilterFactoryPtr> filterFactories;
    filterFactories.push_back(RCF::FilterFactoryPtr( new RCF::ZlibStatelessCompressionFilterFactory() ));
    filterFactories.push_back(RCF::FilterFactoryPtr( new RCF::ZlibStatefulCompressionFilterFactory() ));
    //filterFactories.push_back(RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(serverCertificateFile, serverCertificateFilePassword) ));

    for (unsigned int i=0; i<transportFilterChains.size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr( new RCF::TcpIocpTransportFactory() );
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        {
            std::string title = "";
            runPerformanceTest(
                title,
                clientTransportAutoPtr,
                serverTransportPtr,
                RCF::Twoway,
                "asdf",
                1,
                filterFactories,
                std::vector<RCF::FilterPtr>(),
                transportFilterChains[i],
                100);
        }
    }

    return boost::exit_success;
}

*/
