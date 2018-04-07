//
// Created by ola on 07.04.18.
//

#ifndef NPR_MONITOR_MPI_MESSAGE_H
#define NPR_MONITOR_MPI_MESSAGE_H

#include "Message.h"

const int MAX_MPI_MSG_SIZE = 50;


class MPI_Msg : public Message {
private:
    friend class boost::serialization::access;
    // TODO serialization not working
    template<class Archive>
    void serialize(Archive &archive, const unsigned int version) {
        archive & boost::serialization::base_object<Message>(*this);
        archive & sendersId;
    }

    int sendersId;
    int receiversId;
    int messageType;

public:
    MPI_Msg(int sendersId, int receiversId, int messageType) :
            sendersId(sendersId),
            receiversId(receiversId),
            messageType(messageType) {}
    int getSendersId() const {
        return sendersId;
    }

    int getReceiversId() const {
        return receiversId;
    }

    int getMessageType() const {
        return messageType;
    }

};


#endif //NPR_MONITOR_MPI_MESSAGE_H
