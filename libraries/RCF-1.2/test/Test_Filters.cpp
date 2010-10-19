
#include <vector>

#include <boost/config.hpp>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/FilterService.hpp>
#include <RCF/ObjectFactoryService.hpp>
#include <RCF/OpenSslEncryptionFilter.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/UdpEndpoint.hpp>
#include <RCF/ZlibCompressionFilter.hpp>

#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Platform/OS/Sleep.hpp>

#include <RCF/test/TransportFactories.hpp>


namespace Test_Filters {

    RCF_BEGIN(X, "X")
        RCF_METHOD_R0(int, func)
        RCF_METHOD_V0(void, lockTransportFilters)
        RCF_METHOD_V0(void, unlockTransportFilters)
    RCF_END(X)

    class ServerImpl
    {
    public:
        ServerImpl() : ret(1)
        {}

        int func()
        {
            return ret++;
        }

        void lockTransportFilters()
        {
            RCF::getCurrentRcfSession().lockTransportFilters();
        }

        void unlockTransportFilters()
        {
            RCF::getCurrentRcfSession().unlockTransportFilters();
        }

    private:
        int ret;
    };

    // consider a filter chain to be removable if all its filters are removable
    bool isFilterChainRemovable(const std::vector<RCF::FilterPtr> &filterChain)
    {
        for (unsigned int i=0; i<filterChain.size(); ++i)
        {
            if (!filterChain[i]->getFilterDescription().getRemovable())
            {
                return false;
            }
        }
        return true;
    }

    void testServerErrorHandling()
    {
        RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
        RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

        RCF::RcfServer server(serverTransportPtr);

        ServerImpl serverImpl;
        server.bind( (X*) 0, serverImpl);

        server.start();

        {
            RcfClient<X> client(clientTransportAutoPtr->clone());
            try
            {
                client.getClientStub().requestTransportFilters( RCF::FilterPtr( new RCF::XorFilter()));
                int ret = client.func();
                RCF_CHECK(1==0);
            }
            catch(RCF::Exception &e)
            {
                RCF_CHECK(1==1);
            }
        }

        // add a filter service, but no filter factories
        RCF::FilterServicePtr filterServicePtr(new RCF::FilterService);
        server.addService( RCF::ServicePtr(filterServicePtr));

        {
            RcfClient<X> client(clientTransportAutoPtr->clone());
            try
            {
                bool ok = client.getClientStub().queryForTransportFilters( RCF::FilterPtr( new RCF::XorFilter()));
                RCF_CHECK(!ok);
                client.getClientStub().requestTransportFilters( RCF::FilterPtr( new RCF::XorFilter()));
                RCF_CHECK(1==0);
                int ret = client.func();
                RCF_CHECK(1==0);
            }
            catch(RCF::Exception &e)
            {
                RCF_CHECK(1==1);
            }
        }

        // add a filter factory
        filterServicePtr->addFilterFactory( RCF::FilterFactoryPtr( new RCF::XorFilterFactory()));

        {
            RcfClient<X> client(clientTransportAutoPtr->clone());
            try
            {
                bool ok = client.getClientStub().queryForTransportFilters( RCF::FilterPtr( new RCF::XorFilter()));
                RCF_CHECK(ok);
                client.getClientStub().requestTransportFilters( RCF::FilterPtr( new RCF::XorFilter()));
                int ret = client.func();
                RCF_CHECK(1==1);
            }
            catch(RCF::Exception &e)
            {
                RCF_CHECK(1==0);
            }
        }

        // check locking and unlocking of transport filters
        {
            RcfClient<X> client(clientTransportAutoPtr->clone());
            client.getClientStub().requestTransportFilters( RCF::FilterPtr( new RCF::XorFilter()));
            client.lockTransportFilters();

            try
            {
                client.getClientStub().requestTransportFilters();
                RCF_CHECK(1==0);
            }
            catch(RCF::RemoteException & e)
            {
                RCF_CHECK(1==1);
                RCF_CHECK(e.getErrorId() == RCF::RcfError_FiltersLocked);
            }
            
            try
            {
                client.getClientStub().requestTransportFilters(RCF::FilterPtr( new RCF::XorFilter()));
                RCF_CHECK(1==0);
            }
            catch(RCF::RemoteException & e)
            {
                RCF_CHECK(1==1);
                RCF_CHECK(e.getErrorId() == RCF::RcfError_FiltersLocked);
            }

            std::vector<RCF::FilterPtr> filters;
            filters.push_back(RCF::FilterPtr( new RCF::XorFilter()));
            filters.push_back(RCF::FilterPtr( new RCF::XorFilter()));
            filters.push_back(RCF::FilterPtr( new RCF::XorFilter()));
            try
            {
                client.getClientStub().requestTransportFilters(filters);
                RCF_CHECK(1==0);
            }
            catch(RCF::RemoteException & e)
            {
                RCF_CHECK(1==1);
                RCF_CHECK(e.getErrorId() == RCF::RcfError_FiltersLocked);
            }

            client.unlockTransportFilters();
            client.getClientStub().requestTransportFilters(filters);
            client.getClientStub().requestTransportFilters(RCF::FilterPtr());
        }
    }

