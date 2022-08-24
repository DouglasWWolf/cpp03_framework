//==========================================================================================================
// udpsock.cpp - Implements a class that manages UDP sockets
//==========================================================================================================
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "udpsock.h"
#include "netutil.h"
using namespace std;





//==========================================================================================================
// create_sender() - Create a socket useful for sending UDP packets
//==========================================================================================================
bool UDPSock::create_sender(int port, string server, int family)
{
    int broadcast = 1;

    // If the socket is open, close it
    close();

    // Replace the word "broadcast" with an IP address
    if (server == "broadcast") server = "255.255.255.255";

    // Fetch information about the server we're trying to connect to
    if (!NetUtil::get_server_addrinfo(SOCK_DGRAM, server, port, family, &m_target)) return false;

    // Create the socket
    m_sd = socket(m_target.ai_family, m_target.ai_socktype, m_target.ai_protocol);

    // If that failed, tell the caller
    if (m_sd < 0) return false;

    // If we're broadcasting, set up the socket for broadcast
    if (server == "255.255.255.255")
    {
        if (setsockopt(m_sd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1)
        {
            return false;
        }
    }

    // Tell the caller that all is well
    return true;
}
//==========================================================================================================


//==========================================================================================================
// create_server() - Creates a socket for listening on a UDP port
//==========================================================================================================
bool UDPSock::create_server(int port, string bind_to, int family)
{
    // If the socket is open, close it
    close();

    // Fetch information about the local machine
    addrinfo info = NetUtil::get_local_addrinfo(SOCK_DGRAM, port, bind_to, family);

    // Create the socket
    m_sd = socket(info.ai_family, info.ai_socktype, info.ai_protocol);

    // If that failed, tell the caller
    if (m_sd < 0) return false;

    // Bind the socket to the specified port
    if (bind(m_sd, info.ai_addr, info.ai_addrlen) < 0) return false;

    // Tell the caller that all is well
    return true;
}
//==========================================================================================================



//==========================================================================================================
// send() - Call this to transmit data on a "Sender" socket
//==========================================================================================================
void UDPSock::send(const void* msg, int length)
{
    sendto(m_sd, msg, length, 0, m_target.ai_addr, m_target.ai_addrlen);
}
//==========================================================================================================



//==========================================================================================================
// receive() - Call this to wait for a packet on a server socket
//
// Returns: The number of bytes in the received message
//==========================================================================================================
int UDPSock::receive(void* buffer, int buf_size, string* p_source)
{
    sockaddr_storage peer;
    
    // Get a sockaddr* to the peer address
    sockaddr* p_peer = (sockaddr*)&peer;

    // We need this for the call to ::recvfrom
    socklen_t addrlen = sizeof(peer);

    // Wait for a UDP message to arrive, and stuff it into the caller's buffer
    int byte_count = recvfrom(m_sd, buffer, buf_size, 0, p_peer, &addrlen);

    // If there is room in the caller's buffer, as a convenience, put a nul-byte after the message
    if (byte_count < buf_size) ((char*)(buffer))[byte_count] = 0;

    // If the caller wants to know who sent the message...
    if (p_source) *p_source = NetUtil::ip_to_string(p_peer);

    // Return the length of the message that was just fetched
    return byte_count;
}
//==========================================================================================================



//==========================================================================================================
// wait_for_data() - Waits for the specified amount of time for data to be available for reading
//
// Passed: timeout_ms = timeout in milliseconds.  -1 = Wait forever
//
// Returns: true if data is available for reading, else false
//==========================================================================================================
bool UDPSock::wait_for_data(int timeout_ms)
{
    fd_set  rfds;
    timeval timeout;

    // Assume for the moment that we are going to wait forever
    timeval* pTimeout = NULL;

    // If the caller wants us to wait for a finite amount of time...
    if (timeout_ms != -1)
    {
        // Convert milliseconds to microseconds
        int usecs = timeout_ms * 1000;

        // Determine the timeout in seconds and microseconds
        timeout.tv_sec  = usecs / 1000000;
        timeout.tv_usec = usecs % 1000000;

        // Point to the timeout structure we just initialized
        pTimeout = &timeout;
    }

    // We'll wait on input from the file descriptor
    FD_ZERO(&rfds);
    FD_SET(m_sd, &rfds);

    // Wait for a character to be available for reading
    int status = select(m_sd+1, &rfds, NULL, NULL, pTimeout);

    // If status > 0, there is a character ready to be read
    return (status > 0);
}
//==========================================================================================================




//==========================================================================================================
// close() - closes the socket
//==========================================================================================================
void UDPSock::close()
{
    // If the socket is open, close it
    if (m_sd != -1) ::close(m_sd);

    // And mark the socket as closed
    m_sd = -1;
}
//==========================================================================================================

