
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "MyService.hpp"

#include <RCF/TcpEndpoint.hpp>

int main()
{
    try
    {
        // Setup a vector of strings.
        std::vector<std::string> v;
        v.push_back("one");
        v.push_back("two");
        v.push_back("three");

        // Print them out.
        std::cout << "Before:\n";
        std::copy(
            v.begin(), 
            v.end(), 
            std::ostream_iterator<std::string>(std::cout, "\n"));

        // Make the call.
        RcfClient<MyService>( RCF::TcpEndpoint("127.0.0.1", 50001) ).reverse(v);

        // Print them out again. This time they are in reverse order.
        std::cout << "\nAfter:\n";
        std::copy(
            v.begin(), 
            v.end(), 
            std::ostream_iterator<std::string>(std::cout, "\n"));
    }
    catch(const RCF::Exception & e)
    {
        std::cout << "Caught exception:\n";
        std::cout << e.getError().getErrorString() << std::endl;
    }

    return 0;
}
