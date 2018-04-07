//
// Created by ola on 07.04.18.
//

#ifndef NPR_MONITOR_MESSAGE_H
#define NPR_MONITOR_MESSAGE_H

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>


class Message {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &archive, Message &msg, const unsigned int version) {
        archive & sendersClock;
    }

    int sendersClock = 0;

public:
    int getSendersClock() const {
        return sendersClock;
    }

    void setSendersClock(int sendersClock) {
        this->sendersClock = sendersClock;
    }
};


#endif //NPR_MONITOR_MESSAGE_H
