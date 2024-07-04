#include "http_tcp_server.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>

namespace
{
    const int BUFFER_SIZE = 30720;

    void log(const std::string &message)
    {
        std::cout << message << std::endl;
    }

    void exitWithError(const std::string &errorMessage)
    {
        log("ERROR: " + errorMessage);
        exit(1);
    }
}

namespace http
{
    TcpServer::TcpServer(std::string ip_address, int port)
      : m_ip_address(ip_address), m_port(port), m_socket(),
        m_new_socket(), m_incomingMessage(), m_socketAddress(),
        m_socketAddress_len(sizeof(m_socketAddress))
    {    
        m_socketAddress.sin_family = AF_INET;
        m_socketAddress.sin_port = htons(m_port);
        m_socketAddress.sin_addr.s_addr = inet_addr(m_ip_address.c_str());
        
        if (startServer() != 0)
        {
            std::ostringstream ss;
            ss << "Failed to start server with PORT: " << ntohs(m_socketAddress.sin_port);
            log(ss.str());
        }
    }

    TcpServer::~TcpServer()
    {
        closeServer();
    }

    int TcpServer::startServer()
    {        
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0)
        {
            exitWithError("Cannot create socket");
            return 1;
        }
        
        if (bind(m_socket, (sockaddr *)&m_socketAddress, m_socketAddress_len) < 0)
        {
            exitWithError("Cannot connect socket to address");
            return 1;
        }

        return 0;
    }

    void TcpServer::closeServer()
    {
        close(m_socket);
        close(m_new_socket);
        exit(0);
    } 
    
    void TcpServer::startListen()
    {
        if (listen(m_socket, 20) < 0)
        {
            exitWithError("Socket listen failed");
        }

        std::ostringstream ss;
        ss << "\n*** Listening on ADDRESS: " << inet_ntoa(m_socketAddress.sin_addr) << " PORT: " << ntohs(m_socketAddress.sin_port) << " ***\n\n";
        log(ss.str());

        int bytesReceived;

        while (true)
        {
            log("====== Waiting for a new connection ======\n\n\n");
            acceptConnection(m_new_socket);

            char buffer[BUFFER_SIZE] = {0};
            bytesReceived = read(m_new_socket, buffer, BUFFER_SIZE);
            if (bytesReceived < 0)
            {
                exitWithError("Failed to read bytes from client socket connection");
            }
        

            std::ostringstream ss;
            ss << "------ Received Request from client ------\n\n";
            log(ss.str());

            std::string request_string(buffer); //construct string from null-terminated C string.
            handleRequest(request_string);

            close(m_new_socket);
        }
    }
    void TcpServer::handleRequest(std::string request) {
        int method_index = request.find(" ");
        std::string method = request.substr(0, method_index);
        std::ostringstream ss;
        ss << "parsed method: " << method << std::endl;
        log(ss.str());
        int uri_index = request.find(" ", method_index+1); // grab the string after method up till the next space.
        std::string uri = request.substr(method_index+1, uri_index-(method_index+1));
        ss.str("");
        ss << "parsed URI: '" << uri <<"'"<< std::endl;
        log(ss.str());        
        if (method == "GET") { //NOTE: C++ strings implement == operator, can't do in C!
            handleGetMethod(uri);
        }
        else if (method == "HEAD") { //NOTE: C++ strings implement == operator, can't do in C!
            handleHeadMethod(uri);
        }
        else { //501 may be more appropriate here, but this is temporary code.
            sendResponse(build405Response()); //Method Not Allowed.
        }
    }
    void TcpServer::handleGetMethod(std::string uri) {
            if (uri == "/index.html" || uri == "/") {
                sendResponse(buildHomepageResponse(true));
            }
            else {
                sendResponse(build404Response());
            }
    }
    void TcpServer::handleHeadMethod(std::string uri) {
            if (uri == "/index.html" || uri == "/") {
                sendResponse(buildHomepageResponse(false));
            }
            else {
                sendResponse(build404Response());
            }

    }    
    void TcpServer::acceptConnection(int &new_socket) {
        new_socket = accept(m_socket, (sockaddr *)&m_socketAddress, &m_socketAddress_len);
        if (new_socket < 0)
        {
            std::ostringstream ss;
            ss << "Server failed to accept incoming connection from ADDRESS: " << inet_ntoa(m_socketAddress.sin_addr) << "; PORT: " << ntohs(m_socketAddress.sin_port);
            exitWithError(ss.str());
        }
    }
    std::string TcpServer::buildHomepageResponse(bool include_content) {
        std::ifstream html_file("../demo_content/index.html");
        std::string html_string((std::istreambuf_iterator<char>(html_file)), std::istreambuf_iterator<char>());
        std::ostringstream ss;
        ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " <<html_string.size() << "\n\n" << html_string;
        std::string response(ss.str());
        if (include_content == true) {
            response.append(html_string);
        }
        return response;
    }
    std::string TcpServer::build404Response() {
        std::ifstream html_file("../demo_content/404-page.html");
        std::string html_string((std::istreambuf_iterator<char>(html_file)), std::istreambuf_iterator<char>());
        std::ostringstream ss;
        ss << "HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: " <<html_string.size() << "\n\n" << html_string;
        std::string response(ss.str());
        response.append(html_string);
        return response;
    }
    std::string TcpServer::build405Response() {
        std::ifstream html_file("../demo_content/405-page.html");
        std::string html_string((std::istreambuf_iterator<char>(html_file)), std::istreambuf_iterator<char>());
        std::ostringstream ss;
        ss << "HTTP/1.1 405 Method Not Allowed\nAllow: GET, HEAD "<< "\n\n" << html_string;
        std::string response(ss.str());
        response.append(html_string);
        return response;
    } 
    void TcpServer::sendResponse(std::string response) {
        long bytesSent;
        bytesSent = write(m_new_socket, response.c_str(), response.size());
        if (bytesSent == response.size()) {
            log("------ Server Response sent to client ------\n\n");
        }
        else {
            log("Error sending response to client");
        }
    }
}