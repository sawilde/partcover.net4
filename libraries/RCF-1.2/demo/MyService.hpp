
#ifndef INCLUDE_MYSERVICE_HPP
#define INCLUDE_MYSERVICE_HPP

#include <string>
#include <vector>

#include <RCF/Idl.hpp>
#include <SF/vector.hpp>

RCF_BEGIN(MyService, "MyService")
    RCF_METHOD_V1(void, reverse, std::vector<std::string> &);
RCF_END(MyService);

#endif // ! INCLUDE_MYSERVICE_HPP
