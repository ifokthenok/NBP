/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dlfcn.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include <cutils/log.h>
#include <cutils/properties.h>

#include "helper.h"
#include "gralloc_priv.h"

/*****************************************************************************/


struct fb_context_t {
    framebuffer_device_t  device;
};

/*****************************************************************************/

static int fb_setSwapInterval(struct framebuffer_device_t* dev,
            int interval)
{
    fb_context_t* ctx = (fb_context_t*)dev;
    if (interval < dev->minSwapInterval || interval > dev->maxSwapInterval)
        return -EINVAL;
    // FIXME: implement fb_setSwapInterval
    return 0;
}

static int fb_setUpdateRect(struct framebuffer_device_t* dev,
        int l, int t, int w, int h)
{
    if (((w|h) <= 0) || ((l|t)<0))
        return -EINVAL;
	// FIXME  implement fb_setUpdateRect
    return 0;
}

extern void fb_post_hook(int bufferIndex, int* bufferStates);

static void* externalBuffer = 0;

static int getExternalBufferFd(int size) {

	int fd = open("/tmp/externalbuffer", O_RDWR);
	if (fd < 0) {
		ALOGE("mapFrameBufferLocked(): can't open /tmp/externalbuffer");
		return -errno;
	}
	void* vaddr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (vaddr == MAP_FAILED) {
		ALOGE("Error mapping the framebuffer (%s)", strerror(errno));
		return -errno;
	}

	externalBuffer = vaddr;
	return fd;
}
static int fb_post(struct framebuffer_device_t* dev, buffer_handle_t buffer)
{
    if (private_handle_t::validate(buffer) < 0)
        return -EINVAL;

    private_handle_t const* hnd = reinterpret_cast<private_handle_t const*>(buffer);
    private_module_t* m = reinterpret_cast<private_module_t*>(dev->common.module);

    if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {
		// Hook fb_post()
		fb_post_hook(hnd->index, m->bufferStates);
    } 
    return 0;
}



static int fb_post_ext(struct framebuffer_device_t* dev, int display, buffer_handle_t buffer) {
    if (private_handle_t::validate(buffer) < 0)
        return -EINVAL;

	private_handle_t const* hnd = reinterpret_cast<private_handle_t const*>(buffer);
    private_module_t* m = reinterpret_cast<private_module_t*>(dev->common.module);

	if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {
		
		int oldDisplay = getBufferDisplay(hnd->index, m->bufferStates);
		int newDisplay = display > 0 ? DISPLAY_EXTERNAL : DISPLAY_PRIMARY;
		if (oldDisplay != newDisplay) {
			setBufferDisplay(hnd->index, newDisplay, m->bufferStates);
		}
		// Hook fb_post()
		fb_post_hook(hnd->index, m->bufferStates);
    }
	
	return 0;
}
	

/*****************************************************************************/
int mapFrameBufferLocked(struct private_module_t* module)
{
    // Already initialized...
    if (module->framebuffer) {
        return 0;
    }

	// Get display count (wait until the property available)
	const int isWait = 1;
	int displayCount = getDisplayCount(isWait);
	if (displayCount < 0) {
		return -1;
	}
	module->displayCount = displayCount;

	size_t fbSize = 0;
	for (int i=0; i < displayCount; i++) {
		int width = 0;
		int height = 0;
		int format = 0;
		int bpp = 4;
		
		// Get each display properties
		if (getDisplayProperties(i, &width, &height, &format) < 0) {
			return -1;
		}
		if (format == FORMAT_RGBX8888) {
			format = HAL_PIXEL_FORMAT_RGBX_8888;
			bpp = 4;
		}
		
		module->displays[i].width = width;
		module->displays[i].height = height;
		module->displays[i].format = format;
		module->displays[i].stride = width * bpp;
		module->displays[i].xdpi = 160.0f;
    	module->displays[i].ydpi = 160.0f;
    	module->displays[i].fps = 25.0f;

		fbSize += NUM_DISPLAY_BUFFERS * roundUpToPageSize(width*height*bpp);
	}

	// Open and map the shared buffer file
	int fd = -1;
	void* vaddr = 0;
	const int isCreate = 1;
	if (getSharedBuffer(isCreate, fbSize, displayCount*NUM_DISPLAY_BUFFERS*sizeof(int), &fd, &vaddr) < 0) {
		return -errno;
	}

	// Initialize gralloc module members
	module->framebuffer = new private_handle_t(dup(fd), fbSize, 0);
	module->framebuffer->base = intptr_t(vaddr);
    module->numBuffers = displayCount * NUM_DISPLAY_BUFFERS;
    module->bufferMask = 0;
	module->bufferStates = (int*)((char*)vaddr + fbSize);
	
	for (int i=0; i < displayCount; i++) {
		ALOGI(	
			"display %d properties:\n"
			"width        = %d px\n"
			"height       = %d px\n"
			"stride       = %d bytes\n"
			"xdpi         = %.2f\n"
			"ydpi         = %.2f\n"
			"refresh rate = %.2f Hz\n",
			i,
			module->displays[i].width,
			module->displays[i].height,
			module->displays[i].stride,
			module->displays[i].xdpi,
			module->displays[i].ydpi,
			module->displays[i].fps);
	}

	// Notify wl2droid the displays enabled
    return setDisplaysEnable();
}

static int mapFrameBuffer(struct private_module_t* module)
{
    pthread_mutex_lock(&module->lock);
    int err = mapFrameBufferLocked(module);
    pthread_mutex_unlock(&module->lock);
    return err;
}

/*****************************************************************************/

static int fb_close(struct hw_device_t *dev)
{
    fb_context_t* ctx = (fb_context_t*)dev;
    if (ctx) {
        free(ctx);
    }
    return 0;
}

int fb_device_open(hw_module_t const* module, const char* name,
        hw_device_t** device)
{
    int status = -EINVAL;
    if (!strcmp(name, GRALLOC_HARDWARE_FB0)) {
        fb_context_t *dev = (fb_context_t*)malloc(sizeof(*dev));
        memset(dev, 0, sizeof(*dev));

        // Initialize the procs
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close    = fb_close;
        dev->device.setSwapInterval = fb_setSwapInterval;
        dev->device.post            = fb_post;
        dev->device.setUpdateRect = 0;
		dev->device.reserved_proc[0] = (void*)fb_post_ext;

        private_module_t* m = (private_module_t*)module;
        status = mapFrameBuffer(m);
        if (status >= 0) {        
            const_cast<uint32_t&>(dev->device.flags) = 0;
            const_cast<uint32_t&>(dev->device.width) = m->displays[0].width;
            const_cast<uint32_t&>(dev->device.height) = m->displays[0].height;
            const_cast<int&>(dev->device.stride) = m->displays[0].width;
            const_cast<int&>(dev->device.format) = m->displays[0].format;
            const_cast<float&>(dev->device.xdpi) = m->displays[0].xdpi;
            const_cast<float&>(dev->device.ydpi) = m->displays[0].ydpi;
            const_cast<float&>(dev->device.fps) = m->displays[0].fps;
            const_cast<int&>(dev->device.minSwapInterval) = 1;
            const_cast<int&>(dev->device.maxSwapInterval) = 1;
            *device = &dev->device.common;
        }
    }
    return status;
}
