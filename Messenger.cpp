#include "Messenger.h"

#include <iostream>
///////////// Prototypes ///////////////////////////////////
                                                          //
//                                                        //
// Main listener's routine                                //
//                                                        //
void listenerMainWorker(                                  //
    char*                            cPipeName,           //
    std::vector<std::list<Message>>* vectMessages,        //
    std::deque<std::mutex>*          qMutexes,            //
    std::list<Message>*              listReaderMessages,  //
    std::mutex*                      mutReaderMessages    //
);                                                        //
                                                          //
//                                                        //
// Listener for one client                                //
//                                                        //
void subListenerWorker(                                   //
    HANDLE                          hListenerPipe,        //
    std::vector<std::list<Message>>* vectMessages,        //
    std::deque<std::mutex>*          qMutexes,            //
    std::list<Message>*              listReaderMessages,  //
    std::mutex*                      mutReaderMessages    //
);                                                        //
                                                          //
////////////////////////////////////////////////////////////

void listenerMainWorker(
    char*                            cPipeName,
    std::vector<std::list<Message>>* vectMessages,
    std::deque<std::mutex>*          qMutexes,
    std::list<Message>*              listReaderMessages,
    std::mutex*                      mutReaderMessages
)
{
    BOOL   fConnected = FALSE;
    HANDLE hPipe1;
    while (true)
    {
        hPipe1 = CreateNamedPipe(
            cPipeName,                // pipe name 
            PIPE_ACCESS_DUPLEX,       // read/write access 
            PIPE_TYPE_MESSAGE |       // message type pipe 
            PIPE_READMODE_MESSAGE |   // message-read mode 
            PIPE_WAIT,                // blocking mode 
            PIPE_UNLIMITED_INSTANCES, // max. instances  
            PIPE_BUFFER_SIZE,         // output buffer size 
            PIPE_BUFFER_SIZE,         // input buffer size 
            0,                        // client time-out 
            NULL                      // default security attribute 
        );
        if (hPipe1 == INVALID_HANDLE_VALUE)
        {
            printf("CreateNamedPipe failed, GLE=%d.\n", GetLastError());
        }
        // Wait for the client to connect; if it succeeds, the function returns a nonzero value.
        // If the function returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
        //fConnected = ConnectNamedPipe(*hListenerPipe, NULL) ?
        fConnected = ConnectNamedPipe(hPipe1, NULL) ?
            TRUE :
            (GetLastError() == ERROR_PIPE_CONNECTED);

        if (fConnected)
        {
            // Create a thread for this client
            //std::thread* tmp = new std::thread(subListenerWorker, hListenerPipe, vectMessages, qMutexes, listReaderMessages, mutReaderMessages);
            std::thread* tmp = new std::thread(subListenerWorker, hPipe1, vectMessages, qMutexes, listReaderMessages, mutReaderMessages);
            //tmp->detach();
        }
        else
        {
            // The client could not connect, so close the pipe. 
            CloseHandle(hPipe1);
        }
    }
}

void subListenerWorker(
    HANDLE                           hListenerPipe,
    std::vector<std::list<Message>>* vectMessages,
    std::deque<std::mutex>*          qMutexes,
    std::list<Message>*              listReaderMessages,
    std::mutex*                      mutReaderMessages
)
{
    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    char cRequest[PIPE_BUFFER_SIZE];
    BOOL fSuccess = FALSE;

    // Loop until done reading
    do {
        // Read client requests from the pipe. This simplistic code only allows messages
        // up to BUFSIZE characters in length.
        fSuccess = ReadFile(
            hListenerPipe,   // handle to pipe 
            cRequest,         // buffer to receive data 
            PIPE_BUFFER_SIZE, // size of buffer 
            &cbBytesRead,     // number of bytes read 
            NULL              // not overlapped I/O
        );
    } while (!fSuccess); // repeat loop if ERROR_MORE_DATA

    Message msg((void*)cRequest);
    
    if (msg.GetID() == -100)
    {
        return;
    }

    // Choice of appropriate list of messages and mutex
    std::mutex*         pMut      = ((msg.GetSender() == -1) ? mutReaderMessages  : &(*qMutexes)[msg.GetSender()]    );
    std::list<Message>* pMessages = ((msg.GetSender() == -1) ? listReaderMessages : &(*vectMessages)[msg.GetSender()]);

    // Write the message to list
    pMut->lock();
    pMessages->push_back(msg);
    pMut->unlock();

    CloseHandle(hListenerPipe);
}


Messenger::Messenger(
    char* cPipeName, 
    int   iNodesNum
)
{
    memcpy(cListenerPipeName, cPipeName, strlen(cPipeName));

    iNodesQuantity = iNodesNum;
    for (int i = 0; i < iNodesQuantity; i++)
    {
        std::list<Message> tmp;
        vectRecievedMessages.push_back(tmp);
    }
#ifdef ADVERSARY
    std::list<Message> tmp;
    vectRecievedMessages.push_back(tmp);
#endif

    qMutexes = new std::deque<std::mutex>(
#ifndef ADVERSARY
        iNodesQuantity
#else
        iNodesQuantity + 1
#endif
        );

    //pListenerThread = new std::thread(listenerMainWorker, &hPipe, &vectRecievedMessages, qMutexes, &listReaderMessages, &mutReaderMessages);
    pListenerThread = new std::thread(listenerMainWorker, cListenerPipeName, &vectRecievedMessages, qMutexes, &listReaderMessages, &mutReaderMessages);
    //pListenerThread->detach();
}

