#include "log.h"
#include "ffwrapper.h"
#include "video_render.h"
#include "video_device.h"

#undef  LOG_TAG 
#define LOG_TAG "VideoRender"

VideoRender::VideoRender() {
    videoDevice = VideoDevice::create("SurfaceDevice"); 
}

VideoRender::~VideoRender() {    
    VideoDevice::release(videoDevice);
}

int VideoRender::toIdle() {
    State current = states.getCurrent();
    if (current == IDLE) {
        LOGW("toIdle: current state is IDLE");
        return STATUS_SUCCESS;
    }
    
    if (current != READY) {
        LOGE("toIdle failed: current state is %s", cstr(current));
        return STATUS_FAILED;
    }

    return STATUS_FAILED;
}

int VideoRender::toReady() {
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

int VideoRender::toPaused() {
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

int VideoRender::toPlaying() {
    State current = states.getCurrent();
    if (current == PLAYING) {
        LOGW("toPlaying: current state is PLAYING");
        return STATUS_SUCCESS;
    }
    if (current != PAUSED) {
        LOGE("toPlaying failed: current state is %s", cstr(current));
        return STATUS_FAILED;
    }
    return STATUS_SUCCESS;
}

int VideoRender::setState(State state) {
    return STATUS_FAILED;
}

int VideoRender::sendEvent(const Event& event) {
    return STATUS_FAILED;
}


int VideoRender::pushBuffer(const Buffer& buffer) {
    /*
    if (states.getCurrent() == IDLE) {
        LOGE("pushBuffer failed: current state is IDLE");
        return STATUS_FAILED;
    }

    if (!bufferQueue.push(buffer)) {
        LOGE("pushBuffer failed: buffer queue is empty");
        return STATUS_FAILED;  
    }
    */
    AVFrame* frame = static_cast<AVFrame*>(buffer.data);
    videoDevice->write(frame, sizeof(AVFrame));
    return STATUS_SUCCESS;
}


bool VideoRender::BufferCompare::operator()(const Buffer& lBuf, const Buffer& rBuf) {
    AVFrame* lp = static_cast<AVFrame*>(lBuf.data);
    AVFrame* rp = static_cast<AVFrame*>(rBuf.data);
    return lp->pts >= rp->pts;
};