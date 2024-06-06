#include <stddef.h>
#include <iostream>
#include <functional>
#include <sstream>
#include <tuple>

#include <gflags/gflags.h>

#include "Socket.hpp"

DEFINE_string(host, "127.0.0.1", "IP address the client sends to");
DEFINE_uint32(port, 1060, "TCP port number");
DEFINE_bool(client, false, "run as the client");

std::ostream& operator <<(std::ostream& out, const std::tuple<std::string, uint16_t>& address)
{
    out << "(" << std::get<0>(address) << ", " << std::get<1>(address) << ")";
    return out;
}

std::string usage(const char* prog) {
    std::ostringstream os;
    os << "\nusage: " << prog << " [--help] [--client] [--host HOST] [--port PORT]\n\n"
        << "Transmit & receive a data stream\n";
    return os.str();
}

void server(const char* host, uint16_t port) {
    Socket sock(AF_INET, SOCK_STREAM);
    sock.Setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);
    sock.Bind(host, port);
    sock.Listen(1);
    std::cout << "Run this script in another window with '--client' to connect\n";
    std::cout << "Listening at " << sock.Getsockname() << "\n";
    std::tuple<std::string, uint16_t> sockname;
    Socket sc = sock.Accept(&sockname);
    std::cout << "Accepted connection from " << sockname << "\n";
    sc.Shutdown(SHUT_WR);
    std::string message;
    while (true) {
        std::string more = sc.Recv(8192);
        if (more.empty()) {
            std::cout << "Received zero bytes - end of file\n";
            break;
        }
        std::cout << "Received " << more.length() << " bytes\n";
        message += more;
    }
    std::cout << "Message:\n";
    std::cout << message << std::endl;
    sc.Close();
    sock.Close();
}

void client(const char* host, uint16_t port) {
    Socket sock(AF_INET, SOCK_STREAM);
    sock.Connect(host, port);
    sock.Shutdown(SHUT_RD);
    sock.Sendall("Beautiful is better than ugly.\n");
    sock.Sendall("Explicit is better than implicit.\n");
    sock.Sendall("Simple is better than complex.\n");
    sock.Close();
}

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage(usage(argv[0]));
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::function<void(const char*, uint16_t)> function;
    if (FLAGS_client) {
        function = &client;
    } else {
        function = &server;
    }
    function(FLAGS_host.c_str(), FLAGS_port);

    return 0;
}