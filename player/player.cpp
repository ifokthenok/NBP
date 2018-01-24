#include "player.h"
#include "element.h"


int Player::setSource(const char* url) {
    return STATUS_FAILED;
}
int Player::prepare() {
    return STATUS_FAILED;
}
int Player::play() {
    return STATUS_FAILED;
}
int Player::pause() {
    return STATUS_FAILED;
}
int Player::resume() {
    return STATUS_FAILED;
}
int Player::seek(int64_t pos) {
    return STATUS_FAILED;
}
int Player::getState() {
    return STATUS_FAILED;
}
int Player::getPosition(int64_t& pos) {
    return STATUS_FAILED;
}
int Player::getDuration(int64_t& duration) {
    return STATUS_FAILED;
}
int Player::getMessage(int& messagedId, void*& messsage) {
    return STATUS_FAILED;
}