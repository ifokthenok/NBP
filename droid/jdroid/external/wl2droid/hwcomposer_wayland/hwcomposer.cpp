/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hardware/hardware.h>

#include <fcntl.h>
#include <errno.h>

#include <cutils/log.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>


#include <utils/StrongPointer.h>

#include <hardware/hwcomposer.h>
#include <hardware/fb.h>

#include <EGL/egl.h>
#include "vsync.h"
#include "helper.h"


using namespace android;


/*****************************************************************************/


typedef int (*fb_post_proc_t)(struct framebuffer_device_t* dev, int display, buffer_handle_t buffer);


struct displays_attributes_t{
	struct display {
		int width;
		int height;
		int vsyncPeriod;
		int dpiX;
		int dpiY;
	} displays[NUM_BUILTIN_DISPLAYS];
	int numDisplays;
};

struct hwc_context_t {
    hwc_composer_device_1_t device;
    //TODO: our private state goes below here 
    displays_attributes_t dispAttributes;
	framebuffer_device_t* fbDevice;
	sp<VSyncThread> vsyncThread[NUM_BUILTIN_DISPLAYS];
};

static void dump_layer(const hwc_layer_1_t* l, int layer) {
    ALOGD("layer=%d, type=%d, flags=%08x, handle=%p, transform=%02x, blend=%04x, {%d,%d,%d,%d}, {%d,%d,%d,%d}",
			layer,
			l->compositionType, l->flags, l->handle, l->transform, l->blending,
			l->sourceCrop.left,
			l->sourceCrop.top,
			l->sourceCrop.right,
			l->sourceCrop.bottom,
			l->displayFrame.left,
			l->displayFrame.top,
			l->displayFrame.right,
			l->displayFrame.bottom);
}

static void dump_displays(size_t numDisplays, hwc_display_contents_1_t** displays) {
	for(size_t i=0; i < numDisplays && displays[i]; i++) {
		ALOGD("display=%d flags=%d, numHwLayers=%d", 
			i, displays[i]->flags, displays[i]->numHwLayers);
		for(size_t j=0; j < displays[i]->numHwLayers; j++) {
			dump_layer(&displays[i]->hwLayers[j], j);
		}
	}
}

static int getDisplayAttributes(displays_attributes_t& dispAttributes) {

	int displayCount = getDisplayCount(1);
	if (displayCount < 0) {
		return -1;
	}
	dispAttributes.numDisplays = displayCount;
	
	int width, height, format;
	for (int i = 0; i < dispAttributes.numDisplays; i++) {
		if (getDisplayProperties(i, &width, &height, &format) < 0) {
			return -1;
		}
		dispAttributes.displays[i].width = width;
		dispAttributes.displays[i].height = height;
		dispAttributes.displays[i].vsyncPeriod = 40*1000*1000;
		dispAttributes.displays[i].dpiX = 160*1000;
		dispAttributes.displays[i].dpiY = 160*1000;
	}
	
	return 0;
}

static int loadFbHalModule(framebuffer_device_t** fbDevice) {
    hw_module_t const* module;
    int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
    if (err != 0) {
        ALOGE("%s module not found", GRALLOC_HARDWARE_MODULE_ID);
        return err;
    }
	
    return framebuffer_open(module, fbDevice);
}



/*****************************************************************************/
/*
 * (*prepare)() is called for each frame before composition and is used by
 * SurfaceFlinger to determine what composition steps the HWC can handle.
 *
 * (*prepare)() can be called more than once, the last call prevails.
 *
 * The HWC responds by setting the compositionType field in each layer to
 * either HWC_FRAMEBUFFER or HWC_OVERLAY. In the former case, the
 * composition for the layer is handled by SurfaceFlinger with OpenGL ES,
 * in the later case, the HWC will have to handle the layer's composition.
 * compositionType and hints are preserved between (*prepare)() calles
 * unless the HWC_GEOMETRY_CHANGED flag is set.
 *
 * (*prepare)() is called with HWC_GEOMETRY_CHANGED to indicate that the
 * list's geometry has changed, that is, when more than just the buffer's
 * handles have been updated. Typically this happens (but is not limited to)
 * when a window is added, removed, resized or moved. In this case
 * compositionType and hints are reset to their default value.
 *
 * For HWC 1.0, numDisplays will always be one, and displays[0] will be
 * non-NULL.
 *
 * For HWC 1.1, numDisplays will always be HWC_NUM_PHYSICAL_DISPLAY_TYPES.
 * Entries for unsupported or disabled/disconnected display types will be
 * NULL.
 *
 * In HWC 1.3, numDisplays may be up to HWC_NUM_DISPLAY_TYPES. The extra
 * entries correspond to enabled virtual displays, and will be non-NULL.
 *
 * returns: 0 on success. An negative error code on error. If an error is
 * returned, SurfaceFlinger will assume that none of the layer will be
 * handled by the HWC.
 */
