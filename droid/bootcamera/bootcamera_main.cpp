#include <utils/Log.h>
/*
#include <ui/FramebufferNativeWindow.h>
#include <hardware/camera.h>
#include <CameraHardwareInterface.h>
*/

#include <hardware/camera.h>
#include "BootCamera.h"

#undef  LOG_TAG
#define LOG_TAG "bootcamera_main"


using namespace android;

int main(int argc, char** argv)
{
    BootCamera* camera = new BootCamera();
    int cameraCount = camera->getCameraCount();
    if (cameraCount > 0) {
        int i;
        for(i=0; i < cameraCount; i++) {
            camera_info cameraInfo;
            camera->getCameraInfo(i, &cameraInfo);
            if (cameraInfo.facing == CAMERA_FACING_BACK) {
                break;
            }
        }
        if (i == cameraCount) {
            LOGE("No back facing camera exist, try to open the first front facing camera");
            i = 0;
        }
        camera->open(i);
        camera->startPreview();
        CameraParameters parameters = camera->getParameters();
        
        LOGE("KEY_PREVIEW_SIZE=%s", parameters.get(
            CameraParameters::KEY_PREVIEW_SIZE));
        LOGE("KEY_SUPPORTED_PREVIEW_SIZES=%s", parameters.get(
            CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES));
        
        LOGE("KEY_PREVIEW_FPS_RANGE=%s", parameters.get(
            CameraParameters::KEY_PREVIEW_FPS_RANGE));
        LOGE("KEY_SUPPORTED_PREVIEW_FPS_RANGE=%s", parameters.get
            (CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE));
        
        LOGE("KEY_PREVIEW_FORMAT=%s", parameters.get(CameraParameters::KEY_PREVIEW_FORMAT));
        LOGE("KEY_SUPPORTED_PREVIEW_FORMATS=%s", parameters.get
            (CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS));
        
        LOGE("KEY_PREVIEW_FRAME_RATE=%s", parameters.get(
            CameraParameters::KEY_PREVIEW_FRAME_RATE));
        LOGE("KEY_SUPPORTED_PREVIEW_FRAME_RATES=%s", parameters.get(
            CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES));
        
        sleep(15);
        camera->stopPreview();
        camera->close();
    }
    delete camera;
    return 0;
}