    void testTransportFilters(
        const std::vector< RCF::FilterFactoryPtr > &filterFactories,
        const std::vector< std::vector<RCF::FilterPtr> > &filterChains,
        RCF::ServerTransportPtr serverTransportPtr,
        RCF::ClientTransportAutoPtr clientTransportAutoPtr)
    {

        RCF::RcfServer server(serverTransportPtr);

        RCF::FilterServicePtr filterServicePtr(new RCF::FilterService);
        for (unsigned int i=0; i<filterFactories.size(); ++i)
        {
            filterServicePtr->addFilterFactory(filterFactories[i]);
        }
        server.addService( RCF::ServicePtr(filterServicePtr));

        RCF::ObjectFactoryServicePtr objectFactoryServicePtr( new RCF::ObjectFactoryService(60, 60));
        objectFactoryServicePtr->bind( (X*) 0, (ServerImpl**) 0);
        server.addService( RCF::ServicePtr(objectFactoryServicePtr));

        ServerImpl serverImpl;
        server.bind( (X*) 0, serverImpl);

        server.start();

        int ret = 0;

        for (unsigned int i=0; i<filterChains.size(); ++i)
        {
            for (unsigned int j=0; j<2; ++j)
            {
                std::auto_ptr< RcfClient<X> > clientAutoPtr( new RcfClient<X>(clientTransportAutoPtr->clone()));
                int ret0 = 0;
                if (j == 0)
                {
                    bool ok = tryCreateRemoteObject<X>(*clientAutoPtr);
                    RCF_CHECK(ok);
                }
                int &myret = j == 0 ? ret0 : ret;
                RcfClient<X> &client = *clientAutoPtr;

                RCF_CHECK(client.func(RCF::Twoway) == ++myret);
                RCF_CHECK(client.func(RCF::Twoway) == ++myret);

                if (j > 0)
                {
                    std::for_each(filterChains[i].begin(), filterChains[i].end(), boost::bind(&RCF::Filter::reset, _1));
                }
                
                client.getClientStub().requestTransportFilters(filterChains[i]);

                RCF_CHECK(client.func(RCF::Twoway) == ++myret);
                RCF_CHECK(client.func(RCF::Twoway) == ++myret);

                client.getClientStub().disconnect();

                RCF_CHECK(client.func(RCF::Twoway) == ++myret);
                RCF_CHECK(client.func(RCF::Twoway) == ++myret);

                if (!isFilterChainRemovable(filterChains[i]))
                {
                    client.getClientStub().clearTransportFilters();
                }
                else
                {
                    client.getClientStub().requestTransportFilters(std::vector<RCF::FilterPtr>());
                }

                RCF_CHECK(client.func(RCF::Twoway) == ++myret);
                RCF_CHECK(client.func(RCF::Twoway) == ++myret);

                // reinstate the filter chain, have to reset the filters first
                std::for_each(filterChains[i].begin(), filterChains[i].end(), boost::bind(&RCF::Filter::reset, _1));
                client.getClientStub().requestTransportFilters(filterChains[i]);

                RCF_CHECK(client.func(RCF::Twoway) == ++myret);
                RCF_CHECK(client.func(RCF::Twoway) == ++myret);

                // check that auto reconnect works properly
                // can only expect this to work with filters that don't put any extra data on the wire
                if (isFilterChainRemovable(filterChains[i]))
                {
                    //server.stop();
                    server.close();
                    server.start();

                    RCF_CHECK(client.func(RCF::Twoway) == ++myret);
                    RCF_CHECK(client.func(RCF::Twoway) == ++myret);
                }
            }
        }
    }

