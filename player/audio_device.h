#pragma once

#include <string>

#define AUDIO_ENGIN         0x01
#define AUDIO_SAMPLE_RATE   0x02

struct AudioDevice {
    static AudioDevice* create(const std::string& name);
    static void release(AudioDevice* device);
    virtual ~AudioDevice() {};
    virtual bool setProperty(int key, void* value) = 0;
    virtual int  getSampleRate() = 0;
    virtual int  getSampleFormat() = 0;
    virtual int  getChannels() = 0;
    virtual int  getPlaybackPosition() = 0;
    virtual int  write(void* buf, int buflen) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void flush() = 0;
    virtual void stop() = 0;
};

