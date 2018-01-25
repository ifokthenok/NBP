#include "log.h"
#include "player.h"
#include "bus.h"
#include "demuxer.h"
#include "audio_decoder.h"
#include "video_decoder.h"

#undef  LOG_TAG 
#define LOG_TAG "main"

int main(int argc, char* argv[]) {
    Player player;
    LOGD("player.play()=%d", player.play());

    Message msg;
    Message msg2(1);
    Message msg3(2, &msg);
    Bus bus;
    bus.sendMessage(msg);
    bus.sendMessage(msg2);
    bus.sendMessage(msg3);

    Message omsg;
    bus.getMessage(omsg);
    LOGD("msg={id=%d, data=%p}", omsg.id, omsg.data);
    bus.getMessage(omsg);
    LOGD("msg={id=%d, data=%p}", omsg.id, omsg.data);
    bus.getMessage(omsg);
    LOGD("msg={id=%d, data=%p}", omsg.id, omsg.data);


    FFWrapper ffWrapper;
    Demuxer demuxer;

    AudioDecoder audioDecoder;
    audioDecoder.setEngine(&ffWrapper);
    audioDecoder.setSource(&demuxer);

    VideoDecoder videoDecoder;
    videoDecoder.setEngine(&ffWrapper);
    videoDecoder.setSource(&demuxer);

    demuxer.setEngine(&ffWrapper);
    demuxer.setSource(argv[1]);
    demuxer.setAudioSink(&audioDecoder);
    demuxer.setVideoSink(&videoDecoder);

    int status = demuxer.setState(READY);
    LOGD("demuxer.setState(): %d, state=%s", status, cstr(demuxer.getState()));
    status = demuxer.setState(PAUSED);
    LOGD("demuxer.setState(): %d, state=%s", status, cstr(demuxer.getState()));
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    status = demuxer.setState(PLAYING);
    LOGD("demuxer.setState(): %d, state=%s", status, cstr(demuxer.getState()));
    status = demuxer.setState(PAUSED);
    LOGD("demuxer.setState(): %d, state=%s", status, cstr(demuxer.getState()));
    status = demuxer.setState(READY);
    LOGD("demuxer.setState(): %d, state=%s", status, cstr(demuxer.getState()));
    status = demuxer.setState(IDLE);
    LOGD("demuxer.setState(): %d, state=%s", status, cstr(demuxer.getState()));
    return 0;
}