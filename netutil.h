//==========================================================================================================
// netutil.h - Defines some helpful utility functions for networking
//==========================================================================================================
#pragma once
#include <netinet/in.h>
#include <string>
#include <stdexcept>

struct NetUtil
{
    // Returns addrinfo about the local machine
    static addrinfo get_local_addrinfo(int type, int port, std::string bind_to, int family);

    // Returns addrinfo about a remove server
    static bool get_server_addrinfo(int type, std::string server, int port, int family, addrinfo* p_result);

    // Fetch the IP address of the local machine. family should be AF_INET or AF_INET6
    static std::string get_local_ip(std::string iface, int family = AF_INET);

    // Fetches the ASCII IP address from a sockaddr*.  (Works for IPv4 and IPv6)
    static std::string ip_to_string(sockaddr* addr);
};


