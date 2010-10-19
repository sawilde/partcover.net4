
#include <string>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <RCF/test/TestMinimal.hpp>
#include <RCF/test/Usernames.hpp>

#include <RCF/FilterService.hpp>
#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/Schannel.hpp>
#include <RCF/SspiFilter.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/ZlibCompressionFilter.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/CommandLine.hpp>

#include <RCF/util/AutoBuild.hpp>

// towupper()
#include <ctype.h>
#include <wctype.h>

namespace Test_SspiFilter {

    // TODO: auto detect the domain, and run this test with domain and without.
    bool iHaveADomain = true;

    typedef RCF::tstring tstring;

    // case insensitive comparison, for Windows user names
    bool icmpCh(char ch1, char ch2)
    {
        return toupper(ch1) == toupper(ch2);
    }

    bool icmpWch(wchar_t ch1, wchar_t ch2)
    {
        return towupper(ch1) == towupper(ch2);
    }

    bool icmp(const std::string &s1, const std::string &s2)
    {
        return
            s1.size() == s2.size() &&
            std::equal(s1.begin(), s1.end(), s2.begin(), icmpCh);
    }

#ifdef UNICODE

    bool icmp(const std::wstring &s1, const std::wstring &s2)
    {
        return
            s1.size() == s2.size() &&
            std::equal(s1.begin(), s1.end(), s2.begin(), icmpWch);
    }

#endif

    bool isSspiFilter(RCF::FilterPtr filterPtr)
    {
        return boost::dynamic_pointer_cast<RCF::SspiFilter>(filterPtr);
    }

    class Echo
    {
    public:
        tstring echo(const tstring &s)
        {
            return s;
        }

        tstring getUserName()
        {
            std::vector<RCF::FilterPtr> filters;
            RCF::getCurrentRcfSession().getTransportFilters(filters);

            std::vector<RCF::FilterPtr>::const_iterator iter = std::find_if(
                filters.begin(),
                filters.end(),
                isSspiFilter);

            if (iter != filters.end())
            {
                RCF::SspiFilterPtr sspiFilterPtr = 
                    boost::dynamic_pointer_cast<RCF::SspiFilter>(*iter);

                boost::shared_ptr<RCF::SchannelServerFilter> schannelFilter =
                    boost::dynamic_pointer_cast<RCF::SchannelServerFilter>(*iter);

                if (schannelFilter)
                {
                    return RCF::getMyUserName();
                }
                else if (sspiFilterPtr)
                {
                    RCF::SspiImpersonator impersonator(sspiFilterPtr);
                    bool ok = impersonator.impersonate();
                    return RCF::getMyUserName();
                }
                else
                {
                    return RCF::getMyUserName();
                }
            }
            else
            {
                return RCF::getMyUserName();
            }
        }

        tstring getWin32PipeUserName()
        {
            RCF::Win32NamedPipeImpersonator impersonator;
            RCF::tstring myUserName = RCF::getMyUserName();
            return myUserName;
        }
    };

    RCF_BEGIN(I_Echo, "I_Echo")
        RCF_METHOD_R1(tstring, echo, const tstring &)
        RCF_METHOD_R0(tstring, getUserName)
        RCF_METHOD_R0(tstring, getWin32PipeUserName)
    RCF_END(I_Echo)

    typedef boost::function1<
        RCF::FilterPtr, RCF::SspiFilter::QualityOfProtection>
            CreateFilter;

    class CreateFilterInfo
    {
    public:
        CreateFilterInfo()
        {}

        CreateFilterInfo(
            CreateFilter createFilter,
            const tstring &username,
            const std::string &desc) :
                mCreateFilter(createFilter),
                mUsername(username),
                mDesc(desc)
        {}

        CreateFilter mCreateFilter;
        tstring mUsername;
        std::string mDesc;
    };

    RCF::FilterPtr createSspiNtlmFilter(
        const tstring &userName,
        const tstring &password,
        const tstring &domain,
        const tstring &targetName,
        RCF::SspiFilter::QualityOfProtection qop)
    {
        RCF_UNUSED_VARIABLE(targetName);
        return RCF::FilterPtr(
            new RCF::NtlmFilter(
                userName,
                password,
                domain,
                qop));
    }

