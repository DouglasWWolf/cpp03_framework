//==========================================================================================================
// CANSock.h - Defines a class for sending and receiving data over a CAN interface
//==========================================================================================================
#pragma once
#include <sys/socket.h>
#include <linux/can.h>
#include <string>

class CANSock
{
public:
    
    // Default constuctor
    CANSock() {m_sd = -1;}

    // Default destructor closes the interface connection
    ~CANSock() {close();}

    // Call this to connect to a CAN interface
    bool    connect(std::string interface);

    // Closes the connection to the CAN interface
    void    close();

    // Call this to place a message into the CAN bus
    void    put(int msg_id, const void* buffer, size_t buf_size);

    // Call this to fetch the next message from the CAN bus.  Timeout of -1 means "wait forever"
    bool    get(can_frame* p_frame, int timeout_ms = -1);

protected:

    // The socket descriptor
    int     m_sd;
};




