
#include <iostream>
#include <sstream>
#include <string>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/CurrentSession.hpp>
#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/RcfSession.hpp>

#include <RCF/test/TransportFactories.hpp>

#include <RCF/util/CommandLine.hpp>

#ifdef RCF_USE_BOOST_SERIALIZATION
// boost serialization requires following headers to appear down here, after inclusion of all archive headers...
#include <boost/serialization/export.hpp>
#include <boost/serialization/void_cast.hpp>
#endif

// We get enormous errors from boost.serialization (gcc 3.4, but not gcc 3.2) if this file is included farther up...
#include <SF/SerializeParent.hpp>

namespace Test_Polymorphic {

    class A
    {
    public:
        A() : a(RCF_DEFAULT_INIT) {}
        A(int a) : a(a) {}
        virtual ~A() {}
        int a;

        template<typename Archive>
        void serialize(Archive &ar, unsigned int)
        {
            ar & a;
        }

        void serialize(SF::Archive &ar)
        {
            ar & a;
        }
    };

    class B : public A
    {
    public:
        B() : A(), b(RCF_DEFAULT_INIT) {}
        B(int a, int b) : A(a),  b(b) {}
        int b;

        template<typename Archive>
        void serialize(Archive &ar, unsigned int)
        {
            serializeParent( (A*) 0, ar, *this);
            ar & b;
        }

#ifdef RCF_USE_SF_SERIALIZATION

        void serialize(SF::Archive &ar)
        {
            serializeParent( (A*) 0, ar, *this);
            ar & b;
        }

#endif

    };

    class C : public A
    {
    public:
        C() : A(), b(RCF_DEFAULT_INIT), c(RCF_DEFAULT_INIT) {}
        C(int a, int b, int c) : A(a),  b(b), c(c) {}
        int b;
        int c;

        template<typename Archive>
        void serialize(Archive &ar, unsigned int)
        {
            serializeParent( (A*) 0, ar, *this);
            ar & b;
            ar & c;
        }

        void serialize(SF::Archive &ar)
        {
            serializeParent( (A*) 0, ar, *this);
            ar & b;
            ar & c;
        }
    };

    // SF serialization
    #ifdef RCF_USE_SF_SERIALIZATION

    // setup the runtime to serialize polymorphic data
    AUTO_RUN( SF::registerType( (B*) 0, "Test_Polymorphic::B") );
    AUTO_RUN( SF::registerType( (C*) 0, "Test_Polymorphic::C") );

    AUTO_RUN(( SF::registerBaseAndDerived( (A *) 0, (B *) 0) ));
    AUTO_RUN(( SF::registerBaseAndDerived( (A *) 0, (C *) 0) ));

    #endif


    // boost serialization
    //#ifdef RCF_USE_BOOST_SERIALIZATION

    // need to go back up to global namespace to use BOOST_CLASS_EXPORT_GUID
//}
//BOOST_CLASS_EXPORT_GUID(Test_Polymorphic::B, "Test_Polymorphic::B")
//BOOST_CLASS_EXPORT_GUID(Test_Polymorphic::C, "Test_Polymorphic::C")
//namespace Test_Polymorphic {

    //#ifdef __MWERKS__
    //AUTO_RUN(( boost::serialization::void_cast_register<B, A>(NULL, NULL) ))
    //AUTO_RUN(( boost::serialization::void_cast_register<C, A>(NULL, NULL) ))
    //#endif

    //#endif
   
    // More recent versions of boost don't need this workaround (1.33.0+ ?)
    // gcc < 3.4 doesn't support boost::is_abstract so we have to be explicit
    //namespace boost {
    //    template<>
    //    struct is_abstract<A>
    //    {
    //        enum { value = false };
    //    };
    //}
   

    class X
    {
    public:
        std::string f1(A *a)
        {
            return typeid(*a).name();
        }
   
        //void f3(const std::string &s, A *&pa)
        //{
        //    static A *pa_ = new A;
        //    static B *pb_ = new B;
        //    static C *pc_ = new C;
        //    if (s == typeid(A).name())
        //        pa = pa_;
        //    else if (s == typeid(B).name())
        //        pa = pb_;
        //    else if (s == typeid(C).name())
        //        pa = pc_;
        //    else
        //        throw std::runtime_error("unknown type name: " + s);
        //}
   
