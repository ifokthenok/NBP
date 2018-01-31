#include "log.h"
#include "ffwrapper.h"
#include "audio_device.h"

class AudioTrackDevice: public AudioDevice {
public:
    bool setProperty(int key, void* value) override {
        switch(key) {
        case AUDIO_ENGIN:
            ffWrapper = static_cast<FFWrapper*>(value);
            break;
        case AUDIO_SAMPLE_RATE:
            sampleRate = *static_cast<int*>(value);
            break;
        default:
            return false;
        }
        return true;
    }

    int  getSampleRate() override {
        return 0;
    }

    int  getSampleFormat() override {
        return 0;
    }

    int  getChannels() override {
        return 0;
    }

    int  getPlaybackPosition() override {
        return 0;
    }

    int  write(void* buf, int buflen) override {
        AVFrame* frame = static_cast<AVFrame*>(buf);
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
        return 2 * 2 * dst_samples;
    }

    void play() override {
    }

    void pause() override {
    }

    void flush() override {
    }

    void stop() override {
    }
private:
    int sampleRate = 0;
    FFWrapper* ffWrapper = nullptr;
};

AudioDevice* AudioDevice::create(const std::string& name) {
    if (name != "AudioTrackDevice") {
        return nullptr;
    }
    return new AudioTrackDevice;
}

void AudioDevice::release(AudioDevice* device) {
    delete device;
}