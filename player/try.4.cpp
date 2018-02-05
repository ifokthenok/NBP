#include "log.h"
#include "player.h"
#include "bus.h"
#include "demuxer.h"
#include "audio_decoder.h"
#include "audio_render.h"
#include "video_decoder.h"
#include "video_render.h"

#undef  LOG_TAG 
#define LOG_TAG "main"

int main(int argc, char* argv[]) {
    Player player;
    LOGD("player.play()=%d", player.play());

    FFWrapper ffWrapper;
    Demuxer demuxer;

    AudioDecoder audioDecoder;
    audioDecoder.setEngine(&ffWrapper);
    audioDecoder.setSource(&demuxer);

    AudioRender audioRender;
    audioRender.setEngine(&ffWrapper);
    audioRender.setSource(&audioDecoder);
    audioDecoder.setAudioSink(&audioRender);

    VideoDecoder videoDecoder;
    videoDecoder.setEngine(&ffWrapper);
    videoDecoder.setSource(&demuxer);

    VideoRender videoRender;
    videoRender.setEngine(&ffWrapper);
    videoRender.setSource(&videoDecoder);
    videoDecoder.setVideoSink(&videoRender);

    demuxer.setEngine(&ffWrapper);
    demuxer.setSource(argv[1]);
    demuxer.setAudioSink(&audioDecoder);
    demuxer.setVideoSink(&videoDecoder);

    int status = demuxer.setState(READY);
    LOGD("demuxer.setState(): %d, state=%s", status, cstr(demuxer.getState()));
    status = videoDecoder.setState(READY);
    LOGD("videoDecoder.setState(): %d, state=%s", status, cstr(videoDecoder.getState()));
    status = audioDecoder.setState(READY);
    LOGD("audioDecoder.setState(): %d, state=%s", status, cstr(audioDecoder.getState()));
    status = audioRender.setState(READY);
    LOGD("audioRender.setState(): %d, state=%s", status, cstr(audioRender.getState()));

    status = demuxer.setState(PAUSED);
    LOGD("demuxer.setState(): %d, state=%s", status, cstr(demuxer.getState()));
    status = videoDecoder.setState(PAUSED);
    LOGD("videoDecoder.setState(): %d, state=%s", status, cstr(videoDecoder.getState()));
    status = audioDecoder.setState(PAUSED);
    LOGD("audioDecoder.setState(): %d, state=%s", status, cstr(audioDecoder.getState()));
    status = audioRender.setState(PAUSED);
    LOGD("audioRender.setState(): %d, state=%s", status, cstr(audioRender.getState()));

    status = demuxer.setState(PLAYING);
    LOGD("demuxer.setState(): %d, state=%s", status, cstr(demuxer.getState()));
    status = videoDecoder.setState(PLAYING);
    LOGD("videoDecoder.setState(): %d, state=%s", status, cstr(videoDecoder.getState()));
    status = audioDecoder.setState(PLAYING);
    LOGD("audioDecoder.setState(): %d, state=%s", status, cstr(audioDecoder.getState()));
    status = audioRender.setState(PLAYING);
    LOGD("audioRender.setState(): %d, state=%s", status, cstr(audioRender.getState()));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    status = demuxer.setState(PAUSED);
    LOGD("demuxer.setState(): %d, state=%s", status, cstr(demuxer.getState()));
    status = videoDecoder.setState(PAUSED);
    LOGD("videoDecoder.setState(): %d, state=%s", status, cstr(videoDecoder.getState()));
    status = audioDecoder.setState(PAUSED);
    LOGD("audioDecoder.setState(): %d, state=%s", status, cstr(audioDecoder.getState()));
    status = audioRender.setState(PAUSED);
    LOGD("audioRender.setState(): %d, state=%s", status, cstr(audioRender.getState()));

    status = demuxer.setState(READY);
    LOGD("demuxer.setState(): %d, state=%s", status, cstr(demuxer.getState()));
    status = videoDecoder.setState(READY);
    LOGD("videoDecoder.setState(): %d, state=%s", status, cstr(videoDecoder.getState()));
    status = audioDecoder.setState(READY);
    LOGD("audioDecoder.setState(): %d, state=%s", status, cstr(audioDecoder.getState()));
    status = audioRender.setState(READY);
    LOGD("audioRender.setState(): %d, state=%s", status, cstr(audioRender.getState()));

    status = demuxer.setState(IDLE);
    LOGD("demuxer.setState(): %d, state=%s", status, cstr(demuxer.getState()));
    status = videoDecoder.setState(IDLE);
    LOGD("videoDecoder.setState(): %d, state=%s", status, cstr(videoDecoder.getState()));
    status = audioDecoder.setState(IDLE);
    LOGD("audioDecoder.setState(): %d, state=%s", status, cstr(audioDecoder.getState()));
    status = audioRender.setState(IDLE);
    LOGD("audioRender.setState(): %d, state=%s", status, cstr(audioRender.getState()));
    return 0;
}