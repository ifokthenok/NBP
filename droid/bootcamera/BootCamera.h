#ifndef ANDROID_BOOTCAMERA_H
#define ANDROID_BOOTCAMERA_H

#include <utils/Errors.h>
#include <utils/StrongPointer.h>
#include <camera/CameraParameters.h>

class camera_module;
class camera_info;

namespace android {

class CameraHardwareInterface;

class BootCamera {
public:
    BootCamera();
    ~BootCamera();
    status_t getCameraCount();
    status_t getCameraInfo(int cameraId, camera_info* cameraInfo);
    status_t open(int cameraId);
    status_t close();
    status_t startPreview();
    status_t stopPreview();
    status_t setParameters(const CameraParameters &params);
    CameraParameters getParameters();
private:
    camera_module* mCameraModule;
    sp<CameraHardwareInterface> mCameraHardware;
    int mCameraCount;
    int mCameraId;
};

} // namespace android

#endif // ANDROID_BOOTCAMERA_H




