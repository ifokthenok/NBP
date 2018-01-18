#pragma once

#include "state.h"
#include "clock.h"
#include "bus.h"

struct Element {
    virtual void setBus(Bus* bus) = 0;
    
    virtual void setClock(Clock* clock) = 0;
    virtual Clock* getClock() = 0;

    virtual int setState(State state) = 0;
    virtual State getState() = 0;
    
    virtual int sendEvent(void* event) = 0;
    virtual int pushBuffer(void* buffer) = 0;
};