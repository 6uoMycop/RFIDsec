#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <thread>
#include <list>
#include <vector>
#include <deque>
#include <mutex>
#include "Message.h"
#include "defines.h"

#include <bitset>

#define PIPE_BUFFER_SIZE 4096 //

#define RECV_MAX_WAIT    10   // Number of waiting iterations
#define RECV_SLEEP_TIME  100  // ms to sleep during one iteration

// 
// Class for messages exchange    
// 
class Messenger
{
public:
    
    Messenger(
        char*    cPipeName, // Name of listener pipe
        int      iNodesNum  // Number of nodes
    );

    ~Messenger();

    //
    // Send pre-constructed message
    //
    int send(
        Message* msg // Pointer to a message to send
    );

    // 
    // Recieve first message in queue
    // 
    int recv(
        int      iFrom, // Sender node's number
        Message* msg    // Message will be stored here
    );
    
    char cListenerPipeName[64];

private:

    std::thread* pListenerThread; // Listener's thread
    int          iNodesQuantity;  // Number of nodes

    // For inter-nodes communication
    std::vector< std::list<Message> > vectRecievedMessages; // Here all recieved messages will be stored until they are processed. A vector's component stands for sender node
    std::deque< std::mutex >*         qMutexes;

    // For communication with reader
    std::list<Message> listReaderMessages; 
    std::mutex         mutReaderMessages;
};

