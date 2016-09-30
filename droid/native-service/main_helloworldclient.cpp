#include <utils/Log.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include "IHelloWorldService.h"

#define LOG_TAG "main_helloworldclient"

using namespace android;

int main(int argc, char** argv) {
    LOGI("HelloWorldClient is starting now");
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> b;
    
    do {
        b = sm->getService(String16("android.app.IHelloWorldService"));
        if (b != 0) {
            break;
        }
        LOGI("HelloWorldService is not working, waiting...");
        usleep(500000);
    } while (true);
    
    sp<IHelloWorldService> helloworld = interface_cast<IHelloWorldService>(b);
    helloworld->helloWorld("Hao");
    return 0;
}