    RCF::FilterPtr createSspiKerberosFilter(
        const tstring &userName,
        const tstring &password,
        const tstring &domain,
        const tstring &targetName,
        RCF::SspiFilter::QualityOfProtection qop)
    {
        return RCF::FilterPtr(
            new RCF::KerberosFilter(
                userName,
                password,
                domain,
                targetName,
                qop));
    }

    RCF::FilterPtr createSspiNegotiateFilter(
        const tstring &userName,
        const tstring &password,
        const tstring &domain,
        const tstring &targetName,
        RCF::SspiFilter::QualityOfProtection qop)
    {
        return RCF::FilterPtr(
            new RCF::SspiNegotiateFilter(
                userName,
                password,
                domain,
                targetName,
                qop));
    }

    void validateCert(RCF::SchannelFilter & schannelFilter)
    {
        PCCERT_CONTEXT pRemoteCert = schannelFilter.getServerCertificate();
    }

    RCF::FilterPtr createSchannelFilter(RCF::SspiFilter::QualityOfProtection qop)
    {
        boost::shared_ptr<RCF::SchannelFilter> filterPtr( 
            new RCF::SchannelFilter() );

        filterPtr->setManualCertValidation( boost::bind(validateCert, _1) );

        return filterPtr;
    }

    RCF::PfxCertificate * gpPfxCertificate;

