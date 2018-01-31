#include "log.h"
#include "ffwrapper.h"
#include "audio_render.h"

#undef  LOG_TAG 
#define LOG_TAG "AudioRender"

AudioRender::AudioRender() {
    audioDevice = AudioDevice::create("AudioTrackDevice");  
}

AudioRender::~AudioRender() {
    delete audioDevice;    
}

int AudioRender::toIdle() {
    State current = states.getCurrent();
    if (current == IDLE) {
        LOGW("toIdle: current state is IDLE");
        return STATUS_SUCCESS;
    }
    if (current != READY) {
        LOGE("toIdle failed: current state is %s", cstr(current));
        return STATUS_FAILED;
    }
    return STATUS_SUCCESS;
}

int AudioRender::toReady() {
    State current = states.getCurrent();
    if (current == READY) {
        LOGW("toReady: current state is READY");
        return STATUS_SUCCESS;
    }

    if (current == IDLE) {
        // TODO:
        return STATUS_SUCCESS;
    } 
    
    if (current == PAUSED) {
        // TODO: such as stop threads etc.
        return STATUS_SUCCESS;
    } 

    LOGE("toReady failed: current state is %s", cstr(current));
    return STATUS_FAILED;
}

int AudioRender::toPaused() {
    State current = states.getCurrent();
    if (current == PAUSED) {
        LOGW("toPaused: current state is PAUSED");
        return STATUS_SUCCESS;
    }

    if (current == READY) {
        // TODO: start thread etc.
        return STATUS_SUCCESS;
    } 
    
    if (current == PLAYING) {
        // TODO: flash buffer etc.
        return STATUS_SUCCESS;
    } 

    LOGE("toPaused failed: current state is %s", cstr(current));
    return STATUS_FAILED;
}

int AudioRender::toPlaying() {
    return STATUS_FAILED;
}

int AudioRender::setState(State state) {
    return STATUS_FAILED;
}

int AudioRender::sendEvent(const Event& event) {
    return STATUS_FAILED;
}


int AudioRender::pushBuffer(const Buffer& buffer) {
    // if (states.getCurrent() == IDLE) {
    //     LOGE("pushBuffer failed: current state is IDLE");
    //     return STATUS_FAILED;
    // }

    // if (!bufferQueue.push(buffer)) {
    //     LOGE("pushBuffer failed: buffer queue is empty");
    //     return STATUS_FAILED;  
    // }
    AVFrame* frame = static_cast<AVFrame*>(buffer.data);
    audioDevice->write(frame, sizeof(AVFrame));
    return STATUS_SUCCESS;
}


bool AudioRender::BufferCompare::operator()(const Buffer& lBuf, const Buffer& rBuf) {
    AVFrame* lp = static_cast<AVFrame*>(lBuf.data);
    AVFrame* rp = static_cast<AVFrame*>(rBuf.data);
    return lp->pts >= rp->pts;
};