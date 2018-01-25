#include "log.h"
#include "ffwrapper.h"
#include "audio_decoder.h"

#undef  LOG_TAG 
#define LOG_TAG "AudioDecoder"

AudioDecoder::AudioDecoder() {   
}

AudioDecoder::~AudioDecoder() {    
}

int AudioDecoder::toIdle() {
    return STATUS_FAILED;
}

int AudioDecoder::toReady() {
    State current = states.getCurrent();
    if (current == READY) {
        LOGW("toReady: current state is READY");
        return STATUS_SUCCESS;
    }

    if (current == IDLE) {
        // TODO:
        return STATUS_FAILED;
    } 
    
    if (current == PAUSED) {
        // TODO: such as stop threads etc.
        return STATUS_FAILED;
    } 

    LOGE("toReady failed: current state is %s", cstr(current));
    return STATUS_FAILED;
}

int AudioDecoder::toPaused() {
    State current = states.getCurrent();
    if (current == PAUSED) {
        LOGW("toReady: current state is PAUSED");
        return STATUS_SUCCESS;
    }

    if (current == READY) {
        // TODO: start thread etc.
        return STATUS_FAILED;
    } 
    
    if (current == PLAYING) {
        // TODO: flash buffer etc.
        return STATUS_FAILED;
    } 

    LOGE("toPaused failed: current state is %s", cstr(current));
    return STATUS_FAILED;
}

int AudioDecoder::toPlaying() {
    return STATUS_FAILED;
}

int AudioDecoder::setState(State state) {
    return STATUS_FAILED;
}

int AudioDecoder::sendEvent(const Event& event) {
    return STATUS_FAILED;
}


int AudioDecoder::pushBuffer(const Buffer& buffer) {
    if (states.getCurrent() == IDLE) {
        LOGE("pushBuffer failed: current state is IDLE");
        return STATUS_FAILED;
    }

    if (!bufferQueue.push(buffer)) {
        LOGE("pushBuffer failed: buffer queue is empty");
        return STATUS_FAILED;  
    }
    return STATUS_SUCCESS;
}


bool AudioDecoder::BufferCompare::operator()(const Buffer& lBuf, const Buffer& rBuf) {
    AVPacket* lp = static_cast<AVPacket*>(lBuf.data);
    AVPacket* rp = static_cast<AVPacket*>(rBuf.data);
    return lp->dts >= rp->dts;
};