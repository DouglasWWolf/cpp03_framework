//==========================================================================================================
// netutil.cpp - Implements some common networking utility functions
//==========================================================================================================
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <string>
#include "netutil.h"
using namespace std;


//==========================================================================================================
// get_local_addrinfo() - Returns an addrinfo structure for the local machine
//
// Passed:  type    = SOCK_STREAM or SOCK_DGRAM
//          port    = The TCP port number we want to create a socket on
//          bind_to = The IP address of the network card to bind to (optional)
//          family  = AF_UNSPEC, AF_INET, or AF_INET6
//
// Returns: an addrinfo structure.
//          if addrinfo.ai_family == 0, the call failed
//==========================================================================================================
addrinfo NetUtil::get_local_addrinfo(int type, int port, string bind_to, int family)
{
    char ascii_port[20];
    struct addrinfo hints, *p_res, result;

    // If we fail, our entire return structure will be zero
    memset(&result, 0, sizeof(result));

    // Get a pointer to the IP address we want to bind to
    const char* bind_addr = bind_to.empty() ? NULL : bind_to.c_str();

    // Get an ASCII version of the port number
    sprintf(ascii_port, "%i", port);

    // Tell getaddrinfo about the socket family and type
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = family;  
    hints.ai_socktype = type;
    
    // Handle the case where we're not binding to a specific IP address
    if (bind_addr == NULL) hints.ai_flags = AI_PASSIVE;  

    // Fetch important information about the socket we're going to create
    if (getaddrinfo(bind_addr, ascii_port, &hints, &p_res) != 0) return result;

    // If we didn't get a result from getaddrinfo, something's wrong
    if (p_res == NULL) return result;

    // Save a copy of the results
    result = *p_res;

    // Free the memory that was allocated by getaddrinfo
    freeaddrinfo(p_res);

    // And hand the result to the caller
    return result;
}
//==========================================================================================================



//==========================================================================================================
// get_server_addrinfo() - Returns connection information for a remote server
//==========================================================================================================
bool NetUtil::get_server_addrinfo(int type, string server, int port, int family, addrinfo* p_result)
{
    char ascii_port[20];
    struct addrinfo hints, *p_res;

    // If we fail, our entire return structure will be zero
    memset(p_result, 0, sizeof(addrinfo));

    // Get an ASCII version of the port number
    sprintf(ascii_port, "%i", port);

    // We're going to build an IPv4/IPv6 TCP socket
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = family;
    hints.ai_socktype = type;
    
    // Get information about this server.  If we can't, it doesn't exist
    if (getaddrinfo(server.c_str(), ascii_port, &hints, &p_res) != 0) return false;

    // If we didn't get a result from getaddrinfo, something's wrong
    if (p_res == NULL) return false;

    // Save a copy of the results
    *p_result = *p_res;

    // Free the memory that was allocated by getaddrinfo
    freeaddrinfo(p_res);

    // Tell the caller that we have information about the server he wants to connect o
    return true;
}
//==========================================================================================================



//==========================================================================================================
// get_local_ip() - Fetches the IP address of the local machine
//
// Passed:  iface  = Name of the interface ("eth0", "eth1", etc)
//          family = AF_INET or AF_INET6
//
// Returns: The IP address (in either IPv4 or IPv6 format), or "" if none could be found
//==========================================================================================================
string NetUtil::get_local_ip(string iface, int family)
{
    string ip_address;
    struct ifaddrs *ifaddr, *ifa;

    // Fetch the list of network interfaces
    if (getifaddrs(&ifaddr) < 0) return "";

    // Walk through the linked list of interface information entries
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        // Get a handy reference to this entry
        ifaddrs& entry = *ifa;

        // If this entry is for a different interface, skip it
        if (entry.ifa_name != iface) continue;

        // Get a convenient pointer to the IP address for this entry
        sockaddr* addr = entry.ifa_addr;

        // If there's no IP address for this entry, skip it
        if (addr == NULL) continue;

        // If this entry is for the wrong family, skip it
        if (addr->sa_family != family) continue;

        // Fetch the ASCII IP address for this entry
        ip_address = ip_to_string(addr);

        // On the off chance that the conversion to ASCII failed, skip this entry
        if (ip_address.empty()) continue;

        // And we have our IP address
        break;
    }

    // Free the linked-list that was allocated by getifaddrs()
    freeifaddrs(ifaddr);

    // Hand the resulting IP address to the caller
    return ip_address;
}
//==========================================================================================================


