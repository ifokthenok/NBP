#include "log.h"
#include "player.h"
#include "bus.h"
#include "demuxer.h"
#include "audio_decoder.h"
#include "video_decoder.h"

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

    int status = demuxer.toReady();
    LOGD("demuxer.toReady(): %d, state=%s", status, cstr(demuxer.getState()));

    status = demuxer.toPaused();
    LOGD("demuxer.toPaused(): %d, state=%s", status, cstr(demuxer.getState()));

    status = demuxer.toReady();
    LOGD("demuxer.toReady(): %d, state=%s", status, cstr(demuxer.getState()));

    status = demuxer.toIdle();
    LOGD("demuxer.toIdle(): %d, state=%s", status, cstr(demuxer.getState()));
    return 0;
}