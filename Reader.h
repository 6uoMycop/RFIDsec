#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include "Messenger.h"
#include <bitset>
#include "defines.h"

//
// Class which describes RFID-reader
//
class Reader
{
public:

    Reader(
        int                      iNodesNumber,
        std::mutex*              pMutStdout,
        SYNCHRONIZATION_BARRIER* pBar
    );

    ~Reader();

    //
    // thread->join();
    //
    void start();

    Messenger* messengerInstance; // Messenger to perform communication

private:

    std::thread*              pReaderThread;                                    // Pointer to the reader's thread
    char                      cPipeName[64] = "\\\\.\\pipe\\RFIDsecPipeReader"; // Pipe name
    std::mutex*               mutStdout;                                        // Pointer to mutex for access to console output 
    LPSYNCHRONIZATION_BARRIER pBarrier;

};

