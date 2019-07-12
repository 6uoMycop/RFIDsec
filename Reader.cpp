#include "Reader.h"

int worker(
    int                      iNodesNumber,
    Messenger*               messenger,
    std::mutex*              pMutStdout,
    SYNCHRONIZATION_BARRIER* pBar
)
{
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY);

    char text[128] = "Hello, i\'m reader. ";

    for (int i = iNodesNumber - 1; i >= 0; i--)
    {
        _itoa(i, &text[19], 10);
        Message msg(-1, i, text, 128);
        messenger->send(&msg);
    }
    return 0;
}

Reader::Reader(int iNodesNumber, std::mutex* pMutStdout, SYNCHRONIZATION_BARRIER* pBar)
{
    mutStdout = pMutStdout;

    messengerInstance = new Messenger(cPipeName, iNodesNumber);

    pReaderThread = new std::thread(worker, iNodesNumber, messengerInstance, mutStdout, pBar);
}

Reader::~Reader()
{
}

void Reader::start()
{
    pReaderThread->join();
}