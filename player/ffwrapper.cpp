#include "log.h"
#include "ffwrapper.h"

FFWrapper::FFWrapper() 
    :
    formatContext(NULL),
    videoIndex(-1),
    videoCodecContext(NULL),
    videoFrame(NULL),
    videoScaleContext(NULL),
    audioIndex(-1),
    audioCodecContext(NULL),
    audioFrame(NULL),
    audioResampleContext(NULL) {
    // register all formats and codecs
    av_register_all();

    videoFrame = av_frame_alloc();
    audioFrame = av_frame_alloc();
}

FFWrapper::~FFWrapper() {
    av_frame_free(&videoFrame);
    av_frame_free(&audioFrame);
    if (videoScaleContext) {
        sws_freeContext(videoScaleContext);
    }
    if (audioResampleContext) {
        swr_free(&audioResampleContext);
    }
    close();
}


bool FFWrapper::readPacket(AVPacket& packet) {
    // initialize packet, set data to NULL, let the demuxer fill it
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    if (av_read_frame(formatContext, &packet) < 0) {
        LOGE("FFWrapper", "av_read_frame failed");
        return false;
    }
    LOGD("FFWrapper", "readPacket ok");
    return true;
}
void FFWrapper::freePacket(AVPacket& packet) {
    av_packet_unref(&packet);
    LOGD("FFWrapper", "freePacket ok");
}

bool FFWrapper::decodeVideo(const AVPacket& packet, AVFrame& outframe, int* decoded) {
    int got_frame = 0; 
    int ret = avcodec_decode_video2(videoCodecContext, videoFrame, &got_frame, &packet);
    if (ret < 0) {
        LOGE("FFWrapper", "avcodec_decode_video2 failed: %d", ret);
        return false; 
    }
    if (!got_frame) {
        LOGE("FFWrapper", "avcodec_decode_video2 can't got frame");
        return false; 
    }

    outframe = *videoFrame;
    if (decoded) {
       *decoded = ret;
    }

    LOGD("FFWrapper", "got video frame: pix_fmt=%s, video_size=%dx%d, pts=%.6g", 
        av_get_pix_fmt_name((AVPixelFormat)videoFrame->format), videoFrame->width, videoFrame->height,
        av_q2d(formatContext->streams[videoIndex]->time_base)*videoFrame->pts);
    return true;
}

bool FFWrapper::setVideoScale(int src_w, int src_h, AVPixelFormat src_pix_fmt,
    int dst_w, int dst_h, AVPixelFormat dst_pix_fmt) {
    if (videoScaleContext) {
        LOGW("FFWrapper", "videoScaleContext is not null, free it first");
        sws_freeContext(videoScaleContext);
    }
    videoScaleContext = sws_getContext(
        src_w, src_h, src_pix_fmt, 
        dst_w, dst_h, dst_pix_fmt, 
        SWS_BILINEAR, NULL, NULL, NULL);
    if (!videoScaleContext) {
        LOGE("FFWrapper", "sws_getContext failed: src_size=%dx%d, src_fmt=%s, dst_size=%dx%d, dst_fmt=%s",         
            src_w, src_h, av_get_pix_fmt_name((AVPixelFormat)src_pix_fmt), 
            dst_w, dst_h, av_get_pix_fmt_name((AVPixelFormat)dst_pix_fmt));
        return false;
    }  
    LOGD("FFWrapper", "sws_getContext succeed: src_size=%dx%d, src_fmt=%s, dst_size=%dx%d, dst_fmt=%s",         
        src_w, src_h, av_get_pix_fmt_name((AVPixelFormat)src_pix_fmt), 
        dst_w, dst_h, av_get_pix_fmt_name((AVPixelFormat)dst_pix_fmt)); 
    return true;
}

void FFWrapper::scaleVideo(const AVFrame& frame, uint8_t** dst_data, int* dst_linesize) {
    sws_scale(videoScaleContext, (const uint8_t* const*)frame.data, 
        frame.linesize, 0, frame.height, dst_data, dst_linesize);
}


bool FFWrapper::decodeAudio(const AVPacket& packet, AVFrame& outframe, int* decoded) {
    int got_frame = 0;
    int ret = avcodec_decode_audio4(audioCodecContext, audioFrame, &got_frame, &packet);
    if (ret < 0) {
        LOGE("FFWrapper", "avcodec_decode_audio4 failed: %d", ret);
        return false;
    }
    if (!got_frame) {
        LOGE("FFWrapper", "avcodec_decode_audio4 can't got frame");
        return false; 
    }

    outframe = *audioFrame;
    if (decoded) {
       *decoded = ret > packet.size ? packet.size : ret;
    }

    LOGD("FFWrapper", "got audio frame: channels=%d, nb_samples=%d, pts=%.6g", 
        audioFrame->channels, audioFrame->nb_samples,
        av_q2d(formatContext->streams[audioIndex]->time_base)*audioFrame->pts);
    return true;
}

bool FFWrapper::setAudioResample(int64_t src_ch_layout, int src_rate, AVSampleFormat src_sample_fmt,
    int64_t dst_ch_layout, int dst_rate, AVSampleFormat dst_sample_fmt) {
    if (audioResampleContext) {
        LOGW("FFWrapper", "audioResampleContext is not null, free it first");
        swr_free(&audioResampleContext);
    }

    audioResampleContext = swr_alloc();
    if (!audioResampleContext) {
        LOGE("FFWrapper", "swr_alloc failed");
        return false;
    }
    // set options
    av_opt_set_int(audioResampleContext, "in_channel_layout",       src_ch_layout, 0);
    av_opt_set_int(audioResampleContext, "in_sample_rate",          src_rate, 0);
    av_opt_set_sample_fmt(audioResampleContext, "in_sample_fmt",    src_sample_fmt, 0);
    av_opt_set_int(audioResampleContext, "out_channel_layout",      dst_ch_layout, 0);
    av_opt_set_int(audioResampleContext, "out_sample_rate",         dst_rate, 0);
    av_opt_set_sample_fmt(audioResampleContext, "out_sample_fmt",   dst_sample_fmt, 0);

    // initialize the resampling context
    if (swr_init(audioResampleContext) < 0) {
        LOGE("FFWrapper", "swr_init failed");
        swr_free(&audioResampleContext);
        return false;
    }
    return true;
}

