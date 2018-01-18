#pragma once
#include "pipeline.h"

enum Status {
    FAILED = -1,
    SUCCESS,
    PENDING
};

class Player: public Pipeline {
public:
    int setSource(const char* url) override;
    int prepare() override;
    int play() override;
    int pause() override;
    int resume() override;
    int seek(int64_t pos) override;
    int getState() override;
    int getPosition(int64_t& pos) override;
    int getDuration(int64_t& duration) override;
    int getMessage(int& messagedId, void*& messsage) override;
};