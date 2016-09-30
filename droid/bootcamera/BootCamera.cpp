#include <utils/Log.h>
#include <hardware/camera.h>
#include <CameraHardwareInterface.h>
#include "BootCamera.h"
#include "NativeWindowFactory.h"

#undef  LOG_TAG
#define LOG_TAG "BootCamera"

namespace android {

BootCamera::BootCamera() 
    : mCameraModule(0), mCameraCount(0), mCameraId(-1) {
    if (hw_get_module(CAMERA_HARDWARE_MODULE_ID, (const hw_module_t**)&mCameraModule) < 0) {
        LOGE("hw_get_module(CAMERA_HARDWARE_MODULE_ID...) failed");
        return;
    } 
    
    mCameraCount = mCameraModule->get_number_of_cameras();
    if (!mCameraCount) {
        LOGE("No camera exist");
    }
}

BootCamera::~BootCamera() {
    if (mCameraHardware != 0) {
        mCameraHardware->release();
    }
}


int BootCamera::getCameraCount() {
    return mCameraCount;
}

status_t BootCamera::getCameraInfo(int cameraId, camera_info* cameraInfo) {
    return mCameraModule->get_camera_info(cameraId, cameraInfo);
}


int BootCamera::open(int cameraId) {
    if (cameraId < 0 || cameraId >= mCameraCount) {
        return BAD_INDEX;
    }
    
    if (mCameraId == cameraId) {
        return NO_ERROR;
    }
    
    if (mCameraHardware != 0) {
        LOGE("BootCamera::open() failed: close() must be called before calling this function");
        return INVALID_OPERATION;
    }
    
    char cameraName[16];
    snprintf(cameraName, sizeof(cameraName), "%d", cameraId);
    mCameraHardware = new CameraHardwareInterface(cameraName);
    if (mCameraHardware->initialize((hw_module_t*)mCameraModule)) {
        LOGE("BootCamera::open() failed: can't initialize camera hardware ");
        mCameraHardware.clear();
        return INVALID_OPERATION;
    }
    
    sp<ANativeWindow> wnd = NativeWindowFactory::instance().create(); 
    if (mCameraHardware->setPreviewWindow(wnd)) {
        LOGE("BootCamera::open() failed: can't set preview window");
        return INVALID_OPERATION;
    }
    mCameraId = cameraId;
    return NO_ERROR;
}


int BootCamera::close() {
    if (mCameraHardware != 0) {
        mCameraHardware->release();
        mCameraHardware.clear();
        mCameraId = -1;
        return NO_ERROR;
    }
    LOGE("BootCamera::close() failed: open(...) must be called before calling this function");
    return INVALID_OPERATION;
}


int BootCamera::startPreview() {
    return mCameraHardware != 0 ? mCameraHardware->startPreview() : INVALID_OPERATION;
}


int BootCamera::stopPreview() {
    if (mCameraHardware != 0) {
        mCameraHardware->stopPreview();
    }
    return NO_ERROR;
}

int BootCamera::setParameters(const CameraParameters& params) {
    if (mCameraHardware != 0) {
        return mCameraHardware->setParameters(params);
    }
    return NO_ERROR;
}

CameraParameters BootCamera::getParameters() {
    if (mCameraHardware != 0) {
        return mCameraHardware->getParameters();
    }
    return CameraParameters();
}


}; // namespace android

