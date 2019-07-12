#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <thread>
#include <list>
#include <vector>
#include <deque>
#include <mutex>
#include "Message.h"

#define PIPE_BUFFER_SIZE 1024 //

#define RECV_MAX_WAIT    10   // Number of waiting iterations
#define RECV_SLEEP_TIME  100  // ms to sleep during one iteration

// 
// Class for messages exchange    
// 
class Messenger
{
public:

    Messenger(
        const char* cPipeName, // Name of listener pipe
        int         iNodesNum  // Number of nodes
    );
    ~Messenger();

    //
    // Start recieving messages from certain pipe. Adds them to a queue
    //
    //void startListening(
    //    const char*                cPipeName,  // Name of listener pipe
    //    int                        iNodesNum   // Number of nodes
    //    //std::vector< std::mutex >* vectMutexes // Pointer to mutexes which all threads use
    //);

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

private:


    std::thread* pListenerThread; // Listener's thread

    // For inter-nodes communication
    std::vector< std::list<Message> > vectRecievedMessages; // Here all recieved messages will be stored until they are processed. A vector's component stands for sender node
    std::deque< std::mutex >* qMutexes;

    // For communication with reader
    std::list<Message> listReaderMessages; 
    std::mutex mutReaderMessages;

    int iNodesNumber; // Number of nodes


    HANDLE hPipe; // Handle of listener pipe

    
};

