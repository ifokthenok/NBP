#ifndef ANDROID_NATIVEWINDOW_H
#define ANDROID_NATIVEWINDOW_H

#include <surfaceflinger/SurfaceComposerClient.h>

class ANativeWindow;

namespace android {

class SurfaceComposerClient;

class NativeWindowFactory {
public:
    enum WindowType {
        FRAME_BUFFER_WINDOW,
        SURFACE_FLINGER_WINDOW
    };
    
    sp<ANativeWindow> create(WindowType type = SURFACE_FLINGER_WINDOW);
    static NativeWindowFactory& instance();
private:
    NativeWindowFactory();
    NativeWindowFactory(const NativeWindowFactory&);
    sp<SurfaceComposerClient> mComposer;
    sp<SurfaceControl> mControl;
};


}; // namespace android



#endif // ANDROID_NATIVEWINDOW_H
