#include "http_tcp_server.h"

int main()
{
    using namespace http;

    TcpServer server = TcpServer("0.0.0.0", 8080);
    server.startListen();

    return 0;
}