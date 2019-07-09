#pragma once
#include <Windows.h>

#define MSG_SIZE 512

// 
// Class which describes a message
// 
class Message
{
public:

    //
    // Structure for containing a message
    //
    struct stMessage
    {
        int  iSender;           // Number of sender node
        int  iReciever;         // Number of reciever node
        int  iSize;             // Size of buffer in bytes
        char cBuffer[MSG_SIZE]; // Buffer which contains the message
    };

    // 
    // Empty message
    // 
    Message();

    // 
    // Create an initialized instance of Message class from all data
    // 
    Message(
        int   iSenderNode,   // Number of sender node
        int   iRecieverNode, // Number of reciever node
        void* pData,         // Pointer to data which will be contained in message
        int   iSizeOfData    // Size of buffer in bytes
    );

    // 
    // Create an initialized instance of Message class from a raw message
    // 
    Message(
        void* pRaw           // Pointer to a raw message
    );

    ~Message();

    //
    // Operator =
    //
    Message& operator=(Message& right);

    //
    // Get number of this message's sender node
    //
    int   GetSender();

    //
    // Get number of this message's reciever node
    //
    int   GetReciever();

    //
    // Get size of data (in bytes) which is containing in this message
    //
    int   GetSize();

    //
    // Get pointer to buffer which is containing in this message
    //
    char* GetData();

    //
    // Get a pointer to message which is ready to be sent
    //
    void* GetRawMessage();

private:

    stMessage  msgMessage; // Actually message
};
