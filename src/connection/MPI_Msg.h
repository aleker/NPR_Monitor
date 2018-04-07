#ifndef NPR_MONITOR_MPI_MESSAGE_H
#define NPR_MONITOR_MPI_MESSAGE_H

#include "Message.h"

enum MessageType {
    EMPTY,
    REQUEST,
    REPLY
};

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

    int sendersId = 0;
    int receiversId = 0;
    int messageType = 0;

public:
    MPI_Msg() = default;
    MPI_Msg(int sendersId, int receiversId, int messageType) : Message(),
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
