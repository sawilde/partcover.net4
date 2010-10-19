
#ifndef INCLUDE_RCF_TEST_ENDPOINT_HPP
#define INCLUDE_RCF_TEST_ENDPOINT_HPP

#include <string>

#include <RCF/Idl.hpp>

namespace Test_Endpoint {

    // borland c gets its arg dependent lookup confused for some reason, the extra namespace seems to help

    struct GetEndpointName
    {
        GetEndpointName(const std::string &name) : mName(name) {}
        std::string getEndpointName() { return mName; }
        std::string mName;
    };

    #ifdef __BORLANDC__
    namespace A {
    #endif

        RCF_BEGIN(I_GetEndpointName, "I_GetEndpointName")
            RCF_METHOD_R0(std::string, getEndpointName)
        RCF_END(I_GetEndpointName)

    #ifdef __BORLANDC__
    } // namespace A
    #endif

    #ifdef __BORLANDC__
    typedef A::I_GetEndpointName MyInterface;
    #else
    typedef I_GetEndpointName MyInterface;
    #endif

    typedef MyInterface::RcfClientT MyRcfClient;

} // namespace Test_Endpoint

#endif // ! INCLUDE_RCF_TEST_ENDPOINT_HPP

