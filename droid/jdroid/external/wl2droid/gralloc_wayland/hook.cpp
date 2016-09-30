#include <time.h>
#include <cutils/log.h>

#include "gralloc_priv.h"
#include "helper.h"

void gralloc_lock_hook(int bufferIndex, int* bufferStates){
	int retryCount = 0;
	int state = getBufferState(bufferIndex, bufferStates);
	do {
		if (state == BUFFER_IDLE || state == BUFFER_SHOWN || state == BUFFER_DRAWING ) {
			setBufferState(bufferIndex, BUFFER_DRAWING, bufferStates);
			debugFrameCount(1, DISPLAY_MASK, "normal", "override");
			return;
		}
		usleep(3000);
		state = getBufferState(bufferIndex, bufferStates);

	} while (retryCount++ < 10);
	
	setBufferState(bufferIndex, BUFFER_DRAWING, bufferStates);
	debugFrameCount(0, DISPLAY_MASK, "normal", "override");
}

void gralloc_unlock_hook(int bufferIndex, int* bufferStates){
	// 
	// Do nothing
	// 
}

void fb_post_hook(int bufferIndex, int* bufferStates) {
	setBufferState(bufferIndex, BUFFER_SCANING, bufferStates);
}