void FFWrapper::resampleAudio(const AVFrame& frame, uint8_t** dst_data, int dst_samples) {
    swr_convert(audioResampleContext, 
        dst_data, dst_samples, 
        (const uint8_t**)frame.extended_data, frame.nb_samples);
}

bool FFWrapper::open(const char* url) {
    // open input file, and allocate format context
    if (avformat_open_input(&formatContext, url, NULL, NULL) < 0) {
        LOGE("FFWrapper", "avformat_open_input(url=%s) failed", url);
        return false;
    }

    // retrieve stream information
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGE("FFWrapper", "avformat_find_stream_info failed");
        return false;
    }

    // find video/audio stream
    videoIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    audioIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (videoIndex < 0 || audioIndex < 0) {
        LOGE("FFWrapper", "av_find_best_stream failed: videoIndex=%d, audioIndex=%d", 
            videoIndex, audioIndex);
        return false;
    }

    if (videoIndex >= 0) {
        // find decoder for video stream
        AVCodec* videoDecoder = avcodec_find_decoder(formatContext->streams[videoIndex]->codecpar->codec_id);
        if (!videoDecoder) {
            LOGE("FFWrapper", "avcodec_find_decoder(videoIndex=%d) failed", videoIndex);
            return false;
        }

        // allocate a codec context for the decoder
        videoCodecContext = avcodec_alloc_context3(videoDecoder);
        if (!videoCodecContext) {
            LOGE("FFWrapper", "avcodec_alloc_context3(videoDecoder=%p) failed", videoDecoder);
            return false; 
        }

        // copy codec parameters from input stream to output codec context
        if (avcodec_parameters_to_context(videoCodecContext, formatContext->streams[videoIndex]->codecpar) < 0) {
            LOGE("FFWrapper", "avcodec_parameters_to_context(videoIndex=%d) failed", videoIndex);
            return false;
        }

        if (avcodec_open2(videoCodecContext, videoDecoder, NULL) < 0) {
            LOGE("FFWrapper", "avcodec_open2(videoIndex=%d) failed", videoIndex);
            return false;
         }
    }

    if (audioIndex >= 0) {
        // find decoder for audio stream
        AVCodec* audioDecoder = avcodec_find_decoder(formatContext->streams[audioIndex]->codecpar->codec_id);
        if (!audioDecoder) {
            LOGE("FFWrapper", "avcodec_find_decoder(audioIndex=%d) failed", audioIndex);
            return false;
        }

        // allocate a codec context for the decoder
        audioCodecContext = avcodec_alloc_context3(audioDecoder);
        if (!audioCodecContext) {
            LOGE("FFWrapper", "avcodec_alloc_context3(audioDecoder=%p) failed", audioDecoder);
            return false; 
        }

        // copy codec parameters from input stream to output codec context
        if (avcodec_parameters_to_context(audioCodecContext, formatContext->streams[audioIndex]->codecpar) < 0) {
            LOGE("FFWrapper", "avcodec_parameters_to_context(audioIndex=%d) failed", audioIndex);
            return false;
        }

        if (avcodec_open2(audioCodecContext, audioDecoder, NULL) < 0) {
            LOGE("FFWrapper", "avcodec_open2(audioIndex=%d) failed", audioIndex);
            return false;
         }
    }

    // dump status    
    LOGI("FFWrapper", "open %s succeed. start_time=%.6g, duration=%.6g", 
        url, double(formatContext->start_time)/AV_TIME_BASE, double(formatContext->duration)/AV_TIME_BASE);

    if (videoCodecContext) {
        LOGI("FFWrapper", "video_index=%d, pix_fmt=%s, video_size=%dx%d, start_time=%.6g, frames=%llu, fps=%.6g",
            videoIndex, av_get_pix_fmt_name(videoCodecContext->pix_fmt),
            videoCodecContext->width, videoCodecContext->height,
            av_q2d(formatContext->streams[videoIndex]->time_base)*formatContext->streams[videoIndex]->start_time,
            formatContext->streams[videoIndex]->nb_frames,
            av_q2d(formatContext->streams[videoIndex]->avg_frame_rate));
    }
    if (audioCodecContext) {
        LOGI("FFWrapper", "audio_index=%d, sample_fmt=%s, is_planar=%d, channels=%d, sample_rate=%d",
            audioIndex, av_get_sample_fmt_name(audioCodecContext->sample_fmt), 
            av_sample_fmt_is_planar(audioCodecContext->sample_fmt),
            audioCodecContext->channels, audioCodecContext->sample_rate);
    }
    return true;
}

void FFWrapper::close() {
    if (videoCodecContext) {
        avcodec_free_context(&videoCodecContext);
        videoIndex = -1;
    }
    if (audioCodecContext) {
        avcodec_free_context(&audioCodecContext);
        audioIndex = -1;
    }
    if (formatContext) {
        avformat_close_input(&formatContext);
    }
}

bool FFWrapper::seek(int64_t timestamp) {
    // NOTES: timestamp is in AV_TIME_BASE units
    if (av_seek_frame(formatContext, -1, timestamp, 0) < 0) {
        LOGE("FFWrapper", "av_seek_frame(timestamp=%llu) failed", timestamp);
        return false;
    }
    return true;
}



