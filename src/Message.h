//
// Created by ola on 05.04.18.
//

#ifndef NPR_MONITOR_MESSAGE_H
#define NPR_MONITOR_MESSAGE_H


enum MessageType {
    REQUEST,
    RESPONSE
};

class Message {
protected:
    MessageType messageType;
    int recvId;

public:
    Message(MessageType messageType, int recvId);

    MessageType getMessageType() const;
    const void* serialize();
};


#endif //NPR_MONITOR_MESSAGE_H
