#include "log.h"
#include "ffwrapper.h"
#include "demuxer.h"


Demuxer::Demuxer() {   
}

Demuxer::~Demuxer() {    
}

void Demuxer::demuxing() {
    LOGD("Demuxer::demuxing thread entered");
    for (;;) {
        Event ev;
        if (!eventQueue.pop(ev, 1000)) {
            continue;
        }
        if (ev.id == EVENT_STOP_THREAD) {
            LOGD("Demuxer::demuxing thread exited");
            break;
        }
    }
}

int Demuxer::toIdle() {
    State current = states.getCurrent();
    if (current == IDLE) {
        LOGW("toReady: current state is IDLE");
        return STATUS_SUCCESS;
    }

    if (current == READY) {
        ffWrapper->close();
        states.setCurrent(IDLE);
        return STATUS_SUCCESS;
    }


    LOGE("toReady failed: current state is %s", cstr(current));
    return STATUS_FAILED;
}

int Demuxer::toReady() {
    State current = states.getCurrent();
    if (current == READY) {
        LOGW("toReady: current state is READY");
        return STATUS_SUCCESS;
    }

    if (current == IDLE) {
        if (!ffWrapper->open(url)) {
            LOGE("toReady failed: can't open %s", url);
            return STATUS_FAILED;
        }
        states.setCurrent(READY);
        return STATUS_SUCCESS;
    } 
    
    if (current == PAUSED) {
        sendEvent(Event(EVENT_STOP_THREAD));
        demuxingThread.join();
        states.setCurrent(READY);
        return STATUS_SUCCESS;
    } 

    LOGE("toReady failed: current state is %s", cstr(current));
    return STATUS_FAILED;
}

int Demuxer::toPaused() {
    State current = states.getCurrent();
    if (current == PAUSED) {
        LOGW("toReady: current state is PAUSED");
        return STATUS_SUCCESS;
    }

    if (current == READY) {
        demuxingThread = std::thread(&Demuxer::demuxing, this);
        states.setCurrent(PAUSED);
        return STATUS_SUCCESS;
    } 
    
    if (current == PLAYING) {
        // TODO: flash buffer etc.
        return STATUS_FAILED;
    } 

    LOGE("toPaused failed: current state is %s", cstr(current));
    return STATUS_FAILED;
}

int Demuxer::toPlaying() {
    return STATUS_FAILED;
}

int Demuxer::setState(State state) {
    return STATUS_FAILED;
}

int Demuxer::sendEvent(const Event& event) {
    State current = states.getCurrent();
    if (current == IDLE || current == READY) {
        LOGE("sendEvent failed: current state is %s", cstr(current));
        return STATUS_FAILED;
    }

    if (!eventQueue.push(event)) {
        LOGE("sendEvent failed: event queue is full, current state is %s", cstr(current));
        return STATUS_FAILED;
    }
    return STATUS_SUCCESS;
}


int Demuxer::pushBuffer(const Buffer& buffer) {
    LOGE("pushBuffer failed: not supported");
    return STATUS_FAILED; 
}