        bool f4(A *pa1, A *pa2)
        {
            return pa1 == pa2;
        }
    };

    class Y
    {
    public:
        std::string f1(A &a)
        {
            return typeid(a).name();
        }

        bool f4(A &ra1, A &ra2)
        {
            return &ra1 == &ra2;
        }
    };

    class Z
    {
    public:
        std::string f1(boost::shared_ptr<A> a)
        {
            return typeid(*a).name();
        }

        boost::shared_ptr<A> f2(const std::string &s)
        {
            if (s == typeid(A).name())
                return boost::shared_ptr<A>(new A);
            else if (s == typeid(B).name())
                return boost::shared_ptr<B>(new B);
            else if (s == typeid(C).name())
                return boost::shared_ptr<C>(new C);
            else
                throw std::runtime_error("unknown type name: " + s);
        }
        void f3(const std::string &s, boost::shared_ptr<A> &pa)
        {
            if (s == typeid(A).name())
                pa = boost::shared_ptr<A>(new A);
            else if (s == typeid(B).name())
                pa = boost::shared_ptr<B>(new B);
            else if (s == typeid(C).name())
                pa = boost::shared_ptr<C>(new C);
            else
                throw std::runtime_error("unknown type name: " + s);
        }

        bool f4(boost::shared_ptr<A> pa1, boost::shared_ptr<A> pa2)
        {
            return pa1.get() == pa2.get();
        }

        void f5(boost::shared_ptr<A> pa1, boost::shared_ptr<A> pa2, boost::shared_ptr<A> &pa3, boost::shared_ptr<A> &pa4)
        {
            pa3 = pa1;
            pa4 = pa2;
            // enable pointer tracking for outbound serialization
            RCF::getCurrentRcfSession().enableSfSerializationPointerTracking(true);
        }
    };

} // namespace Test_Polymorphic

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Polymorphic::A)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Polymorphic::B)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Polymorphic::C)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Polymorphic::X)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Polymorphic::Y)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Polymorphic::Z)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(boost::shared_ptr<Test_Polymorphic::A>)

namespace Test_Polymorphic {

    // interface definitions
    RCF_BEGIN(I_X, "X")
        RCF_METHOD_R1(std::string, f1, A*)
        //RCF_METHOD_V2(void, f3, const std::string &, A*&)
        RCF_METHOD_R2(bool, f4, A*, A*)
    RCF_END(I_X)

    RCF_BEGIN(I_Y, "Y")
        RCF_METHOD_R1(std::string, f1, A&)
        RCF_METHOD_R2(bool, f4, A&, A&)
    RCF_END(I_Y)

    RCF_BEGIN(I_Z, "Z")
        RCF_METHOD_R1(std::string, f1, boost::shared_ptr<A>)
        RCF_METHOD_R1(boost::shared_ptr<A>, f2, const std::string &)
        RCF_METHOD_V2(void, f3, const std::string &, boost::shared_ptr<A> &)
        RCF_METHOD_R2(bool, f4, boost::shared_ptr<A>, boost::shared_ptr<A>)
        RCF_METHOD_V4(void, f5, boost::shared_ptr<A>, boost::shared_ptr<A>, boost::shared_ptr<A> &, boost::shared_ptr<A> &)
    RCF_END(I_Z)

} // namespace Test_Polymorphic

