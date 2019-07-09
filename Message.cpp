#include "Message.h"

Message::Message()
{
    msgMessage.iSender   = -1;
    msgMessage.iReciever = -1;
    msgMessage.iSize     = -1;
}

Message::Message(int iSenderNode, int iRecieverNode, void* pData, int iSizeOfData)
{
    msgMessage.iSender   = iSenderNode;
    msgMessage.iReciever = iRecieverNode;
    msgMessage.iSize     = iSizeOfData;
    memcpy(msgMessage.cBuffer, pData, iSizeOfData);
}

Message::Message(void* pRaw)
{
    msgMessage.iSender = ((Message*)pRaw)->GetSender();
    msgMessage.iReciever = ((Message*)pRaw)->GetReciever();
    msgMessage.iSize = ((Message*)pRaw)->GetSize();
    memcpy(msgMessage.cBuffer, ((Message*)pRaw)->GetData(), msgMessage.iSize);
}

Message::~Message()
{
}

Message& Message::operator=(Message& right)
{
    msgMessage.iSender   = right.GetSender();
    msgMessage.iReciever = right.GetReciever();
    msgMessage.iSize     = right.GetSize();
    memcpy(msgMessage.cBuffer, right.GetData(), msgMessage.iSize);
    return *this;
}

int Message::GetSender()
{
    return msgMessage.iSender;
}

int Message::GetReciever()
{
    return msgMessage.iReciever;
}

int Message::GetSize()
{
    return msgMessage.iSize;
}

char* Message::GetData()
{
    return msgMessage.cBuffer;
}

void* Message::GetRawMessage()
{
    return &msgMessage;
}