    // server side setup (which filters will be available)
    void setupTest(
        int k,
        RCF::FilterServicePtr filterServicePtr,
        std::vector<CreateFilterInfo> &factoriesOk,
        std::vector<CreateFilterInfo> &factoriesNotOk)
    {

        tstring NegotiablePackages = iHaveADomain ?
            RCF_T("Kerberos, NTLM") :
            RCF_T("NTLM");

        std::vector<int> filterIds;

        tstring localUsername;
        tstring localPassword;
        tstring localPasswordBad;

        tstring adUsername;
        tstring adPassword;
        //tstring adPasswordBad;
        tstring adDomain;

        {
            std::string localUsername_;
            std::string localPassword_;
            std::string localPasswordBad_;

            std::string adUsername_;
            std::string adPassword_;
            //std::string adPasswordBad_;
            std::string adDomain_;

            Usernames usernames;
            if (!getUsernames(usernames))
            {
                std::string whichFile = RCF_TEMP_DIR "sspi.txt";
                throw std::runtime_error((std::string("File not found: ") + whichFile).c_str());
            }

            localUsername       = util::toTstring(usernames.mLocalUsername);
            localPassword       = util::toTstring(usernames.mLocalPassword);
            localPasswordBad    = util::toTstring(usernames.mLocalPasswordBad);

            adUsername          = util::toTstring(usernames.mAdUsername);
            adPassword          = util::toTstring(usernames.mAdPassword);
            adDomain            = util::toTstring(usernames.mAdDomain);

            if (localUsername.empty() || adUsername.empty())
            {
                RCF_CHECK(1 == 0);
            }
        }

        tstring myUserName = RCF::getMyUserName();
        tstring myDomain = RCF::getMyDomain();
        tstring targetSpn = myDomain + RCF_T("\\") + myUserName;

        CreateFilterInfo implicitNtlm(
            boost::bind(&createSspiNtlmFilter, RCF_T(""), RCF_T(""), RCF_T(""), targetSpn, _1),
            myUserName,
            "NTLM - implicit credentials");

        CreateFilterInfo explicitNtlm(
            boost::bind(&createSspiNtlmFilter, localUsername, localPassword, RCF_T(""), targetSpn, _1),
            localUsername,
            "NTLM - explicit credentials");

        CreateFilterInfo explicitNtlmBad(
            boost::bind(&createSspiNtlmFilter, localUsername, localPasswordBad, RCF_T(""), targetSpn, _1),
            localUsername,
            "NTLM - incorrect explicit credentials");

        CreateFilterInfo implicitKerberos(
            boost::bind(&createSspiKerberosFilter, RCF_T(""), RCF_T(""), RCF_T(""), targetSpn, _1),
            myUserName,
            "Kerberos - implicit credentials");

        CreateFilterInfo explicitKerberos(
            boost::bind(&createSspiKerberosFilter, adUsername, adPassword, adDomain, targetSpn, _1),
            adUsername,
            "Kerberos - explicit credentials");

        //CreateFilterInfo explicitKerberosBad(
        //    boost::bind(&createSspiKerberosFilter, adUsername, adPasswordBad, adDomain, targetSpn, _1),
        //    localUsername,
        //    "Kerberos - incorrect explicit credentials");

        CreateFilterInfo implicitNegotiateNtlm(
            boost::bind(&createSspiNegotiateFilter, RCF_T(""), RCF_T(""), RCF_T(""), RCF_T(""), _1),
            myUserName,
            "Negotiate (NTLM) - implicit credentials");

        CreateFilterInfo implicitNegotiateKerberos(
            boost::bind(&createSspiNegotiateFilter, RCF_T(""), RCF_T(""), RCF_T(""), targetSpn, _1),
            myUserName,
            "Negotiate (Kerberos) - implicit credentials");

        CreateFilterInfo explicitNegotiateNtlm(
            boost::bind(&createSspiNegotiateFilter, localUsername, localPassword, RCF_T(""), RCF_T(""), _1),
            localUsername,
            "Negotiate (NTLM) - explicit credentials");

        CreateFilterInfo explicitNegotiateKerberos(
            boost::bind(&createSspiNegotiateFilter, adUsername, adPassword, adDomain, targetSpn, _1),
            adUsername,
            "Negotiate (Kerberos) - explicit credentials");

        CreateFilterInfo explicitNegotiateNtlmBad(
            boost::bind(&createSspiNegotiateFilter, localUsername, localPasswordBad, RCF_T(""), RCF_T(""), _1),
            localUsername,
            "Negotiate (NTLM) - incorrect explicit credentials");

        //CreateFilterInfo explicitNegotiateKerberosBad(
        //    boost::bind(&createSspiNegotiateFilter, adUsername, adPasswordBad, adDomain, targetSpn, _1),
        //    localUsername,
        //    "Negotiate (Kerberos) - incorrect explicit credentials ");

        CreateFilterInfo implicitSchannel(
            boost::bind(createSchannelFilter, _1),
            myUserName,
            "Schannel - no client certificate");

        std::cout << std::endl;

        switch (k)
        {
        case 0:
            // restrict authentication to Kerberos

            std::cout << "Server side - only Kerberos authentication" << std::endl;

            filterIds.clear();
            filterIds.push_back(RCF::RcfFilter_SspiKerberos);
            filterIds.push_back(RCF::RcfFilter_SspiNegotiate);
            filterServicePtr->addFilterFactory(
                RCF::FilterFactoryPtr( new RCF::KerberosFilterFactory()),
                filterIds);

            if (iHaveADomain)
            {
                factoriesOk.push_back(implicitKerberos);
                factoriesOk.push_back(explicitKerberos);

                //factoriesOk.push_back(implicitNegotiateKerberos);
                //factoriesOk.push_back(explicitNegotiateKerberos);
            }
            else
            {
                factoriesNotOk.push_back(implicitKerberos);
                factoriesNotOk.push_back(explicitKerberos);
                //factoriesNotOk.push_back(implicitNegotiateKerberos);
                //factoriesNotOk.push_back(explicitNegotiateKerberos);
            }

            factoriesNotOk.push_back(implicitNtlm);
            factoriesNotOk.push_back(explicitNtlm);

            //factoriesNotOk.push_back(implicitNegotiateNtlm);
            //factoriesNotOk.push_back(explicitNegotiateNtlm);

            //factoriesNotOk.push_back(explicitKerberosBad);
            factoriesNotOk.push_back(explicitNtlmBad);

            break;

        case 1:

            std::cout << "Server side - only NTLM authentication" << std::endl;

            // restrict authentication to NTLM
            filterIds.clear();
            filterIds.push_back(RCF::RcfFilter_SspiNtlm);
            filterIds.push_back(RCF::RcfFilter_SspiNegotiate);

            filterServicePtr->addFilterFactory(
                RCF::FilterFactoryPtr( new RCF::NtlmFilterFactory()),
                filterIds);

            factoriesOk.push_back(implicitNtlm);

            // TODO: Not testing explicit NTLM authentication currently. Need 
            // to get a user setup with the appropriate privileges.
            //factoriesOk.push_back(explicitNtlm);

            //factoriesOk.push_back(implicitNegotiateNtlm);
            //factoriesOk.push_back(explicitNegotiateNtlm);

            factoriesNotOk.push_back(implicitKerberos);
            factoriesNotOk.push_back(explicitKerberos);

            //factoriesNotOk.push_back(implicitNegotiateKerberos);
            //factoriesNotOk.push_back(explicitNegotiateKerberos);

            //factoriesNotOk.push_back(explicitKerberosBad);
            factoriesNotOk.push_back(explicitNtlmBad);

            break;

        case 2:
            // allow either authentication (through a single Negotiate filter)

            std::cout << "Server side - NTLM or Kerberos authentication (Negotiate)" << std::endl;

            filterIds.clear();
            filterIds.push_back(RCF::RcfFilter_SspiKerberos);
            filterIds.push_back(RCF::RcfFilter_SspiNtlm);
            filterIds.push_back(RCF::RcfFilter_SspiNegotiate);

            filterServicePtr->addFilterFactory(
                RCF::FilterFactoryPtr( new RCF::NegotiateFilterFactory(NegotiablePackages)),
                filterIds);

            // TODO: can't seem to make NTLM connections here, at least not on AD system

            //factoriesOk.push_back(implicitNtlm);
            //factoriesOk.push_back(explicitNtlm);

            //factoriesOk.push_back(implicitNegotiateNtlm);
            //factoriesOk.push_back(explicitNegotiateNtlm);

            if (iHaveADomain)
            {
                 //factoriesOk.push_back(implicitKerberos);
                 //factoriesOk.push_back(explicitKerberos);
                 factoriesOk.push_back(implicitNegotiateKerberos);
                 factoriesOk.push_back(explicitNegotiateKerberos);
            }
            else
            {
                //factoriesNotOk.push_back(implicitKerberos);
                //factoriesNotOk.push_back(explicitKerberos);
                factoriesNotOk.push_back(implicitNegotiateKerberos);
                factoriesNotOk.push_back(explicitNegotiateKerberos);
            }

            //factoriesNotOk.push_back(explicitKerberosBad);
            factoriesNotOk.push_back(explicitNtlmBad);
            //factoriesNotOk.push_back(explicitNegotiateKerberosBad);
            factoriesNotOk.push_back(explicitNegotiateNtlmBad);

            break;

        case 3:
            // allow either authentication (through distinct filters)

            std::cout << "Server side - Kerberos or NTLM authentication (distinct)" << std::endl;

            filterServicePtr->addFilterFactory(
                RCF::FilterFactoryPtr( new RCF::NtlmFilterFactory()));

            filterServicePtr->addFilterFactory(
                RCF::FilterFactoryPtr( new RCF::KerberosFilterFactory()));

            filterServicePtr->addFilterFactory(
                RCF::FilterFactoryPtr(
                    new RCF::NegotiateFilterFactory(NegotiablePackages)));

            factoriesOk.push_back(implicitNtlm);

            // TODO: Not testing explicit NTLM authentication currently. Need 
            // to get a user setup with the appropriate privileges.
            //factoriesOk.push_back(explicitNtlm);

            //factoriesOk.push_back(implicitNegotiateNtlm);
            //factoriesOk.push_back(explicitNegotiateNtlm);

            if (iHaveADomain)
            {
                factoriesOk.push_back(implicitKerberos);
                factoriesOk.push_back(explicitKerberos);
                //factoriesOk.push_back(implicitNegotiateKerberos);
                //factoriesOk.push_back(explicitNegotiateKerberos);
            }
            else
            {
                factoriesNotOk.push_back(implicitKerberos);
                factoriesNotOk.push_back(explicitKerberos);
                factoriesNotOk.push_back(implicitNegotiateKerberos);
                factoriesNotOk.push_back(explicitNegotiateKerberos);
            }

            //factoriesNotOk.push_back(explicitKerberosBad);
            factoriesNotOk.push_back(explicitNtlmBad);

            break;

        case 4:
            
            std::cout << "Server side - Schannel" << std::endl;

#if defined(_MSC_VER) && _MSC_VER == 1200

            // PfxCertificate doesn't build on vc6.
            // ...

#else

            delete gpPfxCertificate;
            gpPfxCertificate = new RCF::PfxCertificate(
                RCF_TEMP_DIR "certA.p12", 
                RCF_T(""),
                RCF_T("localhost"), 
                CERT_FIND_SUBJECT_STR);

            filterServicePtr->addFilterFactory(
                RCF::FilterFactoryPtr( new RCF::SchannelFilterFactory(
                    gpPfxCertificate->getCertContext())));

            factoriesOk.push_back(implicitSchannel);

#endif

            break;

        default:
            RCF_ASSERT(0)(k);
        }

    }

} // namespace Test_SspiFilter


