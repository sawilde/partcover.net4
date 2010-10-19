
#include <memory>
#include <string>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/CommandLine.hpp>

// for broken compilers
namespace Test_Binding {
    class I_E;
}
RCF_NON_RCF_PARENT_INTERFACE(Test_Binding::I_E)

namespace Test_Binding {

    class MyClass
    {
    public:
        MyClass() : mVal()
        {}

        MyClass(int val) : mVal(val)
        {}

        void serialize(SF::Archive &ar)
        {
            ar & mVal;
        }

        template<typename Archive>
        void serialize(Archive &ar, const unsigned int)
        {
            ar & mVal;
        }

        int mVal;

    private:
        MyClass(const MyClass &);
        MyClass operator=(MyClass &);
    };

}

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Binding::MyClass)

namespace Test_Binding {

    class Echo
    {
    public:
        std::string echo(const std::string &s)
        {
            return s;
        }

        void increment(MyClass &myClass1, const MyClass &myClass2)
        {
            ++myClass1.mVal;
        }
    };

    RCF_BEGIN(I_Echo, "I_Echo")
        RCF_METHOD_R1(std::string, echo, const std::string &)
        RCF_METHOD_V2(void, increment, MyClass &, const MyClass &)
    RCF_END(I_Echo)

    // from Test_Inheritance

    // interface inheritance hierarchies
    //
    //A1 <- B1 <- C1 <-
    //                         \
    //                          D
    //                         /
    //A2 <- B2 <- C2 <-
    //
    //A1 <- X <-
    //              \
    //               Z
    //              /
    //A1 <- Y <-
    //
    // I_E <- E

    RCF_BEGIN(A1, "A1")
        RCF_METHOD_R0(std::string, funcA1)
    RCF_END(A1)

    namespace N1 {

        RCF_BEGIN_I1(B1, "B1", A1)
            RCF_METHOD_R0(std::string, funcB1)
        RCF_END(B1)

        namespace N2 {

            RCF_BEGIN_I1(C1, "C1", B1)
                RCF_METHOD_R0(std::string, funcC1)
            RCF_END(C1)

        } // namespace N2

    } // namespace N1

    namespace N3 {

        RCF_BEGIN(A2,"A2")
            RCF_METHOD_R0(std::string, funcA2)
        RCF_END(A2)

        RCF_BEGIN_I1(B2,"B2", A2)
            RCF_METHOD_R0(std::string, funcB2)
        RCF_END(B2)

        RCF_BEGIN_I1(C2,"C2", B2)
            RCF_METHOD_R0(std::string, funcC2)
        RCF_END(C2)

    } // namespace N3

    RCF_BEGIN_I2(D, "D", N1::N2::C1, N3::C2)
        RCF_METHOD_R0(std::string, funcD)
    RCF_END(D)

    RCF_BEGIN_I1(X, "X", A1)
        RCF_METHOD_R0(std::string, funcX)
    RCF_END(X)

    RCF_BEGIN_I1(Y, "Y", A1)
        RCF_METHOD_R0(std::string, funcY)
    RCF_END(Y)

    RCF_BEGIN_I2(Z, "Z", X, Y)
        RCF_METHOD_R0(std::string, funcZ)
    RCF_END(Z)

    class I_E
    {
    public:
        virtual ~I_E() {}
        virtual RCF::FutureImpl<std::string> funcE() = 0;
    };

    RCF_BEGIN_I1(E, "E", I_E)
        RCF_METHOD_R0(std::string, funcE)
    RCF_END(E)

    RCF_BEGIN(F1, "F1")
        RCF_METHOD_R0(std::string, funcF1)
    RCF_END(F1)

    RCF_BEGIN(F2, "F2")
        RCF_METHOD_R0(std::string, funcF2)
    RCF_END(F2)

    RCF_BEGIN(F3, "F3")
        RCF_METHOD_R0(std::string, funcF3)
    RCF_END(F3)

    RCF_BEGIN(F4, "F4")
        RCF_METHOD_R0(std::string, funcF4)
    RCF_END(F4)

    class ServerImpl
    {
    public:
        std::string funcA1() { return "A1"; }
        std::string funcB1() { return "B1"; }
        std::string funcC1() { return "C1"; }

        std::string funcA2() { return "A2"; }
        std::string funcB2() { return "B2"; }
        std::string funcC2() { return "C2"; }

        std::string funcD() { return "D"; }

        std::string funcX() { return "X"; }
        std::string funcY() { return "Y"; }
        std::string funcZ() { return "Z"; }

        std::string funcE() { return "E"; }

