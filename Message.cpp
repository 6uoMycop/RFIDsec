#include "Message.h"

Message::Message()
{
    msgMessage.iMsgID   = -1; // reserved ID of empty message
}

Message::Message(int iSenderNode, int iRecieverNode, int iMsgID, const void* pData, int iSizeOfData)
{
    msgMessage.iSender   = iSenderNode;
    msgMessage.iReciever = iRecieverNode;
    msgMessage.iMsgID    = iMsgID;
    msgMessage.iSize     = iSizeOfData;
    memcpy(msgMessage.cBuffer, pData, iSizeOfData);
}

Message::Message(void* pRaw)
{
    msgMessage.iSender   = ((Message*)pRaw)->GetSender();
    msgMessage.iReciever = ((Message*)pRaw)->GetReciever();
    msgMessage.iMsgID    = ((Message*)pRaw)->GetID();
    msgMessage.iSize     = ((Message*)pRaw)->GetSize();
    memcpy(msgMessage.cBuffer, ((Message*)pRaw)->GetData(), msgMessage.iSize);
}

Message::~Message()
{
}

Message& Message::operator=(Message& right)
{
    msgMessage.iSender   = right.GetSender();
    msgMessage.iReciever = right.GetReciever();
    msgMessage.iMsgID    = right.GetID();
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

int Message::GetID()
{
    return msgMessage.iMsgID;
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
