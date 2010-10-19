#include <vector>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Tools.hpp>
#include <RCF/test/PrintTestHeader.hpp>
#include <RCF/util/CommandLine.hpp>
#include <RCF/util/Profile.hpp>

namespace Test_NetworkPerformance {

    int testUdp(const std::string &ip, int port, bool server, bool client, int calls, int buflen)
    {
        int ret = 0;

        sockaddr_in clientAddr = {0};
        clientAddr.sin_family = AF_INET;
        clientAddr.sin_port = 0;
        clientAddr.sin_addr.s_addr = INADDR_ANY;

        sockaddr_in serverAddr = {0};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        sockaddr_in toAddr = {0};
        toAddr.sin_family = AF_INET;
        toAddr.sin_port = htons(port);
        toAddr.sin_addr.s_addr = inet_addr(ip.c_str());

        sockaddr_in fromAddr = {0};
        int fromAddrLen = sizeof(fromAddr);

        int socket1 = socket( AF_INET, SOCK_DGRAM, 0 );

        int packetsSent = 0;
        int packetsReceived = 0;
        int bytesSent = 0;
        int bytesReceived = 0;

        if (client)
        {
            ret = bind(socket1, (sockaddr *)&clientAddr, sizeof(clientAddr));
            RCF_VERIFY(ret == 0, std::runtime_error("bind()"));

            std::string msg(buflen, 'A');
            std::vector<char> buffer(buflen);

            {
                util::Profile profile("Total time for client calls");
                for (int i=0; i<calls; ++i)
                {
                    ret = Platform::OS::BsdSockets::sendto(socket1, msg.c_str(), msg.size(), 0, (sockaddr *)&toAddr, sizeof(toAddr));
                    RCF_VERIFY(ret > 0, std::runtime_error("sendto()"));
                    bytesSent += ret;
                    packetsSent += 1;

                    ret = Platform::OS::BsdSockets::recvfrom(socket1, &buffer[0], buffer.size(), 0, (sockaddr *)&fromAddr, &fromAddrLen);
                    RCF_VERIFY(ret > 0, std::runtime_error("recvfrom()"));
                    bytesReceived += ret;
                    packetsReceived += 1;
                }
            }

            std::cout << "Packets sent: "       << packetsSent      << std::endl;
            std::cout << "Bytes sent: "         << bytesSent        << std::endl;
            std::cout << "Packets received: "   << packetsReceived  << std::endl;
            std::cout << "Bytes received: "     << bytesReceived    << std::endl;
        }

        else if (server)
        {
            ret = bind(socket1, (sockaddr *)&serverAddr, sizeof(serverAddr));
            RCF_VERIFY(ret == 0, std::runtime_error("bind()"));

            std::vector<char> buffer(buflen);

            while (true)
            {
                ret = Platform::OS::BsdSockets::recvfrom(socket1, &buffer[0], buffer.size(), 0, (sockaddr *)&fromAddr, &fromAddrLen);
                RCF_VERIFY(ret > 0, std::runtime_error("recvfrom()"));
                bytesSent += ret;
                packetsSent += 1;

                ret = Platform::OS::BsdSockets::sendto(socket1, &buffer[0], ret, 0, (sockaddr *)&fromAddr, fromAddrLen);
                RCF_VERIFY(ret > 0, std::runtime_error("sendto()"));
                bytesReceived += ret;
                packetsReceived += 1;
            }
        }

        Platform::OS::BsdSockets::closesocket(socket1);

        return boost::exit_success;

    }

