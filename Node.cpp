#include "Node.h"
#include "Messenger.h"

int worker(int iNodeNum, int iNodesQuantity/*, Messenger* messenger*/, Node* thisNode, SYNCHRONIZATION_BARRIER* pBar, std::mutex* pMutStdout)
{
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY);
    //Sleep(5000);
    pMutStdout->lock();
    std::cout << "Thread hello " << iNodeNum << std::endl;
    pMutStdout->unlock();

    Message Msg;

    if (iNodeNum % 2 == 0)
    {
        char text[32] = "HELLO from thread ";
        _itoa(iNodeNum, &text[18], 10);
        Message msg(iNodeNum, ((iNodeNum == iNodesQuantity - 1) ? 0 : iNodeNum + 1), &text, sizeof(text));
        thisNode->messengerInstance->send(&msg);
    }
    else
    {
        thisNode->messengerInstance->recv(((iNodeNum == 0) ? (iNodesQuantity - 1) : (iNodeNum - 1)), &Msg);

        pMutStdout->lock();
        if (Msg.GetSize() != -1)
        {
            std::cout << "Thread: "        << iNodeNum        << std::endl
                      << "Recieved from: " << Msg.GetSender() << std::endl
                      << "Message: "       << Msg.GetData()   << std::endl << std::endl;
        }
        else
        {
            std::cout << "Thread: " << iNodeNum << " ERROR"<< std::endl;
        }
        pMutStdout->unlock();
    }

    return 0;
}

Node::Node(int iNodeNum, int iNodesNumber, std::mutex* pMutStdout, SYNCHRONIZATION_BARRIER* pBar)
{
    iNodeNumber = iNodeNum;
    _itoa(iNodeNumber, &cPipeName[20], 10);

    mutStdout = pMutStdout;

    pBarrier = pBar;

    pNodeThread = new std::thread(worker, iNodeNumber, iNodesNumber/*, messengerInstance*/, this, pBarrier, mutStdout);
    
    messengerInstance = new Messenger(cPipeName, iNodesNumber);
}


Node::~Node()
{
    //delete messengerInstance;
    //messengerInstance->~Messenger();
    //TODO close pipe handles (maybe in core.cpp)
}

void Node::start()
{
    pNodeThread->join();
}

void Node::GetPipeName(char* pipeName)
{
    if (pipeName != NULL)
    {
        memcpy(pipeName, cPipeName, sizeof(cPipeName));
    }
    else
    {
        exit(-3);
    }
}

//void Node::enterBarrier(LPSYNCHRONIZATION_BARRIER pBarr)
//{
//    EnterSynchronizationBarrier(
//        pBarr,
//        SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY
//    );
//}
//
//void Node::safePrint(const char* str)
//{
//    mutStdout->lock();
//    std::cout << str << std::endl;
//    mutStdout->unlock();
//}

