
#include <string>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/CommandLine.hpp>

namespace Test_InitDeinit {

    class Echo
    {
    public:
        std::string echo(const std::string &s)
        {
            sLog = s;
            return s;
        }
        static std::string sLog;
    };

    std::string Echo::sLog;

    RCF_BEGIN(I_Echo, "I_Echo")
        RCF_METHOD_R1(std::string, echo, const std::string &)
    RCF_END(I_Echo)

} // namespace Test_InitDeinit

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_InitDeinit;

    util::CommandLine::getSingleton().parse(argc, argv);

    RCF::deinit();

    for (unsigned int i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::init();

        {
            RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
            RCF::TransportPair transports = transportFactoryPtr->createTransports();
            RCF::ServerTransportPtr serverTransportPtr( transports.first );
            RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );
       
            RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

            std::string s0 = "something special";

            Echo echo;
            RCF::RcfServer server( boost::dynamic_pointer_cast<RCF::I_Service>(serverTransportPtr) );
            server.start();
            server.bind( (I_Echo*) 0, echo);

            std::string s = RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).echo(s0);
            RCF_CHECK(s0 == s);
        }

        RCF::deinit();

    }
   
    RCF::init();
    RCF::init();

    RCF::deinit();
    RCF::deinit();

    RCF::init();

    return boost::exit_success;
}