Messenger::~Messenger()
{
}

int Messenger::send(
    Message* msg
)
{

    HANDLE hPipeS;
    char   chBuf[PIPE_BUFFER_SIZE] = { '\0' };
    char   cPipeName[64]           = { '\0' };
    BOOL   fSuccess                = FALSE;
    DWORD  cbToWrite, cbWritten, dwMode;
    
    
    switch (msg->GetReciever())
    {
    case ADDR_READER: // To reader
    {
        memcpy(cPipeName, "\\\\.\\pipe\\RFIDsecPipeReader", 26);
        break;
    }
#ifdef ADVERSARY
    case ADDR_ADVERSARY: // To adversary
    {
        memcpy(cPipeName, "\\\\.\\pipe\\RFIDsecPipeAdversary", 29);
        break;
    }
#endif
    default:
    {
        memcpy(cPipeName, "\\\\.\\pipe\\RFIDsecPipe", 20); // Pipe name in format: \\.\pipe\RFIDsecPipe< Number of this node >
        _itoa(msg->GetReciever(), &cPipeName[20], 10);
        break;
    }
    }
        
    // Try to open a named pipe; wait for it, if necessary. 
    while (1)
    {
        hPipeS = CreateFile(
            cPipeName,      // pipe name 
            GENERIC_READ |  // read and write access 
            GENERIC_WRITE,
            0,              // no sharing 
            NULL,           // default security attributes
            OPEN_EXISTING,  // opens existing pipe 
            0,              // default attributes 
            NULL);          // no template file 
    
        // Break if the pipe handle is valid. 
        if (hPipeS != INVALID_HANDLE_VALUE)
        {
            //printf("ok");
            break;
        }
    
        // Exit if an error other than ERROR_PIPE_BUSY occurs.
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            printf("Could not open pipe \"%s\". GLE=%d\n", cPipeName, GetLastError());
            //continue;

            exit(-1);

            return -1;
        }
    
        // All pipe instances are busy, so wait. 
        if (!WaitNamedPipe(cPipeName, NMPWAIT_WAIT_FOREVER))
        {
            printf("Could not open pipe: 2 second wait timed out.\n");


            exit(-1);

            return -1;
        }
    }
    
    // The pipe connected; change to message-read mode. 
    dwMode = PIPE_READMODE_MESSAGE;
    fSuccess = SetNamedPipeHandleState(
        hPipeS,   // pipe handle 
        &dwMode,  // new pipe mode 
        NULL,     // don't set maximum bytes 
        NULL      // don't set maximum time
    );
    if (!fSuccess)
    {
        printf("SetNamedPipeHandleState failed. GLE=%d\n", GetLastError());
        return -1;
    }
    
    // Send a message
    cbToWrite = sizeof(Message::stMessage);
    fSuccess = WriteFile(
        hPipeS,                 // pipe handle 
        msg->GetRawMessage(),   // message 
        cbToWrite,              // message length 
        &cbWritten,             // bytes written 
        NULL                    // not overlapped
    );
    
    if (!fSuccess)
    {
        printf("WriteFile to pipe failed. GLE=%d\n", GetLastError());
        return -1;
    }
    
    //printf("\nMessage sent to %s\n", cPipeName);
    

    //CloseHandle(hPipeS);
    return 0;
}

int Messenger::recv(
    int      iFrom, 
    Message* msg
)
{
    bool bEmpty = true;
    
    std::mutex*         pMut      = ((iFrom == -1) ? &mutReaderMessages  : &(*qMutexes)[iFrom]          ); // Mutex to use
    std::list<Message>* pMessages = ((iFrom == -1) ? &listReaderMessages : &vectRecievedMessages[iFrom] ); // List of messages to use


    for (int i = 0; i < RECV_MAX_WAIT; i++)
    {
        pMut->lock();
        bEmpty = pMessages->empty();
        pMut->unlock();

        if (!bEmpty)
        {
            pMut->lock();
            *msg = pMessages->front();
            pMessages->pop_front();

#ifdef ADVERSARY
            uint8_t tmp[NONCE_LEN * 2];
            memcpy(tmp, msg->GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
            std::cout << msg->GetID() << " " << std::bitset<8>(tmp[0]) << " " << std::bitset<8>(tmp[1]) << std::endl;
#endif // ADVERSARY

            //printf("%i\n", msg->GetID());
            pMut->unlock();

            return 0; // Message was recieved
        }
        else
        {
            Sleep(RECV_SLEEP_TIME);
        }
    }
    //printf("F");
    return -1; // Waiting time is over. Message was not recieved
}
