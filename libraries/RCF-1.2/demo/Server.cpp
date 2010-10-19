
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <RCF/RcfServer.hpp>
#include <RCF/TcpEndpoint.hpp>

#include "MyService.hpp"

class MyServiceImpl
{
public:
    // Reverses the order of strings in the vector.
    void reverse(std::vector<std::string> &v)
    {
        std::cout << "Reversing a vector of strings...\n";
        std::vector<std::string> w;
        std::copy(v.rbegin(), v.rend(), std::back_inserter(w));
        v.swap(w);
    }
};

int main()
{
    // Start a TCP server on port 50001, and expose MyServiceImpl.
    MyServiceImpl myServiceImpl;
    RCF::RcfServer server( RCF::TcpEndpoint("0.0.0.0", 50001) );
    server.bind<MyService>(myServiceImpl);
    server.start();

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}
