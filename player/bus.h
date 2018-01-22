#pragma once

#include "utils.h"

#define MESSAGE_ERROR
#define MESSAGE_EOS
#define MESSAGE_DURATION

struct Message {
    Message(int id, void* data) : id(id), data(data) {}
    Message(int id) : id(id) {}
    Message() {}
    int id = -1;
    void* data = nullptr;
};

struct Bus {
    virtual bool sendMessage(const Message& messages) {
        return messageQueue.push(messages);
    }
    virtual bool getMessage(Message& message) {
        return messageQueue.pop(message);
    }
private:
    Queue<Message> messageQueue;
};