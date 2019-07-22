#pragma once
#include <Windows.h>
#include <vector>
#include "Node.h"
#include "Reader.h"
#include "defines.h"

#ifdef ADVERSARY
#include "Adversary.h"
#endif

class RFIDsecCore
{
public:

    SYNCHRONIZATION_BARRIER barrier_0;
    std::mutex mutStdout;
    
    RFIDsecCore();

    ~RFIDsecCore();

private:
    
    std::vector<Node> vNodes; // Vector of all nodes
    Reader*           reader; // reader

#ifdef ADVERSARY
    Adversary*        adversary;
#endif
};

