#include <list>
#include "log.h"
#include "ffwrapper.h"

using namespace std;

int main(int argc, char* argv[]) {
    FFWrapper ffwrapper;
    ffwrapper.open(argv[1]);

    LOGD("container info: start_time=%.6g, duration=%.6g",
        ffwrapper.startTime()/double(AV_TIME_BASE),
        ffwrapper.duration()/double(AV_TIME_BASE));
    
    LOGD("video info: pix_fmt=%s, video_size=%dx%d, "
        "time_base=%.6g, start_time=%.6g, duration=%.6g, frames=%lld, fps=%.6g",
        ffwrapper.videoPixelFormat(),
        ffwrapper.videoWidth(),
        ffwrapper.videoHeight(),
        ffwrapper.videoTimeBase(),
        ffwrapper.videoTimeBase()*ffwrapper.videoStartTime(), 
        ffwrapper.videoTimeBase()*ffwrapper.videoDuration(),
        ffwrapper.videoFrames(), ffwrapper.videoFPS());
    
    LOGD("audio info: sample_fmt=%s, channels=%d, sample_rate=%d, "
        "time_base=%.6g, start_time=%.6g, duration=%.6g",
        ffwrapper.audioSampleFormat(),
        ffwrapper.audioChannels(),
        ffwrapper.audioSampleRate(),
        ffwrapper.audioTimeBase(),
        ffwrapper.audioTimeBase()*ffwrapper.audioStartTime(),
        ffwrapper.audioTimeBase()*ffwrapper.audioDuration());

    
    ffwrapper.seek(10*AV_TIME_BASE);

    list<AVPacket> videoPackets;
    list<AVPacket> audioPackets;

    int frameCount = 0;
    AVPacket packet;
    
    while (ffwrapper.readPacket(packet)) {
        if (ffwrapper.isVideo(packet)) {
            LOGD("got video packet");
            videoPackets.push_back(packet);
            if (++frameCount >= 500) {
                break;
            }
        } else if (ffwrapper.isAudio(packet)) {
            LOGD("got audio packet");
            audioPackets.push_back(packet);
        } else {
            LOGD("got other packet");
        }
    }


    list<AVFrame*> videoFrames;
    list<AVFrame*> audioFrames;
    
    for (auto& packet : videoPackets) {
        AVFrame* frame = NULL;
        int decoded = -1;
        bool status = ffwrapper.decodeVideo(packet, &frame, &decoded);
        LOGD("video packet.size=%d, decoded=%d", packet.size, decoded);
        FFWrapper::freePacket(packet);
        static bool scale = true;
        if (status) {
            if (scale) {
                ffwrapper.setVideoScale(frame, 800, 600, AV_PIX_FMT_RGB24);
                scale = false;  
            }
            uint8_t* dst_data = new uint8_t[800*600*3];
            int dst_linesize = 800*3;
            ffwrapper.scaleVideo(frame, &dst_data, &dst_linesize);
            videoFrames.push_back(frame);
            FILE* video_dst_file = fopen(argv[2], "ab+");
            fwrite(dst_data, 1, 800*600*3, video_dst_file);
            fclose(video_dst_file);
            delete dst_data;  
        }
    }

    for (auto& packet : audioPackets) {
        AVFrame* frame = NULL;
        int decoded = -1;
        bool status = ffwrapper.decodeAudio(packet, &frame, &decoded);
        LOGD("audio packet.size=%d, decoded=%d", packet.size, decoded);
        FFWrapper::freePacket(packet);
        if (status) {
            static bool resample = true;
            if (resample) {
                ffwrapper.setAudioResample(frame, AV_CH_LAYOUT_STEREO, frame->sample_rate, AV_SAMPLE_FMT_S16);
                resample = false;    
            }
            uint8_t* dst_data = new uint8_t[2 * 2 * frame->nb_samples];
            int dst_samples = frame->nb_samples;
            ffwrapper.resampleAudio(frame, &dst_data, dst_samples);
            audioFrames.push_back(frame);
            FILE* audio_dst_file = fopen(argv[3], "ab+");
            fwrite(dst_data, 1, 2 * 2 * frame->nb_samples, audio_dst_file);
            fclose(audio_dst_file);
            delete dst_data;
        }
    }
    sleep(3);

    for (auto& frame: audioFrames) {
        FFWrapper::freeFrame(frame);
    }
    for (auto& frame: videoFrames) {
        FFWrapper::freeFrame(frame);
    }

    sleep(3);
    LOGD("ffplay -f rawvideo -pix_fmt rgb24 -video_size 800x600 %s", argv[2]);
    LOGD("ffplay -f s16le -ac 2 -ar %d %s\n", ffwrapper.audioSampleRate(), argv[3]);
    return 0;
}