#include <stdlib.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include <cutils/log.h>
#include <cutils/properties.h>

#include "helper.h"


#define DISPLAY_PROPERTY(name) "persist.sys.wayland."#name

int setDisplayCount(int count) {
	char key[PROPERTY_KEY_MAX];
	char value[PROPERTY_VALUE_MAX];
	snprintf(key, sizeof(key), DISPLAY_PROPERTY(num));
	snprintf(value, sizeof(value), "%d", count);
	if (property_set(key, value) < 0) {
		ALOGE("setDisplayCount(): set %s=%s, failed", key, value);
		return -1;
	}
	return 0;
}

int getDisplayCount(int isWait) {
	int num = 0;
	char value[PROPERTY_VALUE_MAX];

	for (property_get(DISPLAY_PROPERTY(num), value, "0"), num = atoi(value); 
		!num && isWait;  
		property_get(DISPLAY_PROPERTY(num), value, "0"), num = atoi(value)) {
		ALOGD("getDisplayCount(): can't get property "DISPLAY_PROPERTY(num)", sleep 500 ms and retry");
		usleep(500000);
	}

	if (num < 0) {
		ALOGE("getDisplayCount(): "DISPLAY_PROPERTY(num)"=%s invalid", value);
		return num;
	}

	if (num > NUM_BUILTIN_DISPLAYS) {
		ALOGW("getDisplayCount(): "DISPLAY_PROPERTY(num)"=%s invalid, support at most %d displays", 
			value, NUM_BUILTIN_DISPLAYS);
		num = NUM_BUILTIN_DISPLAYS;
	}

	return num;
}


int setDisplaysEnable() {
	char key[PROPERTY_KEY_MAX];
	char value[PROPERTY_VALUE_MAX];
	snprintf(key, sizeof(key), DISPLAY_PROPERTY(enable));
	snprintf(value, sizeof(value), "1");
	if (property_set(key, value) < 0) {
		ALOGE("setDisplaysEnable(): set %s=%s, failed", key, value);
		return -1;
	}
	return 0;
}


int getDisplaysEnable(int isWait) {
	
	int enable = 0;
	char value[PROPERTY_VALUE_MAX];
	for (property_get(DISPLAY_PROPERTY(enable), value, "0"), enable = atoi(value); 
		!enable && isWait;  
		property_get(DISPLAY_PROPERTY(enable), value, "0"), enable = atoi(value)) {
		ALOGD("getDisplaysEnable(): can't get property "DISPLAY_PROPERTY(enable)", sleep 500 ms and retry");
		usleep(500000);
	}

	if (enable != 1) {
		ALOGE("getDisplaysEnable(): "DISPLAY_PROPERTY(enable)"=%s invalid", value);
	}
	
	return enable;
}

int setDisplayProperties(int display, int width, int height, int format) {
	char key[PROPERTY_KEY_MAX];
	char value[PROPERTY_VALUE_MAX];

	// Set width
	snprintf(key, sizeof(key), DISPLAY_PROPERTY(%d.width), display);
	snprintf(value, sizeof(value), "%d", width);
	if (property_set(key, value) < 0) {
		ALOGE("setDisplayProperties(): set "DISPLAY_PROPERTY(0.width)"=%s failed", value);
		return -1;
	}

	// Set height
	snprintf(key, sizeof(key), DISPLAY_PROPERTY(%d.height), display);
	snprintf(value, sizeof(value), "%d", height);
	if (property_set(key, value) < 0) {
		ALOGE("setDisplayProperties(): set "DISPLAY_PROPERTY(0.height)"=%s failed", value);
		return -1;
	}

	// Set format
	if (format != FORMAT_RGBX8888) {
		ALOGW("setDisplayProperties(): only support rgbx8888 format ");
	}
	snprintf(key, sizeof(key), DISPLAY_PROPERTY(%d.format), display);
	snprintf(value, sizeof(value), "rgbx8888");
	if (property_set(key, "rgbx8888") < 0) {
		ALOGE("setDisplayProperties(): set "DISPLAY_PROPERTY(0.format)"=%s failed", value);
		return -1;
	}

	return 0;
}


