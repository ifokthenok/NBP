#include "log.h"
#include "ffwrapper.h"
#include "video_render.h"

#undef  LOG_TAG 
#define LOG_TAG "VideoRender"

VideoRender::VideoRender() {   
}

VideoRender::~VideoRender() {    
}

int VideoRender::toIdle() {
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
        return STATUS_FAILED;
    } 
    
    if (current == PAUSED) {
        // TODO: such as stop threads etc.
        return STATUS_FAILED;
    } 

    LOGE("toReady failed: current state is %s", cstr(current));
    return STATUS_FAILED;
}

int VideoRender::toPaused() {
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

int VideoRender::toPlaying() {
    return STATUS_FAILED;
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
    static bool scale = true;
    if (scale) {
        ffWrapper->setVideoScale(frame, 800, 600, AV_PIX_FMT_RGB24);
        scale = false;  
    }
    uint8_t* dst_data = new uint8_t[800*600*3];
    int dst_linesize = 800*3;
    ffWrapper->scaleVideo(frame, &dst_data, &dst_linesize);
    ffWrapper->freeFrame(frame);
    FILE* video_dst_file = fopen("./video_render_800x600.rgb", "ab+");
    fwrite(dst_data, 1, 800*600*3, video_dst_file);
    fclose(video_dst_file);
    delete dst_data;
    return STATUS_SUCCESS;
}


bool VideoRender::BufferCompare::operator()(const Buffer& lBuf, const Buffer& rBuf) {
    AVFrame* lp = static_cast<AVFrame*>(lBuf.data);
    AVFrame* rp = static_cast<AVFrame*>(rBuf.data);
    return lp->pts >= rp->pts;
};