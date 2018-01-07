#include "log.h"
#include "ffwrapper.h"

int main(int argc, char* argv[]) {
    FFWrapper ffwrapper;
    ffwrapper.open(argv[1]);

    LOGD("FFWrapper", "container info: start_time=%.6g, duration=%.6g",
        ffwrapper.startTime()/double(AV_TIME_BASE),
        ffwrapper.duration()/double(AV_TIME_BASE));
    
    LOGD("FFWrapper", "video info: pix_fmt=%s, video_size=%dx%d, "
        "time_base=%.6g, start_time=%.6g, duration=%.6g, frames=%lld, fps=%.6g",
        ffwrapper.videoPixelFormat(),
        ffwrapper.videoWidth(),
        ffwrapper.videoHeight(),
        ffwrapper.videoTimeBase(),
        ffwrapper.videoTimeBase()*ffwrapper.videoStartTime(), 
        ffwrapper.videoTimeBase()*ffwrapper.videoDuration(),
        ffwrapper.videoFrames(), ffwrapper.videoFPS());
    
    LOGD("FFWrapper", "audio info: sample_fmt=%s, channels=%d, sample_rate=%d, "
        "time_base=%.6g, start_time=%.6g, duration=%.6g",
        ffwrapper.audioSampleFormat(),
        ffwrapper.audioChannels(),
        ffwrapper.audioSampleRate(),
        ffwrapper.audioTimeBase(),
        ffwrapper.audioTimeBase()*ffwrapper.audioStartTime(),
        ffwrapper.audioTimeBase()*ffwrapper.audioDuration());

    AVPacket packet;
    int count = 0;
    ffwrapper.seek(10*AV_TIME_BASE);
    while (ffwrapper.readPacket(packet)) {
        if (ffwrapper.isVideo(packet)) {
            LOGD("FFWrapper", "got %s packet",  ffwrapper.isVideo(packet) ? "video" : "audio");   AVFrame frame;
            int decoded = -1;
            ffwrapper.decodeVideo(packet, frame, &decoded);
            LOGD("FFWrapper", "video packet.size=%d, decoded=%d", packet.size, decoded);
            if (++count == 500) {
                ffwrapper.setVideoScale(frame, 800, 600, AV_PIX_FMT_RGB24);
                uint8_t* dst_data = new uint8_t[800*600*3];
                int dst_linesize = 800*3;
                ffwrapper.scaleVideo(frame, &dst_data, &dst_linesize);
                FILE* video_dst_file = fopen(argv[2], "wb");
                fwrite(dst_data, 1, 800*600*3, video_dst_file);
                fclose(video_dst_file);
                delete dst_data;
                LOGD("FFWrapper", "ffplay -f rawvideo -pix_fmt rgb24 -video_size 800x600 %s", argv[2]);
            }
        } else {
            AVFrame frame;
            int decoded = -1;
            bool status = ffwrapper.decodeAudio(packet, frame, &decoded);
            LOGD("FFWrapper", "audio packet.size=%d, decoded=%d", packet.size, decoded);
            if (count <= 500) {
                static bool resample = true;
                if (status && resample) {
                    ffwrapper.setAudioResample(frame, AV_CH_LAYOUT_STEREO, frame.sample_rate, AV_SAMPLE_FMT_S16);
                    resample = false;    
                }
                uint8_t* dst_data = new uint8_t[2 * 2 * frame.nb_samples];
                int dst_samples = frame.nb_samples;
                ffwrapper.resampleAudio(frame, &dst_data, dst_samples);
                FILE* audio_dst_file = fopen(argv[3], "ab+");
                fwrite(dst_data, 1, 2 * 2 * frame.nb_samples, audio_dst_file);
                fclose(audio_dst_file);
                delete dst_data;
            } 
            if (count >= 500) {
                LOGD("FFWrapper", "ffplay -f s16le -ac 2 -ar %d %s\n", frame.sample_rate, argv[3]);
                break;
            }
        }
        ffwrapper.freePacket(packet);
    }
    return 0;
}