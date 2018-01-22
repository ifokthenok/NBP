#include "log.h"
#include "player.h"
#include "bus.h"
#include "demuxer.h"

int main(int argc, char* argv) {
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

    Demuxer demuxer;


    return 0;
}