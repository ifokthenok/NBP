#pragma once

#include "element.h"
#include "ffwrapper.h"

class Demuxer: public Element {

public:    
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
        return status.getCurrent();
    }
    
    int sendEvent(const Event& event) override;
    int pushBuffer(const Buffer& buffer) override;

public:
    void setEngine(FFWrapper* ffWrapper) {
        this->ffWrapper = ffWrapper;
    }
    void setSource(const char* url) {
        this->url = url;
    }
    void setAudioSink(Element* audioSink) {
        this->audioSink = audioSink;
    }
    void setVideoSink(Element* videoSink) {
        this->videoSink = videoSink;
    }

private:
    Clock* clock = nullptr;
    Bus* bus = nullptr;
    FFWrapper* ffWrapper = nullptr;
    const char* url = nullptr;
    Element* audioSink = nullptr;
    Element* videoSink = nullptr;
    States status;

};