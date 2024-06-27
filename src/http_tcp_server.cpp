#include "http_tcp_server.h"

#include <iostream>
#include <sstream>
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

    /**
     * Locates the last occurence of a period within the full file name.  Returns the file extension including the period.
     * 
     * @param file_name pointer to the full file name to proccess.
     * @return pointer of the file extension.
     */
    const char *get_file_extension(const char *file_name)
    {
        const char *dot = strrchr(file_name, '.');
        if (!dot || dot == file_name)
        {
            return "";
        }
        return dot + 1;
    }

    /**
     * Case insentitive comparision of two strings.  Returns true if they are equal, otherwise returns false.
     * 
     * @param s1 first string to compare.
     * @param s2 second string to compare.
     * @return boolean for whether the strings are equal.
     */
    bool case_insensitive_compare(const char *s1, const char *s2) {
        while (*s1 && *s2) {
            if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
                return false;
            }
            s1++;
            s2++;
        }
        return *s1 == *s2;
    }

    /**
     * Opens the current directory and returns the file if found.  If not found, returns 404.
     * 
     * @param file_name
     * @return the file name.
     */
    char *get_file_case_insensitive(const char *file_name) 
    {
        DIR *dir = opendir(".");
        if (dir == NULL) 
        {
            perror("opendir");
            return NULL;
        }

        struct dirent *entry;
        char *found_file_name = NULL;
        while ((entry = readdir(dir)) != NULL) 
        {
            if (case_insensitive_compare(entry->d_name, file_name)) 
            {
                found_file_name = entry->d_name;
                break;
            }
        }

        closedir(dir);
        return found_file_name;
    }
}

namespace http
{
    TcpServer::TcpServer(std::string ip_address, int port)
      : m_ip_address(ip_address), m_port(port), m_socket(),
        m_new_socket(), m_incomingMessage(), m_socketAddress(),
        m_socketAddress_len(sizeof(m_socketAddress)),
        m_serverMessage(buildResponse())
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

            sendResponse();

            close(m_new_socket);
        }
    }

    void TcpServer::acceptConnection(int &new_socket)
    {
        new_socket = accept(m_socket, (sockaddr *)&m_socketAddress, &m_socketAddress_len);
        if (new_socket < 0)
        {
            std::ostringstream ss;
            ss << "Server failed to accept incoming connection from ADDRESS: " << inet_ntoa(m_socketAddress.sin_addr) << "; PORT: " << ntohs(m_socketAddress.sin_port);
            exitWithError(ss.str());
        }
    }

    std::string TcpServer::buildResponse()
    {
        std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";
        std::ostringstream ss;
        ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
           << htmlFile;

        return ss.str();
    }

    void TcpServer::sendResponse()
    {
        long bytesSent;

        bytesSent = write(m_new_socket, m_serverMessage.c_str(), m_serverMessage.size());

        if (bytesSent == m_serverMessage.size())
        {
            log("------ Server Response sent to client ------\n\n");
        }
        else
        {
            log("Error sending response to client");
        }
    }
}