static int hwc_prepare(hwc_composer_device_1 *dev,
				size_t numDisplays, hwc_display_contents_1_t** displays) {
	// ALOGI("hwc_prepare(): dev=%p, numDisplays=%d", dev, numDisplays);
	// dump_displays(numDisplays, displays);

	// hwc 1.0 
	if (dev->common.version == HWC_DEVICE_API_VERSION_1_0 
		&& (displays[0]->flags & HWC_GEOMETRY_CHANGED)) {
        for (size_t j=0 ; j < displays[0]->numHwLayers; j++) {
            displays[0]->hwLayers[j].compositionType = HWC_FRAMEBUFFER;
        }
		return 0;
	}

	// hwc 1.1 or later
	for(size_t i=0; displays && i < numDisplays; i++) {
        for (size_t j=0; displays[i] && j < displays[i]->numHwLayers; j++) {
			if (displays[i]->hwLayers[j].compositionType == HWC_FRAMEBUFFER_TARGET) {
				/*
				* HWC_FRAMEBUFFER_TARGET
				*   Always set by the caller before calling (*prepare)(), this value
				*   indicates this layer is the framebuffer surface used as the target of
				*   OpenGL ES composition. If the HWC sets all other layers to HWC_OVERLAY
				*   or HWC_BACKGROUND, then no OpenGL ES composition will be done, and
				*   this layer should be ignored during set().
				*
				*   This flag (and the framebuffer surface layer) will only be used if the
				*   HWC version is HWC_DEVICE_API_VERSION_1_1 or higher. In older versions,
				*   the OpenGL ES target surface is communicated by the (dpy, sur) fields
				*   in hwc_compositor_device_1_t.
				*
				*   This value cannot be set by the HWC implementation.
				*/ 
			} else if (displays[i]->flags & HWC_GEOMETRY_CHANGED) {
				displays[i]->hwLayers[j].compositionType = HWC_FRAMEBUFFER;
			} else {
				displays[i]->hwLayers[j].compositionType = HWC_FRAMEBUFFER;
			}
        }
	}
    return 0;
}

/*
 * (*set)() is used in place of eglSwapBuffers(), and assumes the same
 * functionality, except it also commits the work list atomically with
 * the actual eglSwapBuffers().
 *
 * The layer lists are guaranteed to be the same as the ones returned from
 * the last call to (*prepare)().
 *
 * When this call returns the caller assumes that the displays will be
 * updated in the near future with the content of their work lists, without
 * artifacts during the transition from the previous frame.
 *
 * A display with zero layers indicates that the entire composition has
 * been handled by SurfaceFlinger with OpenGL ES. In this case, (*set)()
 * behaves just like eglSwapBuffers().
 *
 * For HWC 1.0, numDisplays will always be one, and displays[0] will be
 * non-NULL.
 *
 * For HWC 1.1, numDisplays will always be HWC_NUM_PHYSICAL_DISPLAY_TYPES.
 * Entries for unsupported or disabled/disconnected display types will be
 * NULL.
 *
 * In HWC 1.3, numDisplays may be up to HWC_NUM_DISPLAY_TYPES. The extra
 * entries correspond to enabled virtual displays, and will be non-NULL.
 *
 * IMPORTANT NOTE: There is an implicit layer containing opaque black
 * pixels behind all the layers in the list. It is the responsibility of
 * the hwcomposer module to make sure black pixels are output (or blended
 * from).
 *
 * IMPORTANT NOTE: In the event of an error this call *MUST* still cause
 * any fences returned in the previous call to set to eventually become
 * signaled.  The caller may have already issued wait commands on these
 * fences, and having set return without causing those fences to signal
 * will likely result in a deadlock.
 *
 * returns: 0 on success. A negative error code on error:
 *	  HWC_EGL_ERROR: eglGetError() will provide the proper error code (only
 *		  allowed prior to HWComposer 1.1)
 *	  Another code for non EGL errors.
 */