        std::string funcF1() { return "F1"; }
        std::string funcF2() { return "F2"; }
        std::string funcF3() { return "F3"; }
        std::string funcF4() { return "F4"; }
    };

    RCF_BEGIN(I_M, "I_M")
    RCF_END(I_M)

    RCF_BEGIN_I1(I_N, "I_N", I_M)
    RCF_END(I_Echo)

    class ClientTask
    {
    public:
        ClientTask(const RCF::I_ClientTransport &clientTransport) :
            mClientTransport(clientTransport)
        {}

        void operator()()
        {

            typedef A1::RcfClientT RcfClientA1;
            typedef N1::B1::RcfClientT RcfClientB1;
            typedef N1::N2::C1::RcfClientT RcfClientC1;

            typedef N3::A2::RcfClientT RcfClientA2;
            typedef N3::B2::RcfClientT RcfClientB2;
            typedef N3::C2::RcfClientT RcfClientC2;

            typedef D::RcfClientT RcfClientD;

            typedef X::RcfClientT RcfClientX;
            typedef Y::RcfClientT RcfClientY;
            typedef Z::RcfClientT RcfClientZ;

            typedef RcfClient<E> RcfClientE;

            // quick check for value semantics
            std::vector< RcfClientA1 >      vec1(20);
            std::vector< RcfClientB1 >      vec2(20);
            std::vector< RcfClientC1 >      vec3(20);
            std::vector< RcfClient<I_M> >   vec4(20);
            std::vector< RcfClient<I_N> >   vec5(20);
            RCF_UNUSED_VARIABLE(vec1);
            RCF_UNUSED_VARIABLE(vec2);
            RCF_UNUSED_VARIABLE(vec3);
            RCF_UNUSED_VARIABLE(vec4);
            RCF_UNUSED_VARIABLE(vec5);

            std::string s;

            RcfClientC1 clientC1( mClientTransport.clone());
            RCF_CHECK( &clientC1.getClientStub() == &clientC1.RcfClientB1::getClientStub());
            RCF_CHECK( &clientC1.RcfClientB1::getClientStub() == &clientC1.RcfClientC1::getClientStub());

            s = RcfClientA1(mClientTransport.clone()).funcA1();     RCF_CHECK(s == "A1");
            s = RcfClientB1(mClientTransport.clone()).funcA1();     RCF_CHECK(s == "A1");
            s = RcfClientB1(mClientTransport.clone()).funcB1();     RCF_CHECK(s == "B1");

            s = RcfClientC1(mClientTransport.clone()).funcA1();     RCF_CHECK(s == "A1");
            s = RcfClientC1(mClientTransport.clone()).funcB1();     RCF_CHECK(s == "B1");
            s = RcfClientC1(mClientTransport.clone()).funcC1();     RCF_CHECK(s == "C1");

            s = RcfClientA2(mClientTransport.clone()).funcA2();     RCF_CHECK(s == "A2");
            s = RcfClientB2(mClientTransport.clone()).funcA2();     RCF_CHECK(s == "A2");
            s = RcfClientB2(mClientTransport.clone()).funcB2();     RCF_CHECK(s == "B2");
            s = RcfClientC2(mClientTransport.clone()).funcA2();     RCF_CHECK(s == "A2");
            s = RcfClientC2(mClientTransport.clone()).funcB2();     RCF_CHECK(s == "B2");
            s = RcfClientC2(mClientTransport.clone()).funcC2();     RCF_CHECK(s == "C2");

            s = RcfClientD(mClientTransport.clone()).funcA1();      RCF_CHECK(s == "A1");
            s = RcfClientD(mClientTransport.clone()).funcA2();      RCF_CHECK(s == "A2");
            s = RcfClientD(mClientTransport.clone()).funcB1();      RCF_CHECK(s == "B1");
            s = RcfClientD(mClientTransport.clone()).funcB2();      RCF_CHECK(s == "B2");
            s = RcfClientD(mClientTransport.clone()).funcC1();      RCF_CHECK(s == "C1");
            s = RcfClientD(mClientTransport.clone()).funcC2();      RCF_CHECK(s == "C2");
            s = RcfClientD(mClientTransport.clone()).funcD();       RCF_CHECK(s == "D");

            s = RcfClientX(mClientTransport.clone()).funcA1();      RCF_CHECK(s == "A1");
            s = RcfClientX(mClientTransport.clone()).funcX();       RCF_CHECK(s == "X");

            s = RcfClientY(mClientTransport.clone()).funcA1();      RCF_CHECK(s == "A1");
            s = RcfClientY(mClientTransport.clone()).funcY();       RCF_CHECK(s == "Y");

            s = RcfClientZ(mClientTransport.clone()).RcfClientX::funcA1();  RCF_CHECK(s == "A1");
            s = RcfClientZ(mClientTransport.clone()).funcX();               RCF_CHECK(s == "X");
            s = RcfClientZ(mClientTransport.clone()).RcfClientY::funcA1();  RCF_CHECK(s == "A1");
            s = RcfClientZ(mClientTransport.clone()).funcY();               RCF_CHECK(s == "Y");
            s = RcfClientZ(mClientTransport.clone()).funcZ();               RCF_CHECK(s == "Z");

            RcfClient<E>(mClientTransport.clone()).funcE();
            std::auto_ptr<I_E> e( new RcfClient<E>(mClientTransport.clone()) );
            s = e->funcE(); RCF_CHECK(s == "E");

            s = RcfClient<F1>(mClientTransport.clone(), "F").funcF1();      RCF_CHECK(s == "F1");
            s = RcfClient<F2>(mClientTransport.clone(), "F").funcF2();      RCF_CHECK(s == "F2");
            s = RcfClient<F3>(mClientTransport.clone(), "F").funcF3();      RCF_CHECK(s == "F3");
            s = RcfClient<F4>(mClientTransport.clone(), "F").funcF4();      RCF_CHECK(s == "F4");
        }