    void testTransportFilters(
        const std::vector< RCF::FilterFactoryPtr > &filterFactories,
        const std::vector< std::vector<RCF::FilterPtr> > &filterChains)
    {
        for (unsigned int i=0; i<RCF::getTransportFactories().size(); ++i)
        {
            RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
            RCF::TransportPair transports = transportFactoryPtr->createTransports();
            RCF::ServerTransportPtr serverTransportPtr( transports.first );
            RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

            if (transportFactoryPtr->isConnectionOriented() && transportFactoryPtr->supportsTransportFilters())
            {
                // reset the client filter chains
                for(std::size_t j=0; j<filterChains.size(); ++j)
                {
                    std::for_each(
                        filterChains[j].begin(),
                        filterChains[j].end(),
                        boost::bind(&RCF::Filter::reset, _1));
                }

                testTransportFilters(
                    filterFactories,
                    filterChains,
                    serverTransportPtr,
                    clientTransportAutoPtr);
            }
        }
    }


    void testStatefulPayloadFilters(
        const std::vector< RCF::FilterFactoryPtr > &filterFactories,
        const std::vector< std::vector<RCF::FilterPtr> > &filterChains)
    {

        RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
        RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

        RCF::RcfServer server(serverTransportPtr);

        RCF::FilterServicePtr filterServicePtr(new RCF::FilterService);
        for (unsigned int i=0; i<filterFactories.size(); ++i)
        {
            filterServicePtr->addFilterFactory(filterFactories[i]);
        }
        server.addService( RCF::ServicePtr(filterServicePtr));

        ServerImpl serverImpl;
        server.bind( (X*) 0, serverImpl);

        server.start();

        int ret = 0;

        for (unsigned int i=0; i<filterChains.size(); ++i)
        {
            const std::vector<RCF::FilterPtr> &filterChain = filterChains[i];

            RcfClient<X> client(clientTransportAutoPtr->clone());
            ret = client.func(RCF::Twoway); RCF_CHECK( ret == 6*i+1 );

            client.getClientStub().setMessageFilters(filterChain);
            ret = client.func(RCF::Twoway); RCF_CHECK( ret == 6*i+2 );
            ret = client.func(RCF::Twoway); RCF_CHECK( ret == 6*i+3 );

            client.getClientStub().setMessageFilters(std::vector<RCF::FilterPtr>());
            ret = client.func(RCF::Twoway); RCF_CHECK( ret == 6*i+4 );

            client.getClientStub().setMessageFilters(filterChain);
            ret = client.func(RCF::Twoway); RCF_CHECK( ret == 6*i+5 );
            ret = client.func(RCF::Twoway); RCF_CHECK( ret == 6*i+6 );
        }

    }

