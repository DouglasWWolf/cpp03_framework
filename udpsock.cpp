//==========================================================================================================
// udpsock.cpp - Implements a class that manages UDP sockets
//==========================================================================================================
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "udpsock.h"
using namespace std;

//==========================================================================================================
// create_sender() - Create a socket useful for sending UDP packets
//==========================================================================================================
bool UDPSock::create_sender(int port, string dest_ip)
{
	int broadcast = 1;

	// If the socket is open, close it
	close();

	// Create the socket
    m_sd = socket(AF_INET, SOCK_DGRAM, 0);

    // If that failed, tell the caller
    if (m_sd < 0) return false;

	// Make sure the destination IP address is valid
	if (dest_ip.empty()) dest_ip = "255.255.255.255";

    // If we're broadcasting, set up the socket for broadcast
    if (dest_ip == "255.255.255.255" || dest_ip == "broadcast")
    {
        if (setsockopt(m_sd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1)
        {
            return false;
        }
    }

    // Set up destination address
    memset(&m_to_addr,0,sizeof(m_to_addr));
    m_to_addr.sin_family = AF_INET;
    m_to_addr.sin_addr.s_addr = inet_addr(dest_ip.c_str());
    m_to_addr.sin_port=htons(port);

    // Tell the caller that all is well
    return true;
}
//==========================================================================================================


//==========================================================================================================
// create_server() - Creates a socket for listening on a UDP port
//==========================================================================================================
bool UDPSock::create_server(int port)
{
	sockaddr_in addr;

	// If the socket is open, close it
    close();
    
	// Create the socket
    m_sd = socket(AF_INET, SOCK_DGRAM, 0);

    // If that failed, tell the caller
    if (m_sd < 0) return false;

    // Bind the socket to the specified port
    addr.sin_family = AF_INET;
    addr.sin_port = htons (port);
    addr.sin_addr.s_addr = htonl (INADDR_ANY);

    // Bind the socket to the specified port
    if (bind(m_sd, (sockaddr *) &addr, sizeof (addr)) < 0) return false;

    // Tell the caller that all is well
    return true;
}
//==========================================================================================================



//==========================================================================================================
// send() - Call this to transmit data on a "Sender" socket
//==========================================================================================================
void UDPSock::send(const void* msg, int length)
{
	sendto(m_sd, msg, length, 0, (sockaddr *) &m_to_addr, sizeof(m_to_addr));
}
//==========================================================================================================



//==========================================================================================================
// receive() - Call this to wait for a packet on a server socket
//
// Returns: The number of bytes in the received message
//==========================================================================================================
int UDPSock::receive(void* buffer, int buf_size, string* p_source)
{
    char peer_ip[INET6_ADDRSTRLEN];

    // We need this for the call to ::recvfrom
	socklen_t addrlen = sizeof(m_from_addr);

    // Wait for a UDP message to arrive, and stuff it into the caller's buffer
	int byte_count = recvfrom(m_sd, buffer, buf_size, 0,(sockaddr*)&m_from_addr, &addrlen);

    // If there is room in the caller's buffer, as a convenience, put a nul-byte after the message
    if (byte_count < buf_size) ((char*)(buffer))[byte_count] = 0;

    // If the caller wants to know who sent the message...
    if (p_source)
    {
        // Convert the peer address to a human-readable IP address
        inet_ntop(AF_INET, &(m_from_addr.sin_addr), peer_ip, sizeof(peer_ip));

        // And stuff the address into the caller's field
        *p_source = peer_ip;
    }

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