//==========================================================================================================
// get_local_ip() - Fetches the local IP address in binary
//==========================================================================================================
bool NetUtil::get_local_ip(string iface, int family, void* buffer, size_t bufsize)
{
    // Ensure that the caller's buffer is big enough 
    if (family == AF_INET  && bufsize < 4 ) return false;
    if (family == AF_INET6 && bufsize < 16) return false;

    // Fetch the local IP address as a string
    string ip = get_local_ip(iface, family);

    // If we couldn't fetch an IP address, tell the caller
    if (ip.empty()) return false;

    // Convert the ASCII IP address to binary, and tell the caller if it worked
    return inet_pton(family, ip.c_str(), buffer) > 1;
}
//==========================================================================================================



//==========================================================================================================
// ip_to_string() - Converts an IP address to a string
//==========================================================================================================
string NetUtil::ip_to_string(sockaddr* addr)
{
    // This is the field that will get returned
    char ip_address[64] = {0};

    // Figure out how long the socket address structure is for this family
    socklen_t socklen = (addr->sa_family == AF_INET) ? 
              sizeof(struct sockaddr_in) :
              sizeof(struct sockaddr_in6);

    // Fetch the ASCII IP address for this entry
    int rc = getnameinfo(addr, socklen, ip_address, sizeof(ip_address), NULL, 0, NI_NUMERICHOST);

    // On the off chance that the call to getnameinfo() fails, skip this entry
    if (rc != 0) return "";

    // Is there a '%' in this IP address?
    char* p = strchr(ip_address, '%');
        
    // If so, truncate the IP address at that '%' character
    if (p) *p = 0;

    // Hand the resulting IP address to the caller
    return ip_address;
}
//==========================================================================================================


//==========================================================================================================
// ip_to_string() - Converts an IP address to a string
//==========================================================================================================
std::string NetUtil::ip_to_string(sockaddr_storage& ss)
{
    return ip_to_string((sockaddr*)&ss);
}
//==========================================================================================================


//==========================================================================================================
// wait_for_data() - Waits for the specified amount of time for data to be available for reading
//
// Passed:  timeout_ms = timeout in milliseconds.  -1 = Wait forever
//
// Returns: a bitmap of which descriptors are available for reading
//==========================================================================================================
int NetUtil::wait_for_data(int timeout_ms, int fd1, int fd2, int fd3, int fd4)
{
    int    i, fd;
    fd_set rfds;
    timeval timeout;

    // Put them into an array
    int fd_list[] = {fd1, fd2, fd3, fd4};

    // Find out how many items are in the array
    const int ARRAY_COUNT = sizeof(fd_list) / sizeof(fd_list[0]);

    // We don't know what the highest file descriptor is yet
    int highest_fd = -1;

    // Clear our file descriptor set
    FD_ZERO(&rfds);

    // Loop through each file descriptor that was provided by the caller...
    for (i=0; i<ARRAY_COUNT; ++i)
    {
        // Fetch this file descriptor
        fd = fd_list[i];

        // Skip any invalid file descriptor
        if (fd < 0) continue;   

        // Include this file descriptor in our fd_set
        FD_SET(fd, &rfds);

        // Keep track of the highest file descriptor we have
        if (fd > highest_fd) highest_fd = fd;
    }

    // Assume for the moment that we are going to wait forever
    timeval* p_timeout = NULL;

    // If the caller wants us to wait for a finite amount of time...
    if (timeout_ms != -1)
    {
        // Convert milliseconds to microseconds
        int usecs = timeout_ms * 1000;

        // Determine the timeout in seconds and microseconds
        timeout.tv_sec  = usecs / 1000000;
        timeout.tv_usec = usecs % 1000000;

        // Point to the timeout structure we just initialized
        p_timeout = &timeout;
    }

    // Wait for one of the descriptors to become available for reading
    if (select(highest_fd+1, &rfds, NULL, NULL, p_timeout) < 1) return 0;

    // This is going to be a bitmap of which descriptors are readable
    int result = 0;

    // Loop through each possible descriptor...
    for (i=0; i<ARRAY_COUNT; ++i)
    {
        // Fetch this file descriptor
        fd = fd_list[i];

        // Skip any invalid file descriptor
        if (fd < 0) continue;   

        // If this descriptor is readable, set the appropriate bit in the result
        if (FD_ISSET(fd, &rfds)) result |= (1 << i);
    }

    // Hand the caller a bitmap of which of his descriptors are readable
    return result;
}
//==========================================================================================================