int getDisplayProperties(int display, int* width, int* height, int* format)
{
	char key[PROPERTY_KEY_MAX];
	char value[PROPERTY_VALUE_MAX];

	if (display < 0  || display >= NUM_BUILTIN_DISPLAYS || !width || !height || !format) {
		ALOGE("getDisplayProperties(): input arguments invalid");
		return -1;
	}

	// Obtain width from "sys.persist.wl_output.id.width" 
	snprintf(key, sizeof(key), DISPLAY_PROPERTY(%d.width), display);
	property_get(key, value, "0");
	*width = atoi(value);
	if (*width < 1  || *width > 1680) {
		ALOGE("getDisplayProperties(): %s=%s invalid", key, value);
		return -1;
	}

	// Obtain height from "sys.persist.wl_output.id.height" 
	snprintf(key, sizeof(key), DISPLAY_PROPERTY(%d.height), display);
	property_get(key, value, "0");
	*height = atoi(value);
	if (*height < 1 || *height > 1024) {
		ALOGE("getDisplayProperties(): %s=%s invalid", key, value);
		return -1;
	}

	// Obtain foramt and bpp from "sys.persist.wl_output.id.format" 
	snprintf(key, sizeof(key), DISPLAY_PROPERTY(%d.format), display);
	property_get(key, value, "0");
	if (!strncmp(value, "rgbx8888", sizeof(value))) {
		*format = FORMAT_RGBX8888;
	} else {
		ALOGE("getDisplayProperties(): %s=%s invalid (only support rgbx8888)", key, value);
		return -1;
	}
	
	return 0;
}

void debugFrameCount(unsigned normal, int disply, const char* good, const char* bad) {
	static unsigned prepared = 0;
	static unsigned normalCount;
	static unsigned forcibleCount;
	static unsigned long lastTime;
	if (normal && !prepared) {
		prepared = 1;
	}
	if (prepared) {
		if (normal) {
			normalCount++;
		} else {
			forcibleCount++;
		}
		// Output the debug info in least 5 seconds interval
		if (now() - lastTime > 5000) {
			ALOGD("%s_frame_count():\ttotal=%d,\t%s=%d,\t%s=%d", 
				disply == DISPLAY_PRIMARY ? "primary" : 
				disply == DISPLAY_EXTERNAL ? "external" : "display",
				normalCount+forcibleCount, good, normalCount, bad, forcibleCount);
			lastTime = now();
		}
	}
}


int getSharedBuffer(int isCreate, int bufSize, int stateSize, int* outFd, void** outBuf) {
	int fd = -1;
	void* mapBase = MAP_FAILED;
	if (isCreate) {
		fd = open("/tmp/backbuffer", O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	} else {
		fd = open("/tmp/backbuffer", O_RDWR);
	}
	
	if (fd < 0) {
		ALOGE("getSharedBuffer(): open /tmp/backbuffer failed: %s", strerror(errno));
		return -1;
	}
	
	if (isCreate && ftruncate(fd, bufSize + stateSize) < 0) {
		ALOGE("getSharedBuffer(): ftruncate() failed: %s", strerror(errno));
	}

	/// Map the buffer file to get the shared memory address
	mapBase = mmap(NULL, bufSize + stateSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (mapBase == MAP_FAILED) {
		ALOGE("getSharedBuffer(): mmap failed: %s", strerror(errno));
		close(fd);
		return -1 ;
	}

	if (isCreate) {
		memset(mapBase, 0, bufSize + stateSize);
	}

	if (outFd) {
		*outFd = fd;
	}

	if (outBuf) {
		*outBuf = mapBase;
	}
	
	return 0;
}


int getSharedIndexs(int isWait, int display, const int* states, int numStates, uint32_t* indexs, int numIndexs) {

	uint32_t bufIndexs[NUM_DISPLAY_BUFFERS], bufCount = 0;
	int i , displayType;
	
	if (display < 0  || display >= NUM_BUILTIN_DISPLAYS || !states
		|| !indexs || numIndexs != NUM_DISPLAY_BUFFERS)  {
		ALOGE("getSharedIndexs(): invalid input arguments");
		return -1;
	}

	displayType = display >  0 ?  DISPLAY_EXTERNAL : DISPLAY_PRIMARY;
	while (bufCount != NUM_DISPLAY_BUFFERS) {
		bufCount = 0;
		for (i = 0; i < numStates; i++) {
			if (displayType == getBufferDisplay(i, states)) {
				bufIndexs[bufCount] = i;
				bufCount++;
			}		
		}
		if (!isWait) {
			break;
		}
	}

	if (bufCount != NUM_DISPLAY_BUFFERS) {
		ALOGE("getSharedIndexs(): failed: only %d buffer availabe for display %d", bufCount, display);
		return -1;
	}

	for (i = 0; i < NUM_DISPLAY_BUFFERS; i++) {
		indexs[i] = bufIndexs[i];
	}
	
	return 0;
}


