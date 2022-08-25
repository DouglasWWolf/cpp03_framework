//==========================================================================================================
// netutil.h - Defines some helpful utility functions for networking
//==========================================================================================================
#pragma once
#include <netinet/in.h>
#include <string>
#include <stdexcept>
#include <netdb.h>

struct NetUtil
{
    // Fetch the ASCII IP address of the local machine. family should be AF_INET or AF_INET6
    static std::string get_local_ip(std::string iface, int family = AF_INET);

    // Fetch the binary IP address of the local machine. family should be AF_INET or AF_INET6
    static bool get_local_ip(std::string iface, int family, void* buffer, size_t bufsize);

    // Returns addrinfo about the local machine
    static addrinfo get_local_addrinfo(int type, int port, std::string bind_to, int family);

    // Returns addrinfo about a remove server
    static bool get_server_addrinfo(int type, std::string server, int port, int family, addrinfo* p_result);

    // Fetches the ASCII IP address from a sockaddr*.  
    static std::string ip_to_string(sockaddr* addr);

    // Converts a sockaddr_storage to an ASCII IP address.
    static std::string ip_to_string(sockaddr_storage& ss);
};