static int hwc_set(hwc_composer_device_1* dev,
			size_t numDisplays, hwc_display_contents_1_t** displays) {
	// ALOGI("hwc_set(): dev=%p, numDisplays=%d", dev, numDisplays);
	// dump_displays(numDisplays, displays);

	// hwc 1. 0, we only handle the first display composition.
	if (dev->common.version == HWC_DEVICE_API_VERSION_1_0) {
		EGLBoolean sucess = eglSwapBuffers((EGLDisplay)displays[0]->dpy,
				(EGLSurface)displays[0]->sur);
		if (!sucess) {
			return HWC_EGL_ERROR;
		}

		return 0;
	} 

	// hwc 1.1 or latter, we handle all display compositions
	hwc_context_t* ctx = (hwc_context_t*)dev;
	if (!ctx->fbDevice) {
		loadFbHalModule(&ctx->fbDevice);
	}
		
	for(size_t i=0; displays && i < numDisplays; i++) {

		if (i >= HWC_DISPLAY_VIRTUAL) {
			continue;
		}
		
		for (size_t j=0 ; displays[i] && j < displays[i]->numHwLayers ; j++) {
			if (displays[i]->hwLayers[j].compositionType == HWC_FRAMEBUFFER_TARGET) {
				if (ctx->fbDevice) {
					//ALOGI("display=%d, layer=%d, post handle=%p", 
					//	i, j, displays[i]->hwLayers[j].handle);
					fb_post_proc_t fbPost = (fb_post_proc_t)ctx->fbDevice->reserved_proc[0];
					fbPost(ctx->fbDevice, i, displays[i]->hwLayers[j].handle);
				}
			} 
		}
	}
    return 0;
}

/*
 * eventControl(..., event, enabled)
 * Enables or disables h/w composer events for a display.
 *
 * eventControl can be called from any thread and takes effect
 * immediately.
 *
 *	Supported events are:
 *		HWC_EVENT_VSYNC
 *
 * returns -EINVAL if the "event" parameter is not one of the value above
 * or if the "enabled" parameter is not 0 or 1.
 */
static int hwc_eventControl(hwc_composer_device_1* dev, int disp, int event, int enabled) {
	ALOGI("hwc_eventControl(): dev=%p, disp=%d, event=%d, enabled=%d", dev, disp, event, enabled);

	hwc_context_t* ctx = (hwc_context_t*)dev;
	if (disp >= ctx->dispAttributes.numDisplays) {
		return -1;
	}
	
	if (event != HWC_EVENT_VSYNC) {
	    return -EINVAL;
	}

	if (ctx->vsyncThread[disp] != NULL) {
		ctx->vsyncThread[disp]->setEnabled(enabled);
	}

	return 0;
}

/*
 * blank(..., blank)
 * Blanks or unblanks a display's screen.
 *
 * Turns the screen off when blank is nonzero, on when blank is zero.
 * Multiple sequential calls with the same blank value must be supported.
 * The screen state transition must be be complete when the function
 * returns.
 *
 * returns 0 on success, negative on error.
 */
static int hwc_blank(hwc_composer_device_1* dev, int disp, int blank) {
	ALOGI("hwc_blank(): dev=%p, disp=%d, blank=%d", dev, disp, blank);

	hwc_context_t* ctx = (hwc_context_t*)dev;
	if (disp >= ctx->dispAttributes.numDisplays) {
		return -1;
	}
	
	return 0;
}

/*
 * Used to retrieve information about the h/w composer
 *
 * Returns 0 on success or -errno on error.
 */
