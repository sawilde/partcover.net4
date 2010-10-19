
#ifndef RCF_USE_BOOST_XML_SERIALIZATION
#error Need RCF_USE_BOOST_XML_SERIALIZATION
#endif

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/IpServerTransport.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/TcpEndpoint.hpp>

class A1
{
public:
    int x;
    int y;

    void serialize(SF::Archive &ar)
    {
        ar & x & y;
    }

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & boost::serialization::make_nvp("X", x);
        ar & boost::serialization::make_nvp("Y", y);
    }

    bool operator==(const A1 & rhs)
    {
        return x == rhs.x && y == rhs.y;
    }
};

class A2
{
public:
    int x;
    int y;
    int z;

    void serialize(SF::Archive &ar)
    {
        ar & x & y & z;
    }

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar & boost::serialization::make_nvp("X", x);
        ar & boost::serialization::make_nvp("Y", y);
        ar & boost::serialization::make_nvp("Z", z);
    }

    bool operator==(const A2 & rhs)
    {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }
};

RCF_BEGIN(I_X1, "I_X")
RCF_METHOD_R1(A1, echo, A1)
RCF_END(I_X1)

RCF_BEGIN(I_X2, "I_X")
RCF_METHOD_R1(A2, echo, A2)
RCF_END(I_X2)

class X1
{
public:
    A1 echo(A1 a1)
    {
        return a1;
    }
};

class X2
{
public:
    A2 echo(A2 a2)
    {
        return a2;
    }
};

int test_main(int, char **)
{
    RCF::RcfServer server( RCF::TcpEndpoint(0));
    server.start();

    int port = server.getIpServerTransport().getPort();
    {
        X1 x1;
        server.bind<I_X1>(x1);

        RcfClient<I_X1> client(( RCF::TcpEndpoint(port)));
        A1 a1_1 = {5,6};
        A1 a1_2 = client.echo(a1_1);
        RCF_CHECK(a1_1 == a1_2);
    }

    {
        // If we had proper nvp serialization, we could bind to I_X1.
        //X1 x1;
        //server.bind<I_X1>(x1);
        X2 x2;
        server.bind<I_X2>(x2);

        RcfClient<I_X2> client(( RCF::TcpEndpoint(port)));
        A2 a2_1 = {5,6,7};

        client.getClientStub().setSerializationProtocol(RCF::Sp_BsXml);
        A2 a2_2 = client.echo(a2_1);
        RCF_CHECK(a2_1.x == a2_2.x && a2_1.y == a2_2.y);
    }

    return 0;
}
