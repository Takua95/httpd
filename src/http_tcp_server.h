#ifndef INCLUDED_HTTP_TCPSERVER_LINUX
#define INCLUDED_HTTP_TCPSERVER_LINUX

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string>

namespace http 
{
    class TcpServer
    {
        public:
            TcpServer(std::string ip_address, int port);
            ~TcpServer();
            void startListen();

        private:
            std::string m_ip_address;
            int m_port;
            int m_socket;
            int m_new_socket;
            long m_incomingMessage;
            struct sockaddr_in m_socketAddress;
            struct in_addr;
            unsigned int m_socketAddress_len;

            int startServer();
            void closeServer();
            void acceptConnection(int &new_socket);
            std::string buildHomepageResponse();
            std::string build404Response();
            void sendResponse(std::string);
            void handleRequest(std::string request);
            void handleGetMethod(std::string uri);

    };
}

#endif