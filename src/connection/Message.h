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
const int MAX_MSG_SIZE = 50;

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
        archive & sendersClock;
        archive & messageType;
        archive & receiversId;
        archive & gotRequestClock;
        archive & sendersClock;
        if (!mtxName.empty())
            archive & mtxName;
    }
    /*
    *
    */

    int sendersId = NOT_SET;
    int messageType = MessageType::DATA;
    int receiversId = NOT_SET;
    int gotRequestClock = NOT_SET;
    int sendersClock = NOT_SET;
    std::string mtxName;

public:
    enum MessageType {
        DATA,
        LOCK_MTX,
        UNLOCK_MTX
    };

    Message() = default;
    Message(int sendersId, int messageType, int receiversId, int gotRequestClock, std::string mtxName) :
            sendersId(sendersId),
            messageType(messageType),
            receiversId(receiversId),
            gotRequestClock(gotRequestClock),
            mtxName(mtxName) {}

    Message(int sendersId, int messageType, int receiversId, int gotRequestClock) :
            Message(sendersId, messageType, receiversId, NOT_SET, ""){}


    Message(int sendersId, int messageType, int receiversId) :
            Message(sendersId, messageType, receiversId, NOT_SET, ""){}

    Message(int sendersId, int messageType, std::string mtxName) :
            Message(sendersId, messageType, NOT_SET, NOT_SET, mtxName){}

    Message(int sendersId, int messageType) :
            Message(sendersId, messageType, NOT_SET, NOT_SET, ""){}

    int getSendersClock() const {
        return sendersClock;
    }

    int getSendersId() const {
        return sendersId;
    }

    int getReceiversId() const {
        return receiversId;
    }

    int getMessageType() const {
        return messageType;
    }

    void setSendersClock(int sendersClock) {
        this->sendersClock = sendersClock;
    }

    void setReceiversId(int receiversId) {
        this->receiversId = receiversId;
    }

    std::string getMtxName() {
        return mtxName;
    }

    void setMtxName(std::string mtxName) {
        this->mtxName = mtxName;
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
