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

const int NO_REQUEST_CLOCK = -1;
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
        archive & sendersId;
        archive & receiversId;
        archive & gotRequestClock;
        archive & messageType;
    }
    /*
    *
    */

    int sendersClock = 0;
    int sendersId = 0;
    int receiversId = 0;
    int gotRequestClock = NO_REQUEST_CLOCK;
    int messageType = 0;

public:
    Message() = default;
    Message(int sendersId, int receiversId, int gotRequestClock, int messageType) :
            sendersId(sendersId),
            receiversId(receiversId),
            gotRequestClock(gotRequestClock),
            messageType(messageType) {}

    Message(int sendersId, int receiversId, int messageType) :
            Message(sendersId, receiversId, NO_REQUEST_CLOCK, messageType){
    }

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
