#include "log.h"
#include "player.h"

int main(int argc, char* argv) {
    Player player;
    LOGD("player.play()=%d", player.play());
    return 0;
}