#pragma once
#include <Windows.h>
#include <vector>
#include "Node.h"
#include "Reader.h"

class RFIDsecCore
{
public:

    SYNCHRONIZATION_BARRIER barrier_0;
    std::mutex mutStdout;
    
    RFIDsecCore(
        int  iNumberOfNodes
    );

    ~RFIDsecCore();

private:
    
    std::vector<Node> vNodes; // Vector of all nodes

    Reader* reader;           // reader

};

