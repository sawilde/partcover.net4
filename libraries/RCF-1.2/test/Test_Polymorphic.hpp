
#ifndef INCLUDE_RCF_TEST_POLYMORPHIC_HPP
#define INCLUDE_RCF_TEST_POLYMORPHIC_HPP

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/base_object.hpp>
#include <RCF/Idl.hpp>

class A
{
public:
    A() : a(RCF_DEFAULT_INIT) {}
    A(int a) : a(a) {}
    int a;

    virtual int sum() { return a; }

    template<typename Archive>
    void serialize(Archive &ar, unsigned int)
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

    virtual int sum() { return a + b; }

    template<typename Archive>
    void serialize(Archive &ar, unsigned int)
    {
        ar & boost::serialization::base_object<A>(*this);
        ar & b;
    }

};

class C : public A
{
public:
    C() : A(), b(RCF_DEFAULT_INIT), c(RCF_DEFAULT_INIT) {}
    C(int a, int b, int c) : A(a),  b(b), c(c) {}
    int b;
    int c;

    virtual int sum() { return a + b + c; }

    template<typename Archive>
    void serialize(Archive &ar, unsigned int)
    {
        ar & boost::serialization::base_object<A>(*this);
        ar & b;
        ar & c;
    }

};

RCF_BEGIN(I_X, "X")
    RCF_METHOD_R1(std::string, f1, A*)
    RCF_METHOD_V2(void, f3, const std::string &, A*&)
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
RCF_END(I_Z)


BOOST_CLASS_EXPORT_GUID(B, "my B class")
BOOST_CLASS_EXPORT_GUID(C, "my C class")

#endif // ! INCLUDE_RCF_TEST_POLYMORPHIC_HPP