int test_main(int argc, char **argv)
{    

    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_Polymorphic;

    util::CommandLine::getSingleton().parse(argc, argv);

    RCF_ASSERT( !RCF::getTransportFactories().empty() );

    RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[0];
    RCF::TransportPair transports = transportFactoryPtr->createTransports();
    RCF::ServerTransportPtr serverTransportPtr( transports.first );
    RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

    RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

    RCF::RcfServer server(serverTransportPtr);
   
    X x;
    Y y;
    Z z;
    server.bind( (I_X*) 0, x);
    server.bind( (I_Y*) 0, y);
    server.bind( (I_Z*) 0, z);
   
    server.start();

    for (int protocol=1; protocol<=2; ++protocol)
    {
        RCF::SerializationProtocol sp = RCF::SerializationProtocol(protocol);
        if (RCF::isSerializationProtocolSupported(sp))
        {
            std::string protocolName = RCF::getSerializationProtocolName(protocol);
            std::cout << "Testing serialization protocol: " << protocol << ": " << protocolName << std::endl;

            std::string s;

            RcfClient<I_X> xClient(clientTransportAutoPtr->clone());
            RcfClient<I_Y> yClient(clientTransportAutoPtr->clone());
            RcfClient<I_Z> zClient(clientTransportAutoPtr->clone());

            xClient.getClientStub().setSerializationProtocol(sp);
            yClient.getClientStub().setSerializationProtocol(sp);
            zClient.getClientStub().setSerializationProtocol(sp);

            {
                boost::shared_ptr<A> pa(new A);
                boost::shared_ptr<A> pb(new B);
                boost::shared_ptr<A> pc(new C);

                s = xClient.f1(pa.get()); RCF_CHECK( s == typeid(A).name() );
                s = xClient.f1(pb.get()); RCF_CHECK( s == typeid(B).name() );
                s = xClient.f1(pc.get()); RCF_CHECK( s == typeid(C).name() );

                s = yClient.f1(*pa); RCF_CHECK( s == typeid(A).name() );
                s = yClient.f1(*pb); RCF_CHECK( s == typeid(B).name() );
                s = yClient.f1(*pc); RCF_CHECK( s == typeid(C).name() );

                s = zClient.f1(pa); RCF_CHECK( s == typeid(A).name() );
                s = zClient.f1(pb); RCF_CHECK( s == typeid(B).name() );
                s = zClient.f1(pc); RCF_CHECK( s == typeid(C).name() );
            }
            {
                boost::shared_ptr<A> spa;
                spa = zClient.f2( typeid(B).name() ); RCF_CHECK( dynamic_cast<B *>(spa.get()) != NULL );
                spa = zClient.f2( typeid(C).name() ); RCF_CHECK( dynamic_cast<C *>(spa.get()) != NULL );
            }
            {
                A *pa = NULL;
                //xClient.f3(typeid(B).name(), pa); RCF_CHECK( dynamic_cast<B *>(pa) != NULL );
                //xClient.f3(typeid(C).name(), pa); RCF_CHECK( dynamic_cast<C *>(pa) != NULL );

                boost::shared_ptr<A> spa;
                zClient.f3(typeid(B).name(), spa); RCF_CHECK( dynamic_cast<B *>(spa.get()) != NULL );
                zClient.f3(typeid(C).name(), spa); RCF_CHECK( dynamic_cast<C *>(spa.get()) != NULL );
            }
            {
                A *pa1 = new A;
                A *pa2 = pa1;
                A *pa3 = new A;
                A *pa4 = new A;

                A &ra1 = *pa1;
                A &ra2 = *pa2;
                A &ra3 = *pa3;
                A &ra4 = *pa4;

                boost::shared_ptr<A> spa1(pa1);
                boost::shared_ptr<A> spa2(spa1);
                boost::shared_ptr<A> spa3(pa3);
                boost::shared_ptr<A> spa4(pa4);
                boost::shared_ptr<A> spa5;
                boost::shared_ptr<A> spa6;

                // need to enable pointer tracking for following tests to pass

                xClient.getClientStub().enableSfSerializationPointerTracking(true);
                yClient.getClientStub().enableSfSerializationPointerTracking(true);
                zClient.getClientStub().enableSfSerializationPointerTracking(true);

                bool same = false;
                same = xClient.f4(pa1, pa2); RCF_CHECK( same );
                same = xClient.f4(pa3, pa4); RCF_CHECK( !same );
                same = yClient.f4(ra1, ra2); RCF_CHECK( same );
                same = yClient.f4(ra3, ra4); RCF_CHECK( !same );
                same = zClient.f4(spa1, spa2); RCF_CHECK( same );
                same = zClient.f4(spa3, spa4); RCF_CHECK( !same );

                spa5.reset();
                spa6.reset();
                zClient.f5(spa1, spa2, spa5, spa6);
                RCF_CHECK(spa5.get() && spa6.get() && spa5 == spa6);

                spa5.reset();
                spa6.reset();
                zClient.f5(spa3, spa4, spa5, spa6);
                RCF_CHECK(spa5.get() && spa6.get() && spa5 != spa6);
            }
        }
    }
           
    return boost::exit_success;
}