// test named pipe impersonation with explicit usernames and passwords
void testNamedPipeImpersonation()
{
    using namespace Test_SspiFilter;

    RCF::Win32NamedPipeTransportFactory factory;
    RCF::TransportPair transports = factory.createTransports();
    RCF::ServerTransportPtr serverTransportPtr( transports.first );
    RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

    RCF::RcfServer server(serverTransportPtr);
    server.start();

    Echo echo;
    server.bind( (I_Echo*) 0, echo);

    Usernames usernames;
    if (getUsernames(usernames))
    {
        std::string userName = usernames.mLocalUsername;
        std::string password = usernames.mLocalPassword;

        HANDLE handle = 0;

        BOOL ok = LogonUserA(
            (char *) userName.c_str(),
            "",
            (char *) password.c_str(),
            LOGON32_LOGON_INTERACTIVE,
            LOGON32_PROVIDER_DEFAULT,
            &handle);

        DWORD dwErr = GetLastError();
        RCF_CHECK(ok);

        ok = ImpersonateLoggedOnUser(handle);
        dwErr = GetLastError();
        RCF_CHECK(ok);

        // Note: if this program is run under administrator credentials, then the
        // user we are impersonating needs to be an administrator as well, to 
        // be able to connect to the pipe.

        RcfClient<I_Echo> client(clientTransportAutoPtr);
        tstring s = client.getWin32PipeUserName();
        RCF_CHECK(s == util::toTstring(userName));

        ok = RevertToSelf();
        dwErr = GetLastError();
        RCF_CHECK(ok);
    }
    else
    {
        RCF_CHECK(1==0 && "Couldn't load usernames and passwords");
    }
}

