#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include "Messenger.h"
#include "defines.h"

//
// Class which describes RFID-tag
//
class Node
{
public:

    Node(
        int iNodeNum,
        int iNodesNumber,
        std::mutex* pMutStdout,
        SYNCHRONIZATION_BARRIER* pBar
    );

    ~Node();

    //
    // thread->join();
    //
    void start();

    //
    // Writes name of this node's pipe into pre-allocated input buffer
    //
    void GetPipeName(char *pipeName);

    Messenger* messengerInstance; // Messenger to perform communication
    
private:

    int                       iNodeNumber;                                // Number of this node
    std::thread*              pNodeThread;                                // This node's thread
    char                      cPipeName[64] = "\\\\.\\pipe\\RFIDsecPipe"; // Pipe name in format: \\.\pipe\RFIDsecPipe< Number of this node >
    std::mutex*               mutStdout;                                  // Pointer to mutex for access to console output
    LPSYNCHRONIZATION_BARRIER pBarrier;
};