static int hwc_query(hwc_composer_device_1* dev, int what, int* value) {

	ALOGI("hwc_query(): dev=%p, what=%d", dev, what);
	
	/*
     * Must return 1 if the background layer is supported, 0 otherwise.
     */
    int rv;
    switch (what) {
	case HWC_BACKGROUND_LAYER_SUPPORTED :
		rv = 0;
		break;

    /*
     * Returns the vsync period in nanoseconds.
     *
     * This query is not used for HWC_DEVICE_API_VERSION_1_1 and later.
     * Instead, the per-display attribute HWC_DISPLAY_VSYNC_PERIOD is used.
     */
    case HWC_VSYNC_PERIOD :
		rv = 40 * 1000 * 1000;
		break;

    /*
     * Availability: HWC_DEVICE_API_VERSION_1_1
     * Returns a mask of supported display types.
     */
    case HWC_DISPLAY_TYPES_SUPPORTED :
		rv = HWC_DISPLAY_PRIMARY_BIT | HWC_DISPLAY_EXTERNAL_BIT;
		break;
		
	default:
		return -errno;
    }

	if (value) {
		*value = rv;
	}
	
	return 0;
}

/*
 * (*registerProcs)() registers callbacks that the h/w composer HAL can
 * later use. It will be called immediately after the composer device is
 * opened with non-NULL procs. It is FORBIDDEN to call any of the callbacks
 * from within registerProcs(). registerProcs() must save the hwc_procs_t
 * pointer which is needed when calling a registered callback.
 */
static void hwc_registerProcs(hwc_composer_device_1* dev, const hwc_procs_t* procs) {
	ALOGI("hwc_registerProcs(): dev=%p, procs=%p", dev, procs);

	hwc_context_t* ctx = (hwc_context_t*)dev;
	for (int disp=0; disp < ctx->dispAttributes.numDisplays; disp++) {
		if (ctx->vsyncThread[disp] == NULL) {
			ctx->vsyncThread[disp] = new VSyncThread(disp, ctx->dispAttributes.displays[disp].vsyncPeriod);
			ctx->vsyncThread[disp]->registerHwcProcs(procs);
		}
	}
}

/*
 * This field is OPTIONAL and can be NULL.
 *
 * If non NULL it will be called by SurfaceFlinger on dumpsys
 */
static void hwc_dump(hwc_composer_device_1* dev, char* buff, int buff_len) {
	ALOGI("hwc_dump(): dev=%p", dev);

	// TODO: add some dump infos
}

/*
 * (*getDisplayConfigs)() returns handles for the configurations available
 * on the connected display. These handles must remain valid as long as the
 * display is connected.
 *
 * Configuration handles are written to configs. The number of entries
 * allocated by the caller is passed in *numConfigs; getDisplayConfigs must
 * not try to write more than this number of config handles. On return, the
 * total number of configurations available for the display is returned in
 * *numConfigs. If *numConfigs is zero on entry, then configs may be NULL.
 *
 * HWC_DEVICE_API_VERSION_1_1 does not provide a way to choose a config.
 * For displays that support multiple configurations, the h/w composer
 * implementation should choose one and report it as the first config in
 * the list. Reporting the not-chosen configs is not required.
 *
 * Returns 0 on success or -errno on error. If disp is a hotpluggable
 * display type and no display is connected, an error should be returned.
 *
 * This field is REQUIRED for HWC_DEVICE_API_VERSION_1_1 and later.
 * It should be NULL for previous versions.
 */
static int hwc_getDisplayConfigs(hwc_composer_device_1* dev, int disp,
		uint32_t* configs, size_t* numConfigs) {
	ALOGI("hwc_getDisplayConfigs(): dev=%p, disp=%d", dev, disp);
	
	// check the display is connected and the input arguments is valid
	hwc_context_t* ctx = (hwc_context_t*)dev;
	if (disp >= ctx->dispAttributes.numDisplays) {
		return -errno;
	}
	
	return 0;
}

