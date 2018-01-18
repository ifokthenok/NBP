#pragma once

#include <stdint.h>

struct Pipeline {
    virtual int setSource(const char* url) = 0;
    virtual int prepare() = 0;
    virtual int play() = 0;
    virtual int pause() = 0;
    virtual int resume() = 0;
    virtual int seek(int64_t pos) = 0;
    virtual int getState() = 0;
    virtual int getPosition(int64_t& pos) = 0;
    virtual int getDuration(int64_t& duration) = 0;
    virtual int getMessage(int& messagedId, void*& messsage) = 0;
};