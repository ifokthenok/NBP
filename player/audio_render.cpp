#include "log.h"
#include "ffwrapper.h"
#include "audio_render.h"

#undef  LOG_TAG 
#define LOG_TAG "AudioRender"

AudioRender::AudioRender() {   
}

AudioRender::~AudioRender() {    
}

int AudioRender::toIdle() {
    return STATUS_FAILED;
}

int AudioRender::toReady() {
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

int AudioRender::toPaused() {
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
    static bool resample = true;
    if (resample) {
        ffWrapper->setAudioResample(frame, AV_CH_LAYOUT_STEREO, frame->sample_rate, AV_SAMPLE_FMT_S16);
        resample = false;    
    }
    uint8_t* dst_data = new uint8_t[2 * 2 * frame->nb_samples];
    int dst_samples = frame->nb_samples;
    ffWrapper->resampleAudio(frame, &dst_data, dst_samples);
    ffWrapper->freeFrame(frame);
    FILE* audio_dst_file = fopen("./audio_render_s16le_ac2.pcm", "ab+");
    fwrite(dst_data, 1, 2 * 2 * dst_samples, audio_dst_file);
    fclose(audio_dst_file);
    delete dst_data;
    return STATUS_SUCCESS;
}


bool AudioRender::BufferCompare::operator()(const Buffer& lBuf, const Buffer& rBuf) {
    AVFrame* lp = static_cast<AVFrame*>(lBuf.data);
    AVFrame* rp = static_cast<AVFrame*>(rBuf.data);
    return lp->pts >= rp->pts;
};