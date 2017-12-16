#pragma once

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libavutil/imgutils.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

class FFWrapper {
public:
    FFWrapper();
    ~FFWrapper();
    bool open(const char* url);
    void close();
    bool seek(int64_t timestamp);
    bool readPacket(AVPacket& packet);
    void freePacket(AVPacket& packet);

    bool decodeVideo(const AVPacket& packet, AVFrame& frame, int* decoded = NULL);
    bool setVideoScale(int src_w, int src_h, AVPixelFormat src_pix_fmt,
                       int dst_w, int dst_h, AVPixelFormat dst_pix_fmt);
    void scaleVideo(const AVFrame& frame, uint8_t** dst_data, int* dst_linesize);

    bool decodeAudio(const AVPacket& packet, AVFrame& frame, int* decoded = NULL);
    bool setAudioResample(int64_t src_ch_layout, int src_rate, AVSampleFormat src_sample_fmt,
                                     int64_t dst_ch_layout, int dst_rate, AVSampleFormat dst_sample_fmt);

    void resampleAudio(const AVFrame& frame, uint8_t** dst_data, int dst_samples);

public:
    bool isVideo(const AVPacket& packet) {
        return packet.stream_index == videoIndex;
    }
    bool isAudio(const AVPacket& packet) {
        packet.stream_index == audioIndex;
    } 

private:
    
    AVFormatContext* formatContext;
    

    int videoIndex;
    AVCodecContext* videoCodecContext;
    AVFrame* videoFrame;
    SwsContext* videoScaleContext;

    int audioIndex;
    AVCodecContext* audioCodecContext;
    AVFrame* audioFrame;
    SwrContext* audioResampleContext;
};