    void testStatelessPayloadFilters(
        const std::vector< RCF::FilterFactoryPtr > &filterFactories,
        const std::vector< std::vector<RCF::FilterPtr> > &filterChains)
    {

#ifndef RCF_TEST_NO_UDP
        RCF::TransportPair transportPair = RCF::UdpTransportFactory().createTransports();
        RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;
#else
        RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
        RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;
#endif

        RCF::RcfServer server(serverTransportPtr);

        RCF::FilterServicePtr filterServicePtr(new RCF::FilterService);
        for (unsigned int i=0; i<filterFactories.size(); ++i)
        {
            filterServicePtr->addFilterFactory(filterFactories[i]);
        }
        server.addService( RCF::ServicePtr(filterServicePtr));

        ServerImpl serverImpl;
        server.bind( (X*) 0, serverImpl);

        server.start();

        int ret = 0;

        for (unsigned int i=0; i<filterChains.size(); ++i)
        {
            const std::vector<RCF::FilterPtr> &filterChain = filterChains[i];

            RcfClient<X> client(clientTransportAutoPtr->clone());
            ret = client.func(RCF::Twoway); RCF_CHECK( ret == 6*i+1 );

            client.getClientStub().setMessageFilters(filterChain);
            ret = client.func(RCF::Twoway); RCF_CHECK( ret == 6*i+2 );
            ret = client.func(RCF::Twoway); RCF_CHECK( ret == 6*i+3 );

            client.getClientStub().setMessageFilters(std::vector<RCF::FilterPtr>());
            ret = client.func(RCF::Twoway); RCF_CHECK( ret == 6*i+4 );

            client.getClientStub().setMessageFilters(filterChain);
            ret = client.func(RCF::Twoway); RCF_CHECK( ret == 6*i+5 );
            ret = client.func(RCF::Twoway); RCF_CHECK( ret == 6*i+6 );
        }

    }


    void testOnewayFilters(const std::vector<RCF::FilterPtr> &filterChain1, const std::vector<RCF::FilterPtr> &filterChain2)
    {
        std::string testData = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"#¤%&/()=";
        for (int i=0; i<256; ++i)
        {
            testData += char(i);
        }
        for (int i=0; i<1000; i++)
        {
            testData += rand() % 256;
        }

        std::string unfilteredData(testData);

        for (int i=0; i<10; ++i)
        {
            unfilteredData = unfilteredData + testData;
            bool ok = false;

            std::vector<RCF::ByteBuffer> unfilteredBuffers;
            std::vector<RCF::ByteBuffer> filteredBuffers;
            RCF::ByteBuffer unfilteredBuffer;
            RCF::ByteBuffer filteredBuffer;

            unfilteredBuffers.push_back( RCF::ByteBuffer(
                (char*) unfilteredData.c_str(),
                unfilteredData.length(),
                true));

            ok = RCF::filterData(unfilteredBuffers, filteredBuffers, filterChain1);
            RCF_CHECK(ok);
            RCF::copyByteBuffers(filteredBuffers, filteredBuffer);
            ok = RCF::unfilterData(filteredBuffer, unfilteredBuffer, static_cast<int>(unfilteredData.length()), filterChain2);
            RCF_CHECK(ok);
            std::string result(unfilteredBuffer.getPtr(), unfilteredBuffer.getLength());
            RCF_CHECK( result == unfilteredData );
        }

    }

    void testOnewayFilters(RCF::FilterPtr filterPtr1, RCF::FilterPtr filterPtr2)
    {
        std::vector<RCF::FilterPtr> filterChain1;
        filterChain1.push_back(filterPtr1);

        std::vector<RCF::FilterPtr> filterChain2;
        filterChain2.push_back(filterPtr2);

        testOnewayFilters(filterChain1, filterChain2);
    }

#if defined(RCF_USE_ZLIB)

    class Y
    {
    public:
        void ping(int i)
        {
            std::cout << i << std::endl;
        }
    };

    RCF_BEGIN(I_Y, "I_Y")
        RCF_METHOD_V1(void, ping, int);
    RCF_END(I_Y)

