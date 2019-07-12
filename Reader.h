#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include "Messenger.h"

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

    void start();

    Messenger* messengerInstance;

private:

    std::thread* pReaderThread;                                    // Pointer to the reader's thread

    char         cPipeName[64] = "\\\\.\\pipe\\RFIDsecPipeReader"; // Pipe name
    HANDLE       hPipe;                                            // Pipe handle

    std::mutex*  mutStdout;                                        // Pointer to mutex for access to console output 
    LPSYNCHRONIZATION_BARRIER pBarrier;

};

