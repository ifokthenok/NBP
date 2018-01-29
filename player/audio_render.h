#pragma once


#include "element.h"
#include "ffwrapper.h"
#include "utils.h"

class AudioRender: public Element {

public:   
    AudioRender();
    ~AudioRender();

    void setBus(Bus* bus) override {
        this->bus = bus;
    }
    
    void setClock(Clock* clock) override {
        this->clock = clock;
    }
    Clock* getClock() override {
        return clock;
    }

    int setState(State state) override;
    State getState() override {
        return states.getCurrent();
    }
    
    int sendEvent(const Event& event) override;
    int pushBuffer(const Buffer& buffer) override;

public:
    void setEngine(FFWrapper* ffWrapper) {
        this->ffWrapper = ffWrapper;
    }
    void setSource(Element* audioDecoder) {
        this->audioDecoder = audioDecoder;
    }

//private:
    int toIdle();
    int toReady();
    int toPaused();
    int toPlaying();

private:
    Clock* clock = nullptr;
    Bus* bus = nullptr;
    FFWrapper* ffWrapper = nullptr;
    Element* audioDecoder = nullptr;
    Element* audioSink = nullptr;
    States states;

private:
    struct BufferCompare {
        bool operator()(const Buffer& lBuf, const Buffer& rBuf);
    };

    PriorityQueue<Buffer, BufferCompare> bufferQueue;
};