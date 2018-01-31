#pragma once


#include "element.h"
#include "video_device.h"
#include "ffwrapper.h"
#include "utils.h"

class VideoRender: public Element {

public:   
    VideoRender();
    ~VideoRender();

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
        videoDevice->setProperty(VIDEO_ENGIN, ffWrapper);
    }
    void setSource(Element* videoDecoder) {
        this->videoDecoder = videoDecoder;
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
    Element* videoDecoder = nullptr;
    States states;
    VideoDevice* videoDevice = nullptr;

private:
    struct BufferCompare {
        bool operator()(const Buffer& lBuf, const Buffer& rBuf);
    };

    PriorityQueue<Buffer, BufferCompare> bufferQueue;
};