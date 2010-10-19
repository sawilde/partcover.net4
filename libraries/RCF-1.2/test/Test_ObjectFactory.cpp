
#include <string>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/ObjectFactoryService.hpp>
#include <RCF/SessionObjectFactoryService.hpp>
#include <RCF/TcpClientTransport.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/Version.hpp>
#include <RCF/test/PrintTestHeader.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Platform/OS/Sleep.hpp>

namespace Test_ObjectFactoryService {

    class Echo
    {
    public:
        Echo() : 
            mNVal(RCF_DEFAULT_INIT), 
            mBVal(RCF_DEFAULT_INIT), 
            mDVal(RCF_DEFAULT_INIT), 
            mSVal()
        {}

        // I_Echo
        std::string echo(const std::string &s)
        {
            sLog = s;
            return s;
        }
        static std::string sLog;

        void setVal(int val)
        {
            mNVal = val;
        }

        int getVal()
        {
            return mNVal;
        }

        // I_X1
        void setVal(bool val)
        {
            mBVal = val;
        }

        bool getVal(bool)
        {
            return mBVal;
        }

        // I_X2
        void setVal(double val)
        {
            mDVal = val;
        }

        double getVal(double)
        {
            return mDVal;
        }

        // I_X3
        void setVal(std::string val)
        {
            mSVal = val;
        }

        std::string getVal(std::string)
        {
            return mSVal;
        }

    private:
        int mNVal;
        bool mBVal;
        double mDVal;
        std::string mSVal;
    };

    std::string Echo::sLog;

    RCF_BEGIN(I_Echo, "I_Echo")
        RCF_METHOD_R1(std::string, echo, const std::string &)
        RCF_METHOD_V1(void, setVal, int)
        RCF_METHOD_R0(int, getVal)
    RCF_END(I_Echo)

    RCF_BEGIN(I_X1, "I_X1")
        RCF_METHOD_V1(void, setVal, bool)
        RCF_METHOD_R1(bool, getVal, bool)
    RCF_END(I_X1)

    RCF_BEGIN(I_X2, "I_X2")
        RCF_METHOD_V1(void, setVal, double)
        RCF_METHOD_R1(double, getVal, double)
    RCF_END(I_X2)

    RCF_BEGIN(I_X3, "I_X3")
        RCF_METHOD_V1(void, setVal, std::string)
        RCF_METHOD_R1(std::string, getVal, std::string)
    RCF_END(I_X3)

    void exhaustTokenSpace(int numberOfTokens, RCF::I_RcfClient &client)
    {
        int count = 0;
        for (int j=0; j<numberOfTokens+1;++j)
        {
            if (tryCreateRemoteObject<I_Echo>(client,"Echo"))
            {
                ++count;
            }
            std::cout << client.getClientStub().getTargetToken() << std::endl;
        }
        RCF_CHECK(count == numberOfTokens);
    }

} // namespace Test_ObjectFactoryService

