#ifndef NPR_MONITOR_MESSAGE_H
#define NPR_MONITOR_MESSAGE_H

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include <sstream>
#include <boost/archive/text_iarchive.hpp>

const int NOT_SET = -1;
// TODO message not bigger than MAX_MSG_SIZE
const int MAX_MSG_SIZE = 100;

class Message {
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
        archive & sendersDistributedId;
        archive & sendersLocalId;
        archive & messageType;
        archive & messageTypeId;
        archive & receiversDistributedId;
        archive & requestClock;
        archive & sendersClock;
        archive & data;
    }
    /*
    *
    */

    int sendersDistributedId = NOT_SET;
    int sendersLocalId = NOT_SET;
    int messageType = MessageType ::LOCK_MTX;
    int messageTypeId = NOT_SET;
    int receiversDistributedId = NOT_SET;
    int requestClock = NOT_SET;
    int sendersClock = NOT_SET;
    std::string data = "";

public:
    enum MessageType {
        LOCK_MTX,
        LOCK_RESPONSE,
        UNLOCK_MTX
    };

    Message() = default;
    Message(int sendersDistributedId, int sendersLocalId, int messageType, int gotRequestClock, std::string data) :
            sendersDistributedId(sendersDistributedId),
            sendersLocalId(sendersLocalId),
            messageType(messageType),
            requestClock(gotRequestClock),
            data(data) {}

    Message(int sendersDistributedId, int sendersLocalId, int messageType, int gotRequestClock) :
            Message(sendersDistributedId, sendersLocalId, messageType, gotRequestClock, ""){}

    Message(int sendersDistributedId, int sendersLocalId, int messageType, std::string data) :
            Message(sendersDistributedId, sendersLocalId, messageType, NOT_SET, data){}

    Message(int sendersDistributedId, int sendersLocalId, int messageType) :
            Message(sendersDistributedId, sendersLocalId, messageType, NOT_SET, ""){}

    int getSendersClock() const {
        return sendersClock;
    }

    int getSendersDistributedId() const {
        return sendersDistributedId;
    }

    int getSendersLocalId() const {
        return sendersLocalId;
    }

    int getReceiversId() const {
        return receiversDistributedId;
    }

    int getMessageTypeId() const {
        return messageTypeId;
    }

    int getMessageType() const {
        return messageType;
    }

    int getRequestClock() const {
        return requestClock;
    }

    std::string getData() const {
        return data;
    }

    void setSendersClock(int sendersClock) {
        this->sendersClock = sendersClock;
    }

    void setReceiversId(int receiversDistributedId, int receiversLocalId) {
        this->receiversDistributedId = receiversDistributedId;
        this->messageTypeId = this->messageType + receiversLocalId;
    }

    void setClassName(std::string data) {
        this->data = data;
    }

    template<typename T>
    static std::string serializeMessage(T msg) {
        std::ostringstream serialMsg;
        {
            boost::archive::text_oarchive oa(serialMsg);
            oa << msg;
        }
        return serialMsg.str();
    }

    template<typename T>
    static T deserializeMessage(std::string s) {
        T deserializedMessage;
        std::istringstream iss(s);
        {
            boost::archive::text_iarchive ia(iss);
            ia >> deserializedMessage;
        }
        return deserializedMessage;
    }
};


#endif //NPR_MONITOR_MESSAGE_H
