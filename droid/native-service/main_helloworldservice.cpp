#include <utils/Log.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include "HelloWorldService.h"

#define LOG_TAG "main_helloworldservice"

using namespace android;

int main(int argc, char** argv) {
    HelloWorldService::instantiate();
    ProcessState::self()->startThreadPool();
    LOGI("HelloWorldService is starting now");
    IPCThreadState::self()->joinThreadPool();
    return 0;
}