int test_main(int argc, char **argv)
{
    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_ObjectFactoryService;

    util::CommandLineOption<int> clNumberOfTokens("tokens", 5, "number of tokens in object factory");
    util::CommandLineOption<int> clClientStubTimeoutS("timeout", 2, "server object timeout in seconds");
    util::CommandLineOption<int> clWaitIntervalS("wait", 5, "how long to wait for object factory to run cleanup");
    util::CommandLineOption<int> clCleanupIntervalS("cleanup", 2, "object factory cleanup interval in seconds");
    util::CommandLine::getSingleton().parse(argc, argv);

    std::string ip = "127.0.0.1";
    unsigned int numberOfTokens = clNumberOfTokens;
    unsigned int clientStubTimeoutS = clClientStubTimeoutS;
    unsigned int waitIntervalS = clWaitIntervalS;
    unsigned int cleanupIntervalS = clCleanupIntervalS;

    std::string s0 = "something special";

    {
        RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
        RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

        RCF::RcfServer server( serverTransportPtr );

        RCF::ObjectFactoryServicePtr objectFactoryServicePtr(
            new RCF::ObjectFactoryService(
                numberOfTokens, clientStubTimeoutS, cleanupIntervalS) );

        objectFactoryServicePtr->bind( (I_Echo*) 0,  (Echo**) 0, "Echo");

        server.addService( objectFactoryServicePtr );

        server.start();

        RcfClient<I_Echo> client( clientTransportAutoPtr );

        bool ok = tryCreateRemoteObject<I_Echo>(client, "Echo");
        RCF_CHECK(ok);
        std::string s = client.echo(s0);
        RCF::Token token = client.getClientStub().getTargetToken();
        RCF_CHECK( s == s0 );
        RCF_CHECK(token != RCF::Token());

        // check that we're ok w.r.t. server restarting
        s = client.echo(s0);
        server.stop();
        server.start();
        s = client.echo(s0);

        // check that the object isn't deleted before the timeout
        client.getClientStub().disconnect();
        s == client.echo(s0);

        // check that the object isn't deleted while the connection is alive
        Platform::OS::SleepMs(waitIntervalS*1000);
        s = client.echo(s0);

        for (int j=0; j<numberOfTokens-1; ++j)
        {
            ok = tryCreateRemoteObject<I_Echo>(client, "Echo");
            RCF_CHECK(ok);
        }

        ok = tryCreateRemoteObject<I_Echo>(client, "Echo");
        RCF_CHECK(!ok);

        std::cout << "Waiting for " << waitIntervalS << " seconds...\n";
        Platform::OS::Sleep(waitIntervalS);
        exhaustTokenSpace(numberOfTokens, client);

        server.removeService(objectFactoryServicePtr);
        server.addService(objectFactoryServicePtr);

        std::cout << "Waiting for " << waitIntervalS << " seconds...\n";
        Platform::OS::Sleep(waitIntervalS);
        exhaustTokenSpace(numberOfTokens, client);

        server.removeService(objectFactoryServicePtr);
        objectFactoryServicePtr.reset( new RCF::ObjectFactoryService(
            numberOfTokens, clientStubTimeoutS, cleanupIntervalS) );
        objectFactoryServicePtr->bind( (I_Echo*) 0,  (Echo**) 0, "Echo");
        server.addService(objectFactoryServicePtr);

        std::cout << "Waiting for " << waitIntervalS << " seconds...\n";
        Platform::OS::Sleep(waitIntervalS);
        exhaustTokenSpace(numberOfTokens, client);
    }

    {
        // try moving an object factory service from one server to another

        RCF::ObjectFactoryServicePtr objectFactoryServicePtr(
            new RCF::ObjectFactoryService(
                numberOfTokens, clientStubTimeoutS, cleanupIntervalS));
        objectFactoryServicePtr->bind( (I_Echo*) 0,  (Echo**) 0, "Echo");

        RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
        RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

        int val = 17;

        RCF::Token token;

        {
            RCF::RcfServer server(serverTransportPtr);
            server.start();
            server.addService(objectFactoryServicePtr);

            RcfClient<I_Echo> client(clientTransportAutoPtr);
            bool ok = tryCreateRemoteObject<I_Echo>(client, "Echo");
            RCF_CHECK(ok);
            token = client.getClientStub().getTargetToken();
            client.setVal(val);
            exhaustTokenSpace(numberOfTokens-1, client);
        }
        {
            RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
            RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
            RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

            RCF::RcfServer server(serverTransportPtr);
            server.start();
            server.addService(objectFactoryServicePtr);

            {
                RcfClient<I_Echo> client(clientTransportAutoPtr->clone());
                client.getClientStub().setTargetToken(token);
                int myVal = client.getVal();
                RCF_CHECK(val == myVal);
            }

            Platform::OS::Sleep(waitIntervalS);
            try
            {
                // the server object should have been cleaned up by now, so the next call
                // should result in an exception
                RcfClient<I_Echo> client(clientTransportAutoPtr->clone());
                client.getClientStub().setTargetToken(token);
                int myVal = client.getVal();
                RCF_CHECK(1==0);
            }
            catch(const RCF::Exception &e)
            {
                RCF_CHECK(1==1);
                RCF_CHECK( e.getErrorId() == RCF::RcfError_NoServerStub);
            }

        }

    }

    // faceted objects

    // TODO: testing of
    // 1) heavy concurrent access to different interfaces
    // 2) user friendly reporting of interface-not-found errors

    {
        RCF::ObjectFactoryServicePtr objectFactoryServicePtr(
            new RCF::ObjectFactoryService(
            50, 60, cleanupIntervalS));

        objectFactoryServicePtr->bind( (I_Echo*) 0,  (Echo**) 0, "Echo");
        objectFactoryServicePtr->bind( (I_Echo*) 0,  (I_X1*) 0,  (Echo**) 0, "Echo");
        objectFactoryServicePtr->bind( (I_Echo*) 0,  (I_X1*) 0,  (I_X2*) 0,  (Echo**) 0, "Echo");
        objectFactoryServicePtr->bind( (I_Echo*) 0,  (I_X1*) 0,  (I_X2*) 0,  (I_X3*) 0,  (Echo**) 0, "Echo");

        RCF::TransportPair transportPair = RCF::TcpTransportFactory().createTransports();
        RCF::ServerTransportPtr serverTransportPtr = transportPair.first;
        RCF::ClientTransportAutoPtr clientTransportAutoPtr = *transportPair.second;

        int nVal = 17;
        bool bVal = true;
        double dVal = 3.14;
        std::string sVal = "seventeen";

        RCF::Token token;

        RCF::RcfServer server(serverTransportPtr);

        server.start();
        server.addService(objectFactoryServicePtr);

        {
            RcfClient<I_Echo> client(clientTransportAutoPtr->clone());
            bool ok = tryCreateRemoteObject<I_Echo>(client, "Echo");
            RCF_CHECK(ok);
            token = client.getClientStub().getTargetToken();
        }

        {
            RcfClient<I_Echo> client(clientTransportAutoPtr->clone());

            client.getClientStub().setTargetToken(token);
            client.setVal(nVal);
            RCF_CHECK(client.getVal() == nVal);
        }

        {
            RcfClient<I_X1> client(clientTransportAutoPtr->clone());

            client.getClientStub().setTargetToken(token);
            client.setVal(bVal);
            RCF_CHECK(client.getVal(bool()) == bVal);
        }

        {
            RcfClient<I_X2> client(clientTransportAutoPtr->clone());

            client.getClientStub().setTargetToken(token);
            client.setVal(dVal);
            RCF_CHECK(client.getVal(double()) == dVal);
        }

        {
            RcfClient<I_X3> client(clientTransportAutoPtr->clone());

            client.getClientStub().setTargetToken(token);
            client.setVal(sVal);
            RCF_CHECK(client.getVal(std::string()) == sVal);
        }

        {
            // test deletion of remote objects
            RcfClient<I_X3> client(clientTransportAutoPtr->clone());

            client.getClientStub().setTargetToken(token);
            client.setVal(sVal);
            RCF_CHECK(client.getVal(std::string()) == sVal);

            client.getClientStub().deleteRemoteObject();

            try
            {
                client.getVal(std::string());
                RCF_CHECK(1==0);
            }
            catch(const RCF::Exception & e)
            {                
                RCF_CHECK(1==1);
                RCF_CHECK( e.getErrorId() == RCF::RcfError_NoServerStub);
            }
        }

        {
            // try adding some objects of our own to the ofs

            RcfClient<I_Echo> client(clientTransportAutoPtr->clone());

            boost::shared_ptr<Echo> echoPtr(new Echo());
            RCF::Token token1 = objectFactoryServicePtr->addObject( (I_Echo *) 0, echoPtr);
            client.getClientStub().setTargetToken(token1);
            RCF_CHECK(client.echo(s0) == s0);

            boost::weak_ptr<Echo> echoWeakPtr(echoPtr);
            RCF::Token token2 = objectFactoryServicePtr->addObject( (I_Echo *) 0, echoWeakPtr);
            client.getClientStub().setTargetToken(token2);
            RCF_CHECK(client.echo(s0) == s0);

            std::auto_ptr<Echo> echoAutoPtr(new Echo());
            RCF::Token token3 = objectFactoryServicePtr->addObject( (I_Echo *) 0, echoAutoPtr);
            client.getClientStub().setTargetToken(token3);
            RCF_CHECK(client.echo(s0) == s0);

            Echo echo;
            RCF::Token token4 = objectFactoryServicePtr->addObject( (I_Echo *) 0, echo);
            client.getClientStub().setTargetToken(token4);
            RCF_CHECK(client.echo(s0) == s0);

        }

        {
            // test creation and deletion of session objects

            RCF::ObjectFactoryServicePtr ofsPtr = objectFactoryServicePtr;
            server.removeService(ofsPtr);
            
            RCF::SessionObjectFactoryServicePtr sofsPtr( 
                new RCF::SessionObjectFactoryService());

            sofsPtr->bind( (I_X3 *) 0, (Echo **) 0);

            server.addService(sofsPtr);

            RcfClient<I_X3> client(clientTransportAutoPtr->clone());

            client.getClientStub().createRemoteSessionObject();
            client.setVal(sVal);
            RCF_CHECK(client.getVal(std::string()) == sVal);
            client.getClientStub().deleteRemoteSessionObject();

            // check explicit deletion
            try
            {
                client.getVal(std::string());
                RCF_CHECK(1==0);
            }
            catch(const RCF::Exception &)
            {
                RCF_CHECK(1==1);
            }

            client.getClientStub().createRemoteSessionObject();
            client.setVal(sVal);
            RCF_CHECK(client.getVal(std::string()) == sVal);
            client.getClientStub().disconnect();

            // check deletion on disconnect
            try
            {
                client.getVal(std::string());
                RCF_CHECK(1==0);
            }
            catch(const RCF::Exception &)
            {
                RCF_CHECK(1==1);
            }

            // check backwards compatibility to older RCF
            client.getClientStub().createRemoteSessionObject();
            client.getClientStub().setRcfRuntimeVersion(2);
            try
            {
                client.getClientStub().createRemoteSessionObject();
                RCF_CHECK(1==0);
            }
            catch(const RCF::Exception &)
            {
                RCF_CHECK(1==1);
            }

            // create an ofs and then try again to create a session object
            server.removeService(sofsPtr);
            ofsPtr.reset(new RCF::ObjectFactoryService(0,0));
            ofsPtr->bind( (I_X3 *) 0, (Echo **) 0);
            server.addService(ofsPtr);

            client.getClientStub().createRemoteSessionObject();
            
            // set version back to current
            client.getClientStub().setRcfRuntimeVersion(RCF::getRuntimeVersion());

            // should fail because there is no sofs
            try
            {
                client.getClientStub().createRemoteSessionObject();
                RCF_CHECK(1==0);
            }
            catch(const RCF::Exception & e)
            {
                RCF_CHECK(1==1);
                RCF_CHECK( e.getErrorId() == RCF::RcfError_NoServerStub);
            }
            
            server.removeService(ofsPtr);
            server.addService(sofsPtr);

            client.getClientStub().createRemoteSessionObject();
        }
    }

    return boost::exit_success;
}
