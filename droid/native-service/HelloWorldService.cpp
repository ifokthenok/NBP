#include <utils/Log.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include "HelloWorldService.h"

namespace android {


void HelloWorldService::instantiate() {
    defaultServiceManager()->addService(String16("android.app.IHelloWorldService"), 
        new HelloWorldService());
}

status_t HelloWorldService::helloWorld(const char* msg) {
    LOGI("Hello World, %s\n", msg);
    printf("Hello World, %s\n", msg);
    return 0;
}

status_t HelloWorldService::onTransact(uint32_t code, const Parcel& data, 
    Parcel* reply, uint32_t flags) {
    return BnHelloWorldService::onTransact(code, data, reply, flags);
}

HelloWorldService::HelloWorldService() {
    LOGI("HelloWorldService is constructed.");
}

HelloWorldService::~HelloWorldService() {
    LOGI("HelloWorldService is destructed.");
}


}; // namespace android
