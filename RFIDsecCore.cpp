#include "RFIDsecCore.h"
#include "Messenger.h"



RFIDsecCore::RFIDsecCore(int iNumberOfNodes)
{
    if (!InitializeSynchronizationBarrier(
        &barrier_0,
        iNumberOfNodes + 1, // +reader
        0
    ))
    {
        exit(-1);
    }


    for (int i = 0; i < iNumberOfNodes; i++)
    {
        Node tmp(i, iNumberOfNodes, &mutStdout, &barrier_0);
        vNodes.push_back(tmp);
    }
    reader = new Reader(iNumberOfNodes, &mutStdout, &barrier_0);

    //for (int i = 0; i < iNumberOfNodes; i++)
    //{
    //    vNodes[i].start();
    //}
    reader->start();
}

RFIDsecCore::~RFIDsecCore()
{
}

