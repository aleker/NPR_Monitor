//
// Created by ola on 05.04.18.
//

#include "Message.h"

Message::Message(MessageType messageType, int recvId) : messageType(messageType), recvId(recvId) {}

MessageType Message::getMessageType() const {
    return messageType;
}

const void *Message::serialize() {
    // TODO zrob serializacje
    return nullptr;
}
