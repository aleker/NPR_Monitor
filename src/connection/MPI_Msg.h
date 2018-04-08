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
    /*
    * When inheriting you have to add serialization mechanism to your class.
    */
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &archive, const unsigned int version) {
        /*
        * Here you have to call base serialization first
        */
        archive & boost::serialization::base_object<Message>(*this);
        archive & messageType;
    }
    /*
    *
    */

    int messageType = 0;

public:
    MPI_Msg() = default;
    MPI_Msg(int sendersId, int receiversId, int messageType) :
            Message(sendersId, receiversId, NO_REQUEST_CLOCK),
            messageType(messageType) {}
    MPI_Msg(int sendersId, int receiversId, int messageType, int gotRequestClock) :
            Message(sendersId, receiversId, gotRequestClock),
            messageType(messageType) {}

    int getMessageType() const {
        return messageType;
    }

};



#endif //NPR_MONITOR_MPI_MESSAGE_H
