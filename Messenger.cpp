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
/*
#define PIPE_TIMEOUT 5000

struct stStoragePtrs
{
    std::vector<std::list<Message>>* vectMessages;
    std::deque<std::mutex>* qMutexes;
    std::list<Message>* listReaderMessages;
    std::mutex* mutReaderMessages;
};

typedef struct
{
    OVERLAPPED oOverlap;
    HANDLE hPipeInst;
    TCHAR chRequest[PIPE_BUFFER_SIZE];
    DWORD cbRead;
    TCHAR chReply[PIPE_BUFFER_SIZE];
    DWORD cbToWrite;
    int iNodeNumber;
    stStoragePtrs storage;
} PIPEINST, * LPPIPEINST;


VOID DisconnectAndClose(LPPIPEINST);
BOOL CreateAndConnectInstance(LPOVERLAPPED, char* cPipeName);
BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);
VOID StoreMessage(LPPIPEINST);

VOID WINAPI CompletedWriteRoutine(DWORD, DWORD, LPOVERLAPPED);
VOID WINAPI CompletedReadRoutine(DWORD, DWORD, LPOVERLAPPED);

HANDLE hPipe;

//char glPipeName[64] = { 0 };

void listenerMainWorker(
    char*                            cPipeName,
    std::vector<std::list<Message>>* vectMessages,
    std::deque<std::mutex>*          qMutexes,
    std::list<Message>*              listReaderMessages,
    std::mutex*                      mutReaderMessages
)
{
    HANDLE hConnectEvent;
    OVERLAPPED oConnect;
    LPPIPEINST lpPipeInst;
    DWORD dwWait, cbRet;
    BOOL fSuccess, fPendingIO;
    stStoragePtrs storage;


    // Create one event object for the connect operation. 

    hConnectEvent = CreateEvent(
        NULL,    // default security attribute
        TRUE,    // manual reset event 
        TRUE,    // initial state = signaled 
        NULL);   // unnamed event object 

    if (hConnectEvent == NULL)
    {
        printf("CreateEvent failed with %d.\n", GetLastError());
        return;
    }

    oConnect.hEvent = hConnectEvent;




    // Call a subroutine to create one instance, and wait for 
    // the client to connect. 
    fPendingIO = CreateAndConnectInstance(&oConnect, cPipeName);

    while (1)
    {
        // Wait for a client to connect, or for a read or write 
        // operation to be completed, which causes a completion 
        // routine to be queued for execution. 

        dwWait = WaitForSingleObjectEx(
            hConnectEvent,  // event object to wait for 
            INFINITE,       // waits indefinitely 
            TRUE);          // alertable wait enabled 

        switch (dwWait)
        {
            // The wait conditions are satisfied by a completed connect 
            // operation. 
        case 0:
            // If an operation is pending, get the result of the 
            // connect operation. 
        {
            if (fPendingIO)
            {
                fSuccess = GetOverlappedResult(
                    hPipe,     // pipe handle 
                    &oConnect, // OVERLAPPED structure 
                    &cbRet,    // bytes transferred 
                    FALSE);    // does not wait 
                if (!fSuccess)
                {
                    if (GetLastError() == ERROR_PIPE_NOT_CONNECTED)
                    {
                        continue;
                    }
                    printf("ConnectNamedPipe (%d)\n", GetLastError());
                    return;
                }
            }

            // Allocate storage for this instance. 

            lpPipeInst = (LPPIPEINST)GlobalAlloc(GPTR, sizeof(PIPEINST));
            if (lpPipeInst == NULL)
            {
                printf("GlobalAlloc failed (%d)\n", GetLastError());
                return;
            }

            lpPipeInst->hPipeInst = hPipe;


            //Node number
            lpPipeInst->iNodeNumber = (cPipeName[20] == 'R') ? -1 : atoi(&cPipeName[20]);

            lpPipeInst->storage.listReaderMessages = listReaderMessages;
            lpPipeInst->storage.mutReaderMessages = mutReaderMessages;
            lpPipeInst->storage.qMutexes = qMutexes;
            lpPipeInst->storage.vectMessages = vectMessages;


            // Start the read operation for this client. 
            // Note that this same routine is later used as a 
            // completion routine after a write operation. 

            lpPipeInst->cbToWrite = 0;
            CompletedWriteRoutine(0, 0, (LPOVERLAPPED)lpPipeInst);

            // Create new pipe instance for the next client. 

            fPendingIO = CreateAndConnectInstance(&oConnect, cPipeName);
            break;
        }

        // The wait is satisfied by a completed read or write 
        // operation. This allows the system to execute the 
        // completion routine. 
        case WAIT_IO_COMPLETION:
        {
            break;
        }

        // An error occurred in the wait function.
        default:
        {
            printf("WaitForSingleObjectEx (%d)\n", GetLastError());
            return;
        }
        }
    }
}

// CompletedWriteRoutine(DWORD, DWORD, LPOVERLAPPED) 
// This routine is called as a completion routine after writing to 
// the pipe, or when a new client has connected to a pipe instance.
// It starts another read operation. 

VOID WINAPI CompletedWriteRoutine(DWORD dwErr, DWORD cbWritten, LPOVERLAPPED lpOverLap)
{
    LPPIPEINST lpPipeInst;
    BOOL fRead = FALSE;

    // lpOverlap points to storage for this instance. 

    lpPipeInst = (LPPIPEINST)lpOverLap;

    // The write operation has finished, so read the next request (if 
    // there is no error). 

    if ((dwErr == 0) && (cbWritten == lpPipeInst->cbToWrite))
    {
        fRead = ReadFileEx(
            lpPipeInst->hPipeInst,
            lpPipeInst->chRequest,
            PIPE_BUFFER_SIZE * sizeof(TCHAR),
            (LPOVERLAPPED)lpPipeInst,
            (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedReadRoutine);
    }

    // Disconnect if an error occurred. 

    if (!fRead)
    {
        //DisconnectAndClose(lpPipeInst);
    }
}

// CompletedReadRoutine(DWORD, DWORD, LPOVERLAPPED) 
// This routine is called as an I/O completion routine after reading 
// a request from the client. It gets data and writes it to the pipe. 

VOID WINAPI CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap)
{
    LPPIPEINST lpPipeInst;
    BOOL fWrite = FALSE;

    // lpOverlap points to storage for this instance. 

    lpPipeInst = (LPPIPEINST)lpOverLap;

    // The read operation has finished, so write a response (if no 
    // error occurred). 

    if ((dwErr == 0) && (cbBytesRead != 0))
    {
        StoreMessage(lpPipeInst);
        
        fWrite = WriteFileEx(
            lpPipeInst->hPipeInst,
            lpPipeInst->chReply,
            lpPipeInst->cbToWrite,
            (LPOVERLAPPED)lpPipeInst,
            (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedWriteRoutine);
    }

    // Disconnect if an error occurred. 

    if (!fWrite)
        DisconnectAndClose(lpPipeInst);
}

// DisconnectAndClose(LPPIPEINST) 
// This routine is called when an error occurs or the client closes 
// its handle to the pipe. 

VOID DisconnectAndClose(LPPIPEINST lpPipeInst)
{
    // Disconnect the pipe instance. 

    if (!DisconnectNamedPipe(lpPipeInst->hPipeInst))
    {
        printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
    }

    // Close the handle to the pipe instance. 

    CloseHandle(lpPipeInst->hPipeInst);

    // Release the storage for the pipe instance. 

    if (lpPipeInst != NULL)
        GlobalFree(lpPipeInst);
}

// CreateAndConnectInstance(LPOVERLAPPED) 
// This function creates a pipe instance and connects to the client. 
// It returns TRUE if the connect operation is pending, and FALSE if 
// the connection has been completed. 
BOOL CreateAndConnectInstance(LPOVERLAPPED lpoOverlap, char* cPipeName)
{
    hPipe = CreateNamedPipe(
        cPipeName,                        // pipe name 
        PIPE_ACCESS_DUPLEX |              // read/write access 
        FILE_FLAG_OVERLAPPED,             // overlapped mode 
        PIPE_TYPE_MESSAGE |               // message-type pipe 
        PIPE_READMODE_MESSAGE |           // message read mode 
        PIPE_WAIT,                        // blocking mode 
        PIPE_UNLIMITED_INSTANCES,         // unlimited instances 
        PIPE_BUFFER_SIZE * sizeof(TCHAR), // output buffer size 
        PIPE_BUFFER_SIZE * sizeof(TCHAR), // input buffer size 
        PIPE_TIMEOUT,                     // client time-out 
        NULL);                            // default security attributes
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        printf("CreateNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }

    // Call a subroutine to connect to the new client. 

    return ConnectToNewClient(hPipe, lpoOverlap);
}

BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
    BOOL fConnected, fPendingIO = FALSE;

    // Start an overlapped connection for this pipe instance. 
    fConnected = ConnectNamedPipe(hPipe, lpo);

    // Overlapped ConnectNamedPipe should return zero. 
    if (fConnected)
    {
        printf("ConnectNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }

    switch (GetLastError())
    {
        // The overlapped connection in progress. 
    case ERROR_IO_PENDING:
        fPendingIO = TRUE;
        break;

        // Client is already connected, so signal an event. 

    case ERROR_PIPE_CONNECTED:
        if (SetEvent(lpo->hEvent))
            break;

        // If an error occurs during the connect operation... 
    default:
    {
        printf("ConnectNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }
    }
    return fPendingIO;
}

VOID StoreMessage(
    LPPIPEINST pipe
)
{    
    Message msg((void*)pipe->chRequest);
    
    if (msg.GetID() == -100) // Hello
    {
        printf("H");
        return;
    }

    // Choice of appropriate list of messages and mutex
    std::mutex* pMut =              ((msg.GetSender() == -1) ? pipe->storage.mutReaderMessages  : &(*pipe->storage.qMutexes)[msg.GetSender()]);
    std::list<Message>* pMessages = ((msg.GetSender() == -1) ? pipe->storage.listReaderMessages : &(*pipe->storage.vectMessages)[msg.GetSender()]);

    // Write the message to list
    pMut->lock();
    pMessages->push_back(msg);
    pMut->unlock();


    //_tprintf(TEXT("[%d] %s\n"), pipe->hPipeInst, pipe->chRequest);
    //StringCchCopy(pipe->chReply, BUFSIZE, TEXT("Default answer from server"));
    //pipe->cbToWrite = (lstrlen(pipe->chReply) + 1) * sizeof(TCHAR);
}
*/

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
    //hPipe = CreateNamedPipe(
    //    (const char*)cPipeName,   // pipe name 
    //    PIPE_ACCESS_DUPLEX,       // read/write access 
    //    PIPE_TYPE_MESSAGE |       // message type pipe 
    //    PIPE_READMODE_MESSAGE |   // message-read mode 
    //    PIPE_WAIT,                // blocking mode 
    //    PIPE_UNLIMITED_INSTANCES, // max. instances  
    //    PIPE_BUFFER_SIZE,         // output buffer size 
    //    PIPE_BUFFER_SIZE,         // input buffer size 
    //    0,                        // client time-out 
    //    NULL                      // default security attribute 
    //);
    //if (hPipe == INVALID_HANDLE_VALUE)
    //{
    //    printf("CreateNamedPipe failed, GLE=%d.\n", GetLastError());
    //}

    memcpy(cListenerPipeName, cPipeName, strlen(cPipeName));

    iNodesQuantity = iNodesNum;
    for (int i = 0; i < iNodesQuantity; i++)
    {
        std::list<Message> tmp;
        vectRecievedMessages.push_back(tmp);
    }

    qMutexes = new std::deque<std::mutex>(iNodesQuantity);

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
    
    
    if (msg->GetReciever() == -1) // To reader
    {
        memcpy(cPipeName, "\\\\.\\pipe\\RFIDsecPipeReader", 26);
    }
    else // To a tag
    {
        memcpy(cPipeName, "\\\\.\\pipe\\RFIDsecPipe", 20); // Pipe name in format: \\.\pipe\RFIDsecPipe< Number of this node >
        _itoa(msg->GetReciever(), &cPipeName[20], 10);
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
            return -1;
        }
    
        // All pipe instances are busy, so wait. 
        if (!WaitNamedPipe(cPipeName, NMPWAIT_WAIT_FOREVER))
        {
            printf("Could not open pipe: 2 second wait timed out.");
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
            pMut->unlock();

            return 0; // Message was recieved
        }
        else
        {
            Sleep(RECV_SLEEP_TIME);
        }
    }
    printf("F");
    return -1; // Waiting time is over. Message was not recieved
}
