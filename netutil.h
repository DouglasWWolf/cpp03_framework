//==========================================================================================================
// netutil.h - Defines some helpful utility functions for networking
//==========================================================================================================
#pragma once
#include <netinet/in.h>
#include <string>
#include <netdb.h>

struct ipv4_t
{
    unsigned char octet[4];
    std::string   text();
    void          clear();
};

struct ipv6_t
{
    unsigned char octet[16];
    std::string   text();
    void          clear();
};

struct NetUtil
{
    // These fetch a binary IP address for the local host
    static bool get_local_ip(std::string iface, ipv4_t* dest);
    static bool get_local_ip(std::string iface, ipv6_t* dest);

    // Returns addrinfo about the local machine
    static addrinfo get_local_addrinfo(int type, int port, std::string bind_to, int family);

    // Returns addrinfo about a remove server
    static bool get_server_addrinfo(int type, std::string server, int port, int family, addrinfo* p_result);

    // Fetches the ASCII IP address from a sockaddr*.  
    static std::string ip_to_string(sockaddr* addr);

    // Converts a sockaddr_storage to an ASCII IP address.
    static std::string ip_to_string(sockaddr_storage& ss);

    // Call this to wait for data to arrive on anywhere from 1 to 4 descriptors
    // timeout_ms of -1 means "wait forever"
    static int wait_for_data(int timeout_ms, int fd1, int fd2 = -1, int fd3 = -1, int fd4 = -1);
};


