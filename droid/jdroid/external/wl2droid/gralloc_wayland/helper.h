#ifndef GRALLOC_HELPER_H_
#define GRALLOC_HELPER_H_

#include <unistd.h>

enum {
	NUM_BUILTIN_DISPLAYS = 2,
    NUM_DISPLAY_BUFFERS  = 2
};

enum {
	FORMAT_RGBX8888  = 0x01
};


enum {
	BUFFER_IDLE      = 0x01,
	BUFFER_DRAWING   = 0x02,
	BUFFER_SCANING   = 0x04,
	BUFFER_SHOWN     = 0x08,
	BUFFER_MASK      = 0x0f
};

enum {
	DISPLAY_PRIMARY  = 0x10,
	DISPLAY_EXTERNAL = 0x20,
	DISPLAY_MASK     = 0x30
};


struct display_attributes_t {
	uint32_t  width;
    uint32_t  height;
	uint32_t  stride;
	int format;
	float xdpi;
    float ydpi;
    float fps;
};

inline int getBufferState(int bufferIndex, const int* bufferStates) {
	return bufferStates[bufferIndex] & BUFFER_MASK;
}

inline void setBufferState(int bufferIndex, int state, int* bufferStates) {
	bufferStates[bufferIndex] = (bufferStates[bufferIndex] & (~BUFFER_MASK)) | state;
}


inline int getBufferDisplay(int bufferIndex, const int* bufferStates) {
	return bufferStates[bufferIndex] & DISPLAY_MASK;
}

inline void setBufferDisplay(int bufferIndex, int display, int* bufferStates) {
	bufferStates[bufferIndex] = (bufferStates[bufferIndex] & (~DISPLAY_MASK)) | display;
}

inline size_t roundUpToPageSize(size_t x) {
	const long pageSize = sysconf(_SC_PAGESIZE);
	return (x + (pageSize-1)) & ~(pageSize-1);
}

inline unsigned long now() {
    struct timeval t;
    t.tv_sec = t.tv_usec = 0;
    gettimeofday(&t, NULL);
    return t.tv_sec*1000 + t.tv_usec/1000;
}


#ifdef __cplusplus
extern "C" {
#endif

int setDisplayCount(int dispCount);
int getDisplayCount(int isWait);

int setDisplaysEnable();
int getDisplaysEnable(int isWait);

int setDisplayProperties(int display, int width, int height, int format);
int getDisplayProperties(int display, int* width, int* height, int* format);

void debugFrameCount(unsigned normal, int disply, const char* good, const char* bad);

int getSharedBuffer(int isCreate, int bufSize, int stateSize, int* outFd, void** outBuf);

int getSharedIndexs(int isWait, int display, const int* states, int numStates, 
		uint32_t* indexs, int numIndexs);


#ifdef __cplusplus
}
#endif 


#endif // GRALLOC_HELPER_H_