    int testTcp(const std::string &ip, int port, bool server, bool client, int calls, int buflen)
    {
        int ret = 0;

        sockaddr_in clientAddr = {0};
        clientAddr.sin_family = AF_INET;
        clientAddr.sin_port = 0;
        clientAddr.sin_addr.s_addr = INADDR_ANY;

        sockaddr_in serverAddr = {0};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        sockaddr_in toAddr = {0};
        toAddr.sin_family = AF_INET;
        toAddr.sin_port = htons(port);
        toAddr.sin_addr.s_addr = inet_addr(ip.c_str());

        sockaddr_in fromAddr = {0};
        int fromAddrLen = sizeof(fromAddr);

        int socket1 = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        int packetsSent = 0;
        int packetsReceived = 0;
        int bytesSent = 0;
        int bytesReceived = 0;

        if (client)
        {
            ret = Platform::OS::BsdSockets::connect(socket1, (sockaddr *)&toAddr, sizeof(toAddr));
            RCF_VERIFY(ret == 0, std::runtime_error("bind()"));

            std::string msg(buflen, 'A');
            std::vector<char> buffer(buflen);

            {
                util::Profile profile("Total time for client calls");
                for (int i=0; i<calls; ++i)
                {
                    ret = Platform::OS::BsdSockets::send(socket1, msg.c_str(), msg.size(), 0);
                    RCF_VERIFY(ret == buflen, std::runtime_error("send()"))(ret);
                    bytesSent += ret;
                    packetsSent += 1;

                    ret = Platform::OS::BsdSockets::recv(socket1, &buffer[0], buffer.size(), 0);
                    RCF_VERIFY(ret == buflen, std::runtime_error("recv()"))(ret);
                    bytesReceived += ret;
                    packetsReceived += 1;
                }
            }

            std::cout << "Packets sent: "       << packetsSent      << std::endl;
            std::cout << "Bytes sent: "         << bytesSent        << std::endl;
            std::cout << "Packets received: "   << packetsReceived  << std::endl;
            std::cout << "Bytes received: "     << bytesReceived    << std::endl;
        }

        else if (server)
        {
            ret = bind(socket1, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
            RCF_VERIFY(ret == 0, std::runtime_error("bind()"));

            ret = listen(socket1, 10);
            RCF_VERIFY(ret == 0, std::runtime_error("listen()"));

            std::vector<char> buffer(buflen);

            while (true)
            {
                int socket2 = Platform::OS::BsdSockets::accept(socket1, (sockaddr *)&fromAddr, &fromAddrLen);
                RCF_VERIFY(socket2 > 0, std::runtime_error("accept()"))(socket2);

                while (true)
                {
                    ret = Platform::OS::BsdSockets::recv(socket2, &buffer[0], buffer.size(), 0);
                    RCF_VERIFY(ret == 0 || ret == buflen, std::runtime_error("recv()"))(ret);

                    if (ret == 0)
                    {
                        Platform::OS::BsdSockets::closesocket(socket2);
                        break;
                    }

                    bytesSent += ret;
                    packetsSent += 1;

                    ret = Platform::OS::BsdSockets::send(socket2, &buffer[0], ret, 0);
                    RCF_VERIFY(ret == buflen, std::runtime_error("send()"))(ret);

                    bytesReceived += ret;
                    packetsReceived += 1;
                }
            }
        }

        Platform::OS::BsdSockets::closesocket(socket1);

        return boost::exit_success;
    }

} // namespace Test_NetworkPerformance

int test_main(int argc, char **argv)
{

    printTestHeader(__FILE__);

    using namespace Test_NetworkPerformance;

    util::CommandLineOption<bool>           tcp("tcp", true, "use tcp");
    util::CommandLineOption<bool>           udp("udp", false, "use udp");
    util::CommandLineOption<std::string>    ip("ip", "127.0.0.1", "ip address");
    util::CommandLineOption<int>            port( "port", 50001, "port number" );
    util::CommandLineOption<bool>           server("server", false, "act as server");
    util::CommandLineOption<bool>           client("client", false, "act as client");
    util::CommandLineOption<int>            calls("calls", 1, "number of round trip calls");
    util::CommandLineOption<int>            buflen("buflen", 50, "message size (bytes)");
    util::CommandLine::getSingleton().parse(argc, argv);

    if (tcp)
    {
        return testTcp(ip, port, server, client, calls, buflen);
    }
    else if (udp)
    {
        return testUdp(ip, port, server, client, calls, buflen);
    }
    else
    {
        RCF_VERIFY(0, std::runtime_error("no protocol specified (tcp or udp?)"));
    }

}
