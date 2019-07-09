#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include "Messenger.h"
#include "defines.h"


class Node
{
public:

    //
    // nodeArgv[0] - node's number as C-string
    // nodeArgv[1] - stdout mutex pointer
    // nodeArgv[2] - barrier's address
    //
    Node(
        int iNodeNum,
        int iNudesNumber,
        std::mutex* pMutStdout,
        SYNCHRONIZATION_BARRIER* pBar
    );

    ~Node();

    //
    //
    //
    //int init(int nodeArgc, char** nodeArgv[]);

    // 
    // Node's main function
    // In the beginning enterBarrier() should be called
    // 
    //int worker(int arg);

    //
    // pNodeThread->join();
    //
    void start();

    //
    // Writes name of this node's pipe into pre-allocated input buffer
    //
    void GetPipeName(char *pipeName);

    Messenger* messengerInstance;
    
private:

    int          iNodeNumber; // Number of this node

    std::thread* pNodeThread; // This node's thread

    char         cPipeName[64] = "\\\\.\\pipe\\RFIDsecPipe"; // Pipe name in format: \\.\pipe\RFIDsecPipe< Number of this node >
    //HANDLE       hPipeHandle;

    std::mutex*  mutStdout;   // Pointer to mutex for access to console output 
    LPSYNCHRONIZATION_BARRIER pBarrier;


    //std::vector< std::mutex > vectMutexes;

    // 
    // Thread enters synchroization barrier
    // 
    ///void enterBarrier(LPSYNCHRONIZATION_BARRIER pBarr);

    // 
    // Synchronized output
    // 
    ///void safePrint(const char *str);
    
};

