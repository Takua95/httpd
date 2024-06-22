#include <httpc_tcpServer_lnux.h>

int main()
{
    using namespace http;

    TcpServer server = TcpServer("0.0.0.0", 8080);
    server.startListen();

    retrun 0;
}