    void testOnewayZlib()
    {
        RCF::RcfServer server(RCF::TcpEndpoint(0));

        Y y;
        server.bind( (I_Y *) 0, y);

        RCF::FilterServicePtr filterServicePtr(new RCF::FilterService);

        filterServicePtr->addFilterFactory(
            RCF::FilterFactoryPtr(new RCF::ZlibStatefulCompressionFilterFactory));

        server.addService(filterServicePtr);

        server.start();

        int port = server.getIpServerTransport().getPort();

        RCF::TcpEndpoint tcpEndpoint(port);
        RcfClient<I_Y> client(tcpEndpoint);

        client.ping(0);

        client.getClientStub().requestTransportFilters(
            RCF::FilterPtr(new RCF::ZlibStatefulCompressionFilter()));

        for (int i = 0; i<1000; i++)
        {
            client.ping(RCF::Oneway, i);
        }

        client.ping(0);
    }

#endif

} // namespace Test_Filters

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

#if defined(__MINGW32__) && __GNUC__ < 3

    RCF_CHECK(1==0 && "Bizarre failures (crashes) with mingw 2.95.");
    return boost::exit_success;

#endif

    using namespace Test_Filters;

    util::CommandLineOption<std::string> scert("scert", RCF_TEMP_DIR "ssCert2.pem", "OpenSSL server certificate");
    util::CommandLineOption<std::string> spwd("spwd", "mt2316", "OpenSSL server certificate password");
    util::CommandLineOption<std::string> ccert("ccert", RCF_TEMP_DIR "ssCert1.pem", "OpenSSL client certificate");
    util::CommandLineOption<std::string> cpwd("cpwd", "mt2316", "OpenSSL client certificate password");

    util::CommandLine::getSingleton().parse(argc, argv);

    std::string clientCertificateFile = ccert;
    std::string clientCertificateFilePassword = cpwd;
    std::string serverCertificateFile = scert;
    std::string serverCertificateFilePassword = spwd;

    testOnewayFilters(RCF::FilterPtr( new RCF::IdentityFilter()), RCF::FilterPtr( new RCF::IdentityFilter()));
    testOnewayFilters(RCF::FilterPtr( new RCF::XorFilter()), RCF::FilterPtr( new RCF::XorFilter()));

#ifdef RCF_USE_ZLIB
    // TODO: figure out why this fails when using certain combinations of small buffer sizes for the zlib filters (<100 bytes)
    testOnewayFilters(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()), RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
    testOnewayFilters(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()), RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
#endif

    std::vector<RCF::FilterFactoryPtr> filterFactories;
    
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::IdentityFilterFactory()));
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::XorFilterFactory()));
#ifdef RCF_USE_ZLIB
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::ZlibStatelessCompressionFilterFactory()));
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::ZlibStatefulCompressionFilterFactory()));
#endif
#ifdef RCF_USE_OPENSSL
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(serverCertificateFile, serverCertificateFilePassword)));
#endif

    std::vector< std::vector<RCF::FilterPtr> > filterChains;
    std::vector<RCF::FilterPtr> filterChain;

    filterChains.clear();

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChains.push_back(filterChain);

#ifdef RCF_USE_ZLIB
    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
#endif

    filterChains.push_back(filterChain);

    testStatefulPayloadFilters(filterFactories, filterChains);

    filterChains.clear();

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChains.push_back(filterChain);

#ifdef RCF_USE_ZLIB
    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
#endif

    filterChains.push_back(filterChain);

    testStatelessPayloadFilters(filterFactories, filterChains);

    testServerErrorHandling();

    filterFactories.clear();
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::IdentityFilterFactory()));
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::XorFilterFactory()));
#ifdef RCF_USE_ZLIB
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::ZlibStatelessCompressionFilterFactory(7)));
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::ZlibStatefulCompressionFilterFactory(7)));
#endif
#ifdef RCF_USE_OPENSSL
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(serverCertificateFile, serverCertificateFilePassword)));
#endif

    filterChains.clear();

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChains.push_back(filterChain);