/*
 * (*getDisplayAttributes)() returns attributes for a specific config of a
 * connected display. The config parameter is one of the config handles
 * returned by getDisplayConfigs.
 *
 * The list of attributes to return is provided in the attributes
 * parameter, terminated by HWC_DISPLAY_NO_ATTRIBUTE. The value for each
 * requested attribute is written in order to the values array. The
 * HWC_DISPLAY_NO_ATTRIBUTE attribute does not have a value, so the values
 * array will have one less value than the attributes array.
 *
 * This field is REQUIRED for HWC_DEVICE_API_VERSION_1_1 and later.
 * It should be NULL for previous versions.
 *
 * If disp is a hotpluggable display type and no display is connected,
 * or if config is not a valid configuration for the display, a negative
 * value should be returned.
 */
int static hwc_getDisplayAttributes(hwc_composer_device_1* dev, int disp,
		uint32_t config, const uint32_t* attributes, int32_t* values) {

	ALOGI("hwc_getDisplayAttributes(): dev=%p, disp=%d", dev, disp);

	hwc_context_t* ctx = (hwc_context_t*)dev;
	if (disp >= ctx->dispAttributes.numDisplays) {
		return -1;
	}

	for(int i=0; attributes[i] != HWC_DISPLAY_NO_ATTRIBUTE; i++) {
		switch(attributes[i]) {
			
	    /* The vsync period in nanoseconds */
	    case HWC_DISPLAY_VSYNC_PERIOD :
			values[i] = ctx->dispAttributes.displays[disp].vsyncPeriod;
			break;
			
	    /* The number of pixels in the horizontal and vertical directions. */
	    case HWC_DISPLAY_WIDTH :
			values[i] = ctx->dispAttributes.displays[disp].width;
			break;
	    case HWC_DISPLAY_HEIGHT:
			values[i] = ctx->dispAttributes.displays[disp].height;
			break;

	   /* The number of pixels per thousand inches of this configuration.
	     *
	     * Scaling DPI by 1000 allows it to be stored in an int without losing
	     * too much precision.
	     *
	     * If the DPI for a configuration is unavailable or the HWC implementation
	     * considers it unreliable, it should set these attributes to zero.
	     */
	    case HWC_DISPLAY_DPI_X:
			values[i] = ctx->dispAttributes.displays[disp].dpiX;
			break;
	    case HWC_DISPLAY_DPI_Y:
			values[i] = ctx->dispAttributes.displays[disp].dpiY;
	    	break;

		default:
			ALOGI("hwc_getDisplayAttributes(): unknown attridbute=%d", attributes[i]);	
			return -1;
		}
	}
	
	return 0;
}


/*****************************************************************************/


static int hwc_device_close(hw_device_t* dev)
{
    hwc_context_t* ctx = (hwc_context_t*)dev;
    if (ctx) {
        free(ctx);
    }
    return 0;
}

static int hwc_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
    int status = -EINVAL;
    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {
        hwc_context_t* dev = (hwc_context_t*)malloc(sizeof(*dev));
        memset(dev, 0, sizeof(*dev));
		for (int disp=0; disp < NUM_BUILTIN_DISPLAYS; disp++) {
			dev->vsyncThread[disp] = sp<VSyncThread>();
		}
		
        // initialize the procs
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = HWC_DEVICE_API_VERSION_1_3;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = hwc_device_close;
        dev->device.prepare = hwc_prepare;
        dev->device.set = hwc_set;
		dev->device.blank = hwc_blank;
		dev->device.eventControl = hwc_eventControl;
		dev->device.dump = hwc_dump;
		dev->device.getDisplayConfigs = hwc_getDisplayConfigs;
		dev->device.getDisplayAttributes = hwc_getDisplayAttributes;
		dev->device.query = hwc_query;
		dev->device.registerProcs = hwc_registerProcs;
		*device = &dev->device.common;

		// wait the displays available
		status = getDisplayAttributes(dev->dispAttributes);
    }
    return status;
}


static struct hw_module_methods_t hwc_module_methods = {
    .open = hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
    common: {
        tag: HARDWARE_MODULE_TAG,
        version_major: 1,
        version_minor: 0,
        id: HWC_HARDWARE_MODULE_ID,
        name: "wayland hwcomposer module",
        author: "The Android Open Source Project",
        methods: &hwc_module_methods,
    }
};

