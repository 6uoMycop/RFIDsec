#include "RFIDsecCore.h"
#include "Messenger.h"



RFIDsecCore::RFIDsecCore()
{
    if (!InitializeSynchronizationBarrier(
        &barrier_0,
#ifndef ADVERSARY
        NODES_QUANTITY + 1, // +reader
#else
        NODES_QUANTITY + 1 + 1, // +reader +adversary
#endif
        0
    ))
    {
        exit(-1);
    }


    time_t start = clock();

    for (int i = 0; i < NODES_QUANTITY; i++)
    {
        Node tmp(i, NODES_QUANTITY, &mutStdout, &barrier_0);
        vNodes.push_back(tmp);
    }
    reader = new Reader(NODES_QUANTITY, &mutStdout, &barrier_0);

#ifdef ADVERSARY
    adversary = new Adversary(NODES_QUANTITY, &mutStdout, &barrier_0);
    adversary->start();
#endif

    //for (int i = 0; i < iNumberOfNodes; i++)
    //{
    //    vNodes[i].start();
    //}
    reader->start();

    time_t end = clock();

    std::cout << end - start << std::endl;

    FILE* fileRez = NULL;
    fopen_s(&fileRez, "rez.txt", "a");
    fprintf(fileRez, "%lli\n", end - start);


}

RFIDsecCore::~RFIDsecCore()
{
}

