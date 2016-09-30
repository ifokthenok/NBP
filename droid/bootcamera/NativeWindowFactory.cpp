#include <utils/Log.h>
#include <ui/FramebufferNativeWindow.h>
#include <ui/DisplayInfo.h>
#include "NativeWindowFactory.h"

#undef  LOG_TAG
#define LOG_TAG "NativeWindowFactory"

namespace android {

sp<ANativeWindow> NativeWindowFactory::create(WindowType type)
{
    if (type == FRAME_BUFFER_WINDOW) {
        return new FramebufferNativeWindow();
    }
    
    mComposer = new SurfaceComposerClient();
    DisplayInfo dispInfo = {0};
    mComposer->getDisplayInfo(0, &dispInfo);
    
    LOGE("DisplayInfo: w=%d, h=%d", dispInfo.w, dispInfo.h);
    
    mControl = mComposer->createSurface(
        0x7fffffff, dispInfo.w, dispInfo.h, PIXEL_FORMAT_RGB_565);
    
    SurfaceComposerClient::openGlobalTransaction();

    // You can set surface positon or size like the following
    // mControl->setPosition(x, y);
    // mControl->setSize(width, height);
    // mControl->setLayer(0x40000000);
    mControl->setLayer(0x7fffffff);
    SurfaceComposerClient::closeGlobalTransaction();
    
    return mControl->getSurface();
}

NativeWindowFactory::NativeWindowFactory() {
}

NativeWindowFactory& NativeWindowFactory::instance() {
    static NativeWindowFactory windowFactory;
    return windowFactory;
}


}; // namespace android