    private:
        const RCF::I_ClientTransport &mClientTransport;
    };

} // namespace Test_Binding

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_Binding;

    util::CommandLine::getSingleton().parse(argc, argv);

    for (int i=0; i<RCF::getTransportFactories().size(); ++i)
    {
        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        std::string s0 = "whatever";

        {
            Echo e;
            RCF::RcfServer server(serverTransportPtr);
            server.bind( (I_Echo*) 0, e);
            server.start();

            RcfClient<I_Echo> echo(clientTransportAutoPtr->clone());
            std::string s = echo.echo(s0);
            RCF_CHECK(s == s0);

            {
                // make sure that RCF is not making any unnecessary parameter copies
                // MyClass has its copy ctor and assignment op disabled
                int val = 17;
                MyClass myClass1(val);
                MyClass myClass2(val);
                RcfClient<I_Echo>(clientTransportAutoPtr->clone()).increment(myClass1, myClass2);
                RCF_CHECK(myClass1.mVal == val+1);
                RCF_CHECK(myClass2.mVal == val);
            }

            server.unbind( (I_Echo *) 0);
            try
            {
                RcfClient<I_Echo>(clientTransportAutoPtr->clone()).echo(s0);
                RCF_CHECK(1==0);
            }
            catch(const RCF::Exception &e)
            {
                RCF_CHECK(1==1);
            }

        }

        transports = transportFactoryPtr->createTransports();
        serverTransportPtr = transports.first;
        clientTransportAutoPtr = *transports.second;

        {
            std::auto_ptr<Echo> ape( new Echo );
            RCF::RcfServer server(serverTransportPtr);
            server.bind( (I_Echo *) 0, ape);
            server.start();

            RcfClient<I_Echo> echo(clientTransportAutoPtr->clone());
            std::string s = echo.echo(s0);
            RCF_CHECK(s == s0);

            server.unbind( (I_Echo *) 0);
            try
            {
                RcfClient<I_Echo>(clientTransportAutoPtr->clone()).echo(s0);
                RCF_CHECK(1==0);
            }
            catch(const RCF::Exception &e)
            {
                RCF_CHECK(1==1);
            }
        }

        transports = transportFactoryPtr->createTransports();
        serverTransportPtr = transports.first;
        clientTransportAutoPtr = *transports.second;

        {
            boost::shared_ptr<Echo> spe( new Echo );
            RCF::RcfServer server(serverTransportPtr);
            //server.bind<I_Echo>(spe);
            server.bind( (I_Echo *) 0, spe);
            server.start();

            RcfClient<I_Echo> echo(clientTransportAutoPtr->clone());
            std::string s = echo.echo(s0);
            RCF_CHECK(s == s0);

            server.unbind( (I_Echo *) 0);
            try
            {
                RcfClient<I_Echo>(clientTransportAutoPtr->clone()).echo(s0);
                RCF_CHECK(1==0);
            }
            catch(const RCF::Exception &e)
            {
                RCF_CHECK(1==1);
            }
        }

        transports = transportFactoryPtr->createTransports();
        serverTransportPtr = transports.first;
        clientTransportAutoPtr = *transports.second;

        {
            boost::shared_ptr<Echo> spe( new Echo );
            boost::weak_ptr<Echo> wpe(spe);
            RCF::RcfServer server(serverTransportPtr);
            server.bind( (I_Echo *) 0, wpe);
            server.start();

            RcfClient<I_Echo> echo(clientTransportAutoPtr->clone());
            std::string s = echo.echo(s0);
            RCF_CHECK(s == s0);

            server.unbind( (I_Echo *) 0);
            try
            {
                RcfClient<I_Echo>(clientTransportAutoPtr->clone()).echo(s0);
                RCF_CHECK(1==0);
            }
            catch(const RCF::Exception &e)
            {
                RCF_CHECK(1==1);
            }
        }

        transports = transportFactoryPtr->createTransports();
        serverTransportPtr = transports.first;
        clientTransportAutoPtr = *transports.second;

        {
            ServerImpl serverImpl;
            RCF::RcfServer server(serverTransportPtr);

            server.bind( (A1*) 0, serverImpl);
            server.bind( (N1::B1*) 0, serverImpl);
            server.bind( (N1::N2::C1*) 0, serverImpl);

            server.bind( (N3::A2*) 0, serverImpl);
            server.bind( (N3::B2*) 0, serverImpl);
            server.bind( (N3::C2*) 0, serverImpl);

            server.bind( (D*) 0, serverImpl);

            server.bind( (X*) 0, serverImpl);
            server.bind( (Y*) 0, serverImpl);
            server.bind( (Z*) 0, serverImpl);

            server.bind( (E*) 0, serverImpl);

            boost::shared_ptr<ServerImpl> serverImplPtr;
            server.bind( (F1*) 0, serverImplPtr, "F");
            server.bind( (F1*) 0, (F2*) 0, serverImplPtr, "F");
            server.bind( (F1*) 0, (F2*) 0, (F3*) 0, serverImplPtr, "F");
            server.bind( (F1*) 0, (F2*) 0, (F3*) 0, (F4*) 0, serverImplPtr, "F");

            boost::weak_ptr<ServerImpl> serverImplWeakPtr;
            server.bind( (F1*) 0, serverImplWeakPtr, "F");
            server.bind( (F1*) 0, (F2*) 0, serverImplWeakPtr, "F");
            server.bind( (F1*) 0, (F2*) 0, (F3*) 0, serverImplWeakPtr, "F");
            server.bind( (F1*) 0, (F2*) 0, (F3*) 0, (F4*) 0, serverImplWeakPtr, "F");

            std::auto_ptr<ServerImpl> serverImplAutoPtr;
            server.bind( (F1*) 0, serverImplAutoPtr, "F");
            server.bind( (F1*) 0, (F2*) 0, serverImplAutoPtr, "F");
            server.bind( (F1*) 0, (F2*) 0, (F3*) 0, serverImplAutoPtr, "F");
            server.bind( (F1*) 0, (F2*) 0, (F3*) 0, (F4*) 0, serverImplAutoPtr, "F");

            server.bind( (F1*) 0, serverImpl, "F");
            server.bind( (F1*) 0, (F2*) 0, serverImpl, "F");
            server.bind( (F1*) 0, (F2*) 0, (F3*) 0, serverImpl, "F");
            server.bind( (F1*) 0, (F2*) 0, (F3*) 0, (F4*) 0, serverImpl, "F");

            server.start();

            ClientTask task(*clientTransportAutoPtr);
            RCF::Thread thread(task);
            thread.join();
        }

        {
            ServerImpl serverImpl;
            RCF::RcfServer server(serverTransportPtr);
            server.bind( (N1::N2::C1*) 0, serverImpl);
            server.start();

            typedef N1::N2::C1::RcfClientT   RcfClientC1;
            typedef N1::B1::RcfClientT       RcfClientB1;
            typedef A1::RcfClientT           RcfClientA1;

            std::string s;

            RcfClientC1 clientC1( clientTransportAutoPtr->clone());
            s = clientC1.funcC1(); RCF_CHECK( s == "C1");
            s = clientC1.funcB1(); RCF_CHECK( s == "B1");
            s = clientC1.funcA1(); RCF_CHECK( s == "A1");

            {
                RcfClientB1 clientB1( clientTransportAutoPtr->clone());
                try
                {
                    s = clientB1.funcB1();
                    RCF_CHECK(1 == 0);
                }
                catch(const RCF::Exception & e)
                {
                    RCF_CHECK(1 == 1);
                }
            }
            {
                RcfClientB1 clientB1(
                    clientTransportAutoPtr->clone(),
                    RCF::getInterfaceName<N1::N2::C1>());

                s = clientB1.funcB1(); RCF_CHECK( s == "B1");
            }
        }

    }

    return boost::exit_success;
}