#ifdef RCF_USE_ZLIB
    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter(7)));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter(7)));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
    filterChains.push_back(filterChain);
#endif
#ifdef RCF_USE_OPENSSL
    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChains.push_back(filterChain);
#endif

    // watch out for rapidly growing stack size requirements on the client side, when chaining
    // filters together
    // ...

    testTransportFilters(filterFactories, filterChains);

    filterChains.clear();

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChains.push_back(filterChain);

#ifdef RCF_USE_ZLIB
    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter(7)));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter(7)));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
    filterChains.push_back(filterChain);
#endif
#ifdef RCF_USE_OPENSSL
    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChains.push_back(filterChain);
#endif

    testTransportFilters(filterFactories, filterChains);

    filterChains.clear();
    filterChain.clear();
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::XorFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChains.push_back(filterChain);

#ifdef RCF_USE_OPENSSL
    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
#ifndef __BORLANDC__
    // TODO: why does triple openssl fail on borland?
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
#endif
    filterChains.push_back(filterChain);
#endif

#ifdef RCF_USE_ZLIB
    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter(7)));
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter(7)));
    filterChain.push_back(RCF::FilterPtr( new RCF::IdentityFilter()));
    filterChains.push_back(filterChain);
#endif

    testTransportFilters(filterFactories, filterChains);

    filterChains.clear();

#if defined(RCF_USE_ZLIB) && defined(RCF_USE_OPENSSL)
    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter(7)));
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter(7)));
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChains.push_back(filterChain);

    testTransportFilters(filterFactories, filterChains);

    filterChains.clear();

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter(4096)));
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter(4096)));
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChains.push_back(filterChain);
#endif

    testTransportFilters(filterFactories, filterChains);

    filterFactories.clear();
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::IdentityFilterFactory()));
#ifdef RCF_USE_ZLIB
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::ZlibStatelessCompressionFilterFactory(4096)));
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::ZlibStatefulCompressionFilterFactory(4096)));
#endif
#ifdef RCF_USE_OPENSSL
    filterFactories.push_back( RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(serverCertificateFile, serverCertificateFilePassword)));
#endif

    filterChains.clear();

#if defined(RCF_USE_ZLIB) && defined(RCF_USE_OPENSSL)
    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter(7)));
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter(7)));
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChains.push_back(filterChain);
#endif

    testTransportFilters(filterFactories, filterChains);

    filterChains.clear();

#if defined(RCF_USE_ZLIB) && defined(RCF_USE_OPENSSL)
    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter(4096)));
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter(4096)));
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChains.push_back(filterChain);
#endif

    testTransportFilters(filterFactories, filterChains);

    filterChains.clear();

#if defined(RCF_USE_ZLIB) && defined(RCF_USE_OPENSSL)

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChains.push_back(filterChain);

#if defined(BOOST_WINDOWS) && !defined(__BORLANDC__)
    // TODO: why does this test fail on gcc (Solaris, Linux, prob'ly Cygwin, but not mingw), and on borland ?
    std::cout << "testing ->ssl->stateful compression-> transport filter" << std::endl;
    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
    filterChains.push_back(filterChain);
#endif

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChains.push_back(filterChain);

    filterChain.clear();
    filterChain.push_back(RCF::FilterPtr( new RCF::OpenSslEncryptionFilter(clientCertificateFile, clientCertificateFilePassword)));
    filterChain.push_back(RCF::FilterPtr( new RCF::ZlibStatelessCompressionFilter()));
    filterChains.push_back(filterChain);

#endif

    testTransportFilters(filterFactories, filterChains);

#if defined(RCF_USE_ZLIB)

    testOnewayZlib();

#endif

    return boost::exit_success;

}
