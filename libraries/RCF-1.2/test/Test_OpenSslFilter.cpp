
#ifdef RCF_USE_OPENSSL

#include <RCF/test/TestMinimal.hpp>

#include <RCF/FilterService.hpp>
#include <RCF/Idl.hpp>
#include <RCF/OpenSslEncryptionFilter.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/TcpEndpoint.hpp>

#include <RCF/test/TransportFactories.hpp>

namespace Test_OpenSslFilter {

    class Echo
    {
    public:
        std::string echo(const std::string &s)
        {
            return s;
        }
    };

    RCF_BEGIN(I_Echo, "I_Echo")
        RCF_METHOD_R1(std::string, echo, const std::string &)
    RCF_END(I_Echo)

    bool cbVerify(RCF::OpenSslEncryptionFilter &filter, bool ret)
    {
        SSL *ssl = filter.getSSL();
        SSL_CTX *ssl_ctx = filter.getCTX();
        RCF_CHECK(ssl);
        RCF_CHECK(ssl_ctx);

        // run whatever checks we want to here
        // ,,,

        return ret;
    }

}

int test_main(int argc, char **argv)
{
    RCF::RcfInitDeinit rcfInitDeinit;
    printTestHeader(__FILE__);

    using namespace Test_OpenSslFilter;

    util::CommandLine::getSingleton().parse(argc, argv);

    {

        // test involves:
        // 1. server using CA cert to verify client cert occasionally
        // 1. server using CA cert to verify client cert on demand (via session)
        // 2. client using CA cert to verify server always
        // 3. callback to client code for unverified server certs
        // 4. customizing cipher list
        // 5. DONE: using password encrypted certificates

        std::string s0              = "sample text";

        std::string certA           = RCF_TEMP_DIR "certA.pem";
        std::string certPassA       = "";
        std::string certB           = RCF_TEMP_DIR "certB.pem";
        std::string certPassB       = "";
        std::string caCertA         = RCF_TEMP_DIR "caCertA.pem";
        std::string caCertB         = RCF_TEMP_DIR "caCertB.pem";
        std::string ssCert1         = RCF_TEMP_DIR "ssCert1.pem";
        std::string ssCertPass1     = "mt2316";
        std::string ssCert2         = RCF_TEMP_DIR "ssCert2.pem";
        std::string ssCertPass2     = "mt2316";
        std::string ciphers         = "";

        RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
        RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

        RCF::FilterServicePtr filterServicePtr(new RCF::FilterService());

        Echo echo;
        RCF::RcfServer server(serverTransportPtr);
        server.bind( (I_Echo*) 0, echo);
        server.addService( RCF::ServicePtr(filterServicePtr));
        server.start();

        RcfClient<I_Echo> client(clientTransportAutoPtr);
        client.getClientStub().setRemoteCallTimeoutMs(1000*3600);

        typedef boost::shared_ptr<RCF::OpenSslEncryptionFilter> OpenSslEncryptionFilterPtr;
        OpenSslEncryptionFilterPtr filterPtr;

        // not encrypted
        RCF_CHECK(client.echo(s0) == s0);

        // encrypted

        //--------------------------
        // client verifies server certificate

        filterServicePtr->addFilterFactory(
            RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(
            certA,
            certPassA,
            "",
            ciphers)));

        filterPtr.reset( new RCF::OpenSslEncryptionFilter(
            "",
            "",
            caCertA,
            ciphers));

        client.getClientStub().requestTransportFilters(filterPtr);
        RCF_CHECK(client.echo(s0) == s0);

        //--------------------------
        // client fails to verify server certificate (wrong CA)

        filterServicePtr->addFilterFactory(
            RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(
            certB,
            certPassB,
            "",
            ciphers)));

        filterPtr.reset( new RCF::OpenSslEncryptionFilter(
            "",
            "",
            caCertA,
            ciphers));

        client.getClientStub().requestTransportFilters(filterPtr);
        try
        {
            client.echo(s0);
            RCF_CHECK(1==0);
        }
        catch(const RCF::Exception &e)
        {
            RCF_CHECK(1==1);
            RCF_CHECK(e.getErrorId() == RCF::RcfError_SslCertVerification);
            client.getClientStub().clearTransportFilters();
        }

        //--------------------------
        // client overrides verification of server certificate (wrong CA) (accepting)

        filterServicePtr->addFilterFactory(
            RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(
            certB,
            certPassB,
            "",
            ciphers)));

        filterPtr.reset( new RCF::OpenSslEncryptionFilter(
            "",
            "",
            caCertA,
            ciphers,
            boost::bind(cbVerify, _1, true)));

        client.getClientStub().requestTransportFilters(filterPtr);
        RCF_CHECK(client.echo(s0) == s0);

        //--------------------------
        // client overrides verification of server certificate (wrong CA) (not accepting)

        filterServicePtr->addFilterFactory(
            RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(
            certB,
            certPassB,
            "",
            ciphers)));

        filterPtr.reset( new RCF::OpenSslEncryptionFilter(
            "",
            "",
            caCertA,
            ciphers,
            boost::bind(cbVerify, _1, false)));

        client.getClientStub().requestTransportFilters(filterPtr);
        try
        {
            client.echo(s0);
            RCF_CHECK(1==0);
        }
        catch(const RCF::Exception &e)
        {
            RCF_CHECK(1==1);
            RCF_CHECK(e.getErrorId() == RCF::RcfError_SslCertVerification);
            client.getClientStub().clearTransportFilters();
        }

        //--------------------------
        // client fails to verify server certificate (self-signed)

        filterServicePtr->addFilterFactory(
            RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(
            ssCert1,
            ssCertPass1,
            "",
            ciphers)));

        filterPtr.reset( new RCF::OpenSslEncryptionFilter(
            "",
            "",
            caCertA,
            ciphers));

        client.getClientStub().requestTransportFilters(filterPtr);
        try
        {
            client.echo(s0);
            RCF_CHECK(1==0);
        }
        catch(const RCF::Exception &e)
        {
            RCF_CHECK(1==1);
            RCF_CHECK(e.getErrorId() == RCF::RcfError_SslCertVerification);
            client.getClientStub().clearTransportFilters();
        }

        //--------------------------
        // server and client verify each others certificate

        filterServicePtr->addFilterFactory(
            RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(
            certA,
            certPassA,
            caCertB,
            ciphers)));

        filterPtr.reset( new RCF::OpenSslEncryptionFilter(
            certB,
            certPassB,
            caCertA,
            ciphers));

        client.getClientStub().requestTransportFilters(filterPtr);
        RCF_CHECK(client.echo(s0) == s0);

        //--------------------------
        // server fails to verify client certificate (self-signed)

        filterServicePtr->addFilterFactory(
            RCF::FilterFactoryPtr( new RCF::OpenSslEncryptionFilterFactory(
            certA,
            certPassA,
            caCertB,
            ciphers)));

        filterPtr.reset( new RCF::OpenSslEncryptionFilter(
            ssCert2,
            ssCertPass2,
            caCertA,
            ciphers));

        client.getClientStub().requestTransportFilters(filterPtr);
        try
        {
            client.echo(s0);
            RCF_CHECK(1==0);
        }
        catch(const RCF::Exception &e)
        {
            RCF_CHECK(1==1);
            client.getClientStub().clearTransportFilters();
        }

    }

    return boost::exit_success;
}

#else

#include <RCF/test/TestMinimal.hpp>

int test_main(int, char **)
{
    return boost::exit_success;
}

#endif
