#include "Node.h"
#include "Messenger.h"

int worker(
    int                      iNodeNum, 
    int                      iNodesQuantity, 
    Messenger*               messenger, 
    SYNCHRONIZATION_BARRIER* pBar, 
    std::mutex*              pMutStdout
)
{
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY);
    //Sleep(5000);
    //pMutStdout->lock();
    //std::cout << "Thread hello " << iNodeNum << std::endl;
    //pMutStdout->unlock();

    Message Msg;

    messenger->recv(-1, &Msg);

    pMutStdout->lock();
    if (Msg.GetSize() != -2)
    {
        std::cout << "Thread: "        << iNodeNum        << std::endl
                  << "Recieved from: " << Msg.GetSender() << std::endl
                  << "Message: "       << Msg.GetData()   << std::endl << std::endl;
    }
    else
    {
        std::cout << "Thread "         << iNodeNum        << ": ERROR" << std::endl;
    }
    pMutStdout->unlock();

    //if (iNodeNum % 2 == 0)
    //{
    //    char text[32] = "HELLO from thread ";
    //    _itoa(iNodeNum, &text[18], 10);
    //    Message msg(iNodeNum, ((iNodeNum == iNodesQuantity - 1) ? 0 : iNodeNum + 1), &text, sizeof(text));
    //    thisNode->messengerInstance->send(&msg);
    //
    //    ///
    //
    //    thisNode->messengerInstance->recv(((iNodeNum == 0) ? (iNodesQuantity - 1) : (iNodeNum - 1)), &Msg);
    //
    //    pMutStdout->lock();
    //    if (Msg.GetSize() != -1)
    //    {
    //        std::cout << "Thread: " << iNodeNum << std::endl
    //            << "Recieved from: " << Msg.GetSender() << std::endl
    //            << "Message: " << Msg.GetData() << std::endl << std::endl;
    //    }
    //    else
    //    {
    //        std::cout << "Thread: " << iNodeNum << " ERROR" << std::endl;
    //    }
    //    pMutStdout->unlock();
    //}
    //else
    //{
    //    thisNode->messengerInstance->recv(((iNodeNum == 0) ? (iNodesQuantity - 1) : (iNodeNum - 1)), &Msg);
    //
    //    pMutStdout->lock();
    //    if (Msg.GetSize() != -2)
    //    {
    //        std::cout << "Thread: "        << iNodeNum        << std::endl
    //                  << "Recieved from: " << Msg.GetSender() << std::endl
    //                  << "Message: "       << Msg.GetData()   << std::endl << std::endl;
    //    }
    //    else
    //    {
    //        std::cout << "Thread: " << iNodeNum << " ERROR"<< std::endl;
    //    }
    //    pMutStdout->unlock();
    //
    //    ///
    //
    //    char text[32] = "HELLO from thread ";
    //    _itoa(iNodeNum, &text[18], 10);
    //    Message msg(iNodeNum, ((iNodeNum == iNodesQuantity - 1) ? 0 : iNodeNum + 1), &text, sizeof(text));
    //    thisNode->messengerInstance->send(&msg);
    //}

    return 0;
}

Node::Node(int iNodeNum, int iNodesNumber, std::mutex* pMutStdout, SYNCHRONIZATION_BARRIER* pBar)
{
    iNodeNumber = iNodeNum;
    _itoa(iNodeNumber, &cPipeName[20], 10);

    mutStdout = pMutStdout;

    pBarrier = pBar;

    messengerInstance = new Messenger(cPipeName, iNodesNumber);
    
    pNodeThread = new std::thread(worker, iNodeNumber, iNodesNumber, messengerInstance, pBarrier, mutStdout);
}


Node::~Node()
{
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
