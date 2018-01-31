#include "log.h"
#include "ffwrapper.h"
#include "video_device.h"


#undef  LOG_TAG 
#define LOG_TAG "VideoDevice"

class SurfaceDevice: public VideoDevice {
public:
    bool setProperty(int key, void* value) override {
        switch (key) {
            case VIDEO_ENGIN:
                ffWrapper = static_cast<FFWrapper*>(value);
                break;
            case VIDEO_WIDTH:
                videoWidth = *static_cast<int*>(value);
                break;
            case VIDEO_HEIGHT:
                videoHeight = *static_cast<int*>(value);
                break;
            default:
                LOGE("setProperty: unsupported key=%d", key);
                return false;
        }
        return true;
    }

    int getPixelFormat() override {
        return AV_PIX_FMT_RGB24;
    }

    int getWidth() override {
        return 800;
    }

    int getHeight() override {
        return 600;
    }

    int write(void* buf, int buflen) override {
        AVFrame* frame = static_cast<AVFrame*>(buf);
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
        return 800*600*3;
    }
private:
    int videoWidth = 0;
    int videoHeight = 0;
    FFWrapper* ffWrapper = nullptr;
};

VideoDevice* VideoDevice::create(const std::string name) {
    if (name != "SurfaceDevice") {
        return nullptr; 
    } 
    return new SurfaceDevice;
}

void VideoDevice::release(VideoDevice* device) {
    delete device;
}
