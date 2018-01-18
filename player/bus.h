#pragma once

struct Bus {
    virtual int sendMessage(int messageId, void* message);
    virtual int getMessage(int& messagedId, void&* messsage);

}