int test_main(int argc, char **argv)
{
    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

#if defined(__MINGW32__) && __GNUC__ == 3 && __GNUC_MINOR__ <= 2

    RCF_CHECK( 1== 0 && "this test crashes on explicit-incorrect-NTLM to NTLM, with mingw 3.2 , for unknown reasons");
    return boost::exit_success;

#endif

    testNamedPipeImpersonation();

    std::string myUserName = util::toString(RCF::getMyUserName());
    std::string myDomain = util::toString(RCF::getMyDomain());

    std::cout
        << "My user name: " << myUserName
        << std::endl
        << "My domain: " <<  myDomain
        << std::endl
        << std::endl;

    using namespace Test_SspiFilter;

    util::CommandLineOption<int> clClientTimeout(
        "timeout",
        20, "client timeout (s)");

    util::CommandLine::getSingleton().parse(argc, argv);

    tstring usernameOrig = RCF::getMyUserName();

    RCF::setDefaultRemoteCallTimeoutMs(clClientTimeout.get() * 1000);

    /*
    // This code doesn't work but it would be nice if it did.
    {
        tstring s;
        tstring s0 = RCF_T("asdfasdf");

        RCF::FilterServicePtr filterServicePtr(new RCF::FilterService());
        RCF::FilterPtr filterPtr( new RCF::SspiNegotiateClientFilter(RCF_T("")));
        filterServicePtr->addFilterFactory(RCF::FilterFactoryPtr( new RCF::SspiNegotiateFilterFactory()));
        filterServicePtr->addFilterFactory(RCF::FilterFactoryPtr( new RCF::KerberosFilterFactory()));

        Echo echo;
        RCF::RcfServer server( RCF::TcpEndpoint(50001));
        server.bind<I_Echo>(echo);
        server.addService( RCF::ServicePtr(filterServicePtr));
        server.start();

        RcfClient<I_Echo> client( RCF::TcpEndpoint("127.0.0.1", 50001));

        // not encrypted
        s = client.echo(s0);

        // kerberos
        client.getClientStub().requestTransportFilters( RCF::FilterPtr(
            new RCF::KerberosFilter("towersoft\\jarl")));
        s = client.echo(s0);

        // negotiate kerberos
        client.getClientStub().requestTransportFilters( RCF::FilterPtr(
            new RCF::SspiNegotiateFilter("towersoft\\jarl")));
        s = client.echo(s0);

        // negotiate ntlm
        client.getClientStub().requestTransportFilters( RCF::FilterPtr(
            new RCF::SspiNegotiateFilter(RCF_T(""))));
        s = client.echo(s0);
    }
    */

    // simple warm-up test (implicit NTLM authentication)
    {
        tstring s;
        tstring s0 = RCF_T("asdfasdf");

        RCF::FilterServicePtr filterServicePtr(new RCF::FilterService());
        filterServicePtr->addFilterFactory(RCF::FilterFactoryPtr( new RCF::NtlmFilterFactory()));

        Echo echo;
        RCF::RcfServer server( RCF::TcpEndpoint(50001));
        server.bind( (I_Echo*) 0, echo);
        server.addService( RCF::ServicePtr(filterServicePtr));
        server.start();

        RcfClient<I_Echo> client( RCF::TcpEndpoint("127.0.0.1", 50001));

        // not encrypted
        s = client.echo(s0);

        // encrypted
        client.getClientStub().requestTransportFilters( RCF::FilterPtr(
            new RCF::NtlmFilter()));

        s = RCF_T("");
        s = client.echo(s0);
        RCF_CHECK(s == s0);

        s = RCF_T("");
        s = client.echo(s0);
        RCF_CHECK(s == s0);

        s = client.getUserName();
        RCF_CHECK(s == RCF::getMyUserName());
    }

    for (unsigned int i=0; i<RCF::getTransportFactories().size(); ++i)
    {

        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transportPair = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transportPair.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transportPair.second );

        if (transportFactoryPtr->isConnectionOriented())
        {

            RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

            serverTransportPtr->setMaxMessageLength(1000*10);
            clientTransportAutoPtr->setMaxMessageLength(1000*10);

            tstring s0 = RCF_T("something special");

            Echo echo;

            for (int k=0; k<5; ++k)
            {
                RCF::FilterServicePtr filterServicePtr(new RCF::FilterService);
                std::vector<CreateFilterInfo> factoriesOk;
                std::vector<CreateFilterInfo> factoriesNotOk;
                setupTest(
                    k,
                    filterServicePtr,
                    factoriesOk,
                    factoriesNotOk);

                RCF::RcfServer server( serverTransportPtr );
                server.bind( (I_Echo*) 0, echo);
                server.addService( RCF::ServicePtr(filterServicePtr) );
                server.start();

                RcfClient<I_Echo> client( clientTransportAutoPtr->clone() );
                
                // no encryption or authentication
                tstring s = client.echo(s0);
                RCF_CHECK(s0 == s);
                RCF_CHECK(icmp(client.getUserName(), usernameOrig));

                std::cout << "Expecting to fail:" << std::endl;

                // client authentications that are supposed to fail
                for (unsigned int j=0; j<factoriesNotOk.size(); ++j)
                {
                    CreateFilter createFilter = factoriesNotOk[j].mCreateFilter;
                    tstring username = factoriesNotOk[j].mUsername;
                    std::string desc = factoriesNotOk[j].mDesc;
                    std::cout << "Testing: " << desc << std::endl;

                    // no filter
                    RCF_CHECK(client.echo(s0) == s0);
                    RCF_CHECK(icmp(client.getUserName(), usernameOrig));

                    // sspi filter
                    try
                    {
                        client.getClientStub().requestTransportFilters(
                            createFilter(RCF::SspiFilter::None));

                        client.echo(s0);
                        RCF_CHECK(1==0);
                    }
                    catch(const RCF::Exception &e)
                    {
                        RCF_CHECK(1==1);
                    }

                    // no filter
                    client.getClientStub().clearTransportFilters();
                    RCF_CHECK(client.echo(s0) == s0);
                    RCF_CHECK(icmp(client.getUserName(), usernameOrig));

                    // sspi filter
                    try
                    {
                        client.getClientStub().requestTransportFilters(
                            createFilter(RCF::SspiFilter::None));

                        client.echo(s0);
                        RCF_CHECK(1==0);
                    }
                    catch(const RCF::Exception &e)
                    {
                        RCF_CHECK(1==1);
                        client.getClientStub().clearTransportFilters();
                    }
                }

                std::cout << "Expecting to pass:" << std::endl;

                // client authentications that are supposed to succeed
                for (std::size_t j=0; j<factoriesOk.size(); ++j)
                {
                    CreateFilter createFilter = factoriesOk[j].mCreateFilter;
                    tstring username = factoriesOk[j].mUsername;
                    std::string desc = factoriesOk[j].mDesc;
                    std::cout << "Testing: " << desc << std::endl;

                    // sspi filter - no encryption or integrity
                    client.getClientStub().requestTransportFilters(
                        createFilter(RCF::SspiFilter::None));

                    RCF_CHECK(client.echo(s0) == s0);
                    RCF_CHECK(icmp(client.getUserName(), username));

                    // sspi filter - encrypted
                    client.getClientStub().requestTransportFilters(
                        createFilter(RCF::SspiFilter::Encryption));

                    RCF_CHECK(client.echo(s0) == s0);
                    RCF_CHECK(icmp(client.getUserName(), username));

                    // test reconnection
                    server.close();
                    server.start();

                    // sspi filter - still encrypted
                    RCF_CHECK(client.echo(s0) == s0);
                    RCF_CHECK(icmp(client.getUserName(), username));

                    // sspi filter - integrity
                    client.getClientStub().requestTransportFilters(
                        createFilter(RCF::SspiFilter::Integrity));

                    RCF_CHECK(client.echo(s0) == s0);
                    RCF_CHECK(icmp(client.getUserName(), username));

                    // no filters
                    client.getClientStub().clearTransportFilters();
                    RCF_CHECK(client.echo(s0) == s0);
                    RCF_CHECK(icmp(client.getUserName(), usernameOrig));

                    // sspi filter - encrypted
                    client.getClientStub().requestTransportFilters(
                        createFilter(RCF::SspiFilter::Encryption));

                    RCF_CHECK(client.echo(s0) == s0);
                    RCF_CHECK(icmp(client.getUserName(), username));

                    {
                        // try parallel sspi filtered connections
                        typedef boost::shared_ptr<RcfClient<I_Echo> > ClientPtr;
                        std::vector<ClientPtr> clients;
                        for (int j=0; j<3; ++j)
                        {
                            clients.push_back( ClientPtr(
                                new RcfClient<I_Echo>(clientTransportAutoPtr->clone())));
                        }

                        for (int j=0; j<3; ++j)
                        {
                            RCF_CHECK( icmp(clients[i]->getUserName(), usernameOrig));
                            RCF_CHECK( clients[i]->echo(s0) == s0 );

                            clients[i]->getClientStub().requestTransportFilters(
                                createFilter(RCF::SspiFilter::Encryption));

                            RCF_CHECK( icmp(clients[i]->getUserName(), username));
                            RCF_CHECK( clients[i]->echo(s0) == s0 );

                            clients[i]->getClientStub().clearTransportFilters();

                            RCF_CHECK( icmp(clients[i]->getUserName(), usernameOrig));
                            RCF_CHECK( clients[i]->echo(s0) == s0 );
                        }
                    }

#ifdef RCF_USE_ZLIB

                    // test sspi together with zlib filter
                    // TODO: factor this out and use to test all filters

                    filterServicePtr->addFilterFactory(
                        RCF::FilterFactoryPtr(
                            new RCF::ZlibStatefulCompressionFilterFactory()));

                    filterServicePtr->addFilterFactory(
                        RCF::FilterFactoryPtr(
                            new RCF::ZlibStatelessCompressionFilterFactory()));

                    std::vector<RCF::FilterPtr> filters;

                    client.getClientStub().clearTransportFilters();
                    RCF_CHECK( client.getUserName() == usernameOrig );
                    RCF_CHECK( client.echo(s0) == s0 );

                    filters.clear();

                    filters.push_back(
                        RCF::FilterPtr(
                            new RCF::ZlibStatefulCompressionFilter()));

                    filters.push_back(
                        createFilter(RCF::SspiFilter::Encryption));

                    client.getClientStub().requestTransportFilters(filters);
                    RCF_CHECK( icmp(client.getUserName(), username));
                    RCF_CHECK( client.echo(s0) == s0 );

                    filters.clear();

                    filters.push_back(
                        createFilter(RCF::SspiFilter::Encryption));

                    filters.push_back(
                        RCF::FilterPtr(
                            new RCF::ZlibStatefulCompressionFilter()));
#ifndef __BORLANDC__
                    client.getClientStub().requestTransportFilters(filters);
                    RCF_CHECK( icmp(client.getUserName(), username));
                    RCF_CHECK( client.echo(s0) == s0 );
#endif

                    filters.clear();

                    filters.push_back(
                        RCF::FilterPtr(
                            new RCF::ZlibStatefulCompressionFilter()));

                    filters.push_back(
                        createFilter(RCF::SspiFilter::Encryption));

                    filters.push_back(
                        RCF::FilterPtr(
                            new RCF::ZlibStatefulCompressionFilter()));
// TODO: tests with zlib filter after sspi filter fail on borland
#ifndef __BORLANDC__
                    client.getClientStub().requestTransportFilters(filters);
                    RCF_CHECK( icmp(client.getUserName(), username));
                    RCF_CHECK( client.echo(s0) == s0 );
#endif
                    filters.clear();
                    filters.push_back( createFilter(RCF::SspiFilter::Encryption));
                    filters.push_back( createFilter(RCF::SspiFilter::Encryption));
                    filters.push_back( createFilter(RCF::SspiFilter::Encryption));
                    client.getClientStub().requestTransportFilters(filters);
                    RCF_CHECK( icmp(client.getUserName(), username));
                    RCF_CHECK( client.echo(s0) == s0 );

                    filters.clear();
                    filters.push_back( RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
                    filters.push_back( RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
                    filters.push_back( RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
                    filters.push_back( createFilter(RCF::SspiFilter::Encryption));
                    filters.push_back( createFilter(RCF::SspiFilter::Encryption));
                    filters.push_back( createFilter(RCF::SspiFilter::Encryption));
                    client.getClientStub().requestTransportFilters(filters);
                    RCF_CHECK( icmp(client.getUserName(), username));
                    RCF_CHECK( client.echo(s0) == s0 );
#ifndef __BORLANDC__
                    filters.clear();
                    filters.push_back( createFilter(RCF::SspiFilter::Encryption));
                    filters.push_back( createFilter(RCF::SspiFilter::Encryption));
                    filters.push_back( createFilter(RCF::SspiFilter::Encryption));
                    filters.push_back( RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
                    filters.push_back( RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
                    filters.push_back( RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
                    client.getClientStub().requestTransportFilters(filters);
                    RCF_CHECK( icmp(client.getUserName(), username));
                    RCF_CHECK( client.echo(s0) == s0 );

                    filters.clear();
                    filters.push_back( createFilter(RCF::SspiFilter::Encryption));
                    filters.push_back( RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
                    filters.push_back( createFilter(RCF::SspiFilter::Encryption));
                    filters.push_back( RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
                    filters.push_back( createFilter(RCF::SspiFilter::Encryption));
                    filters.push_back( RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
                    client.getClientStub().requestTransportFilters(filters);
                    RCF_CHECK( icmp(client.getUserName(), username));
                    RCF_CHECK( client.echo(s0) == s0 );
#endif
                    filters.clear();
                    client.getClientStub().requestTransportFilters(filters);
                    RCF_CHECK( icmp(client.getUserName(), usernameOrig));
                    RCF_CHECK( client.echo(s0) == s0 );

                    filters.clear();
                    filters.push_back( RCF::FilterPtr( new RCF::ZlibStatefulCompressionFilter()));
                    client.getClientStub().requestTransportFilters(filters);
                    RCF_CHECK( icmp(client.getUserName(), usernameOrig));
                    RCF_CHECK( client.echo(s0) == s0 );

                    // TODO: test with stateless compression filter as well
                    // ...

#endif // RCF_USE_ZLIB

                }
            }
        }
    }
    return boost::exit_success;
}







