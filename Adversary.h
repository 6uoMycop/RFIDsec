#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include "Messenger.h"
#include <bitset>
#include "defines.h"
#include "Crypt.h"

class Adversary
{
public:

    Adversary(
        int                      iNodesNumber,
        std::mutex*              pMutStdout,
        SYNCHRONIZATION_BARRIER* pBar
    );

    ~Adversary();
    
    Messenger* messengerInstance; // Messenger to perform communication

    void start();

private:

    std::thread*              pAdversaryThread;                                    // Pointer to the reader's thread
    char                      cPipeName[64] = "\\\\.\\pipe\\RFIDsecPipeAdversary"; // Pipe name
    std::mutex*               mutStdout;                                           // Pointer to mutex for access to console output 
    LPSYNCHRONIZATION_BARRIER pBarrier;

};

