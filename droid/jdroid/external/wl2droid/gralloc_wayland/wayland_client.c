#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include <cutils/log.h>
#include <cutils/properties.h>

#include <wayland-client.h>
#include "helper.h"


// Stores the wayland ouput data
struct Output {
	struct wl_output* output;
	int x;
	int y;
	uint32_t width;
	uint32_t height;
};

// Stores the data structures used for the window.
struct SWindowData {
   struct wl_display* display;				// The wayland display connection.
   struct wl_registry* registry;	   		// Registry for global wayland objects.
   struct wl_compositor* compositor;		// wl_compositor_create_surface() function is used from this interface.
   struct wl_shell* shell;					// wl_shell_get_shellSurface() function is used from this interface.
   struct wl_surface* surface;				// The wayland surface.
   struct wl_shell_surface* shellSurface; 	// wl_shell_surface_set_toplevel() function is used from this interface
   struct wl_event_queue* frameQueue;		// Used for receiving frame callback event

   struct wl_shm* shm;					   	// The shm interface.
   struct wl_callback* callback;			// The frame callback.
   struct wl_buffer* buffers[NUM_DISPLAY_BUFFERS];	// wayland object that represents the shared memory buffer containing the rendered window content.
   
   struct wl_seat* seat;
   struct wl_pointer* pointer;
   struct wl_touch* touch;
   
   struct Output outputs[NUM_BUILTIN_DISPLAYS];		// Note: support at most 2 wayland output objects.
   struct Output* currentOutput;					// Current output
   uint32_t outputCount;							// The valid ouput count in outputs array	

   struct display_attributes_t displays[NUM_BUILTIN_DISPLAYS];
   uint32_t currentDisplay;
   uint32_t displayCount;
   
   uint32_t formats;
   uint32_t width;
   uint32_t height;
   uint32_t bufferIndexs[NUM_DISPLAY_BUFFERS];
   int* bufferStates;
   int fullScreen;
};

extern const struct wl_pointer_listener pointer_listener;
extern const struct wl_touch_listener touch_listener;
static const struct wl_shm_listener shm_listener; 
static const struct wl_output_listener output_listener; 
static const struct wl_callback_listener callback_listener;

/** 
 * Retrieves the wayland compositor and shell.
 * @param data Structure that holds all the window data. 
 * @param registry The wayland registry.
 * @param id Identifier of the object.
 * @param interface Name of the interface.
 * @param version Wayland version of the object. 
 */
static void registry_global(void *data, struct wl_registry *registry, 
	uint32_t id, const char *interface, uint32_t version) {
	ALOGI("registry_global(): name=%d, interface=%s, version=%d", id, interface, version);
		
	struct SWindowData* window = data;
	if (strcmp(interface, "wl_compositor") == 0) {
		window->compositor = wl_registry_bind(registry, id, &wl_compositor_interface, version);
	} else if (strcmp(interface, "wl_shell") == 0) {
	  	window->shell = wl_registry_bind(registry, id, &wl_shell_interface, version);
	} else if (strcmp(interface, "wl_shm") == 0) {
	  	window->shm = wl_registry_bind(registry, id, &wl_shm_interface, version);
	  	wl_shm_add_listener(window->shm, &shm_listener, window);
	} else if (strcmp(interface, wl_output_interface.name) == 0 && window->outputCount < NUM_BUILTIN_DISPLAYS) {
		window->outputs[window->outputCount].output = wl_registry_bind(registry, id, &wl_output_interface, version);
		wl_output_add_listener(window->outputs[window->outputCount].output, &output_listener, window);
		window->outputCount++;
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
	  	window->seat = wl_registry_bind(registry, id, &wl_seat_interface, version);
	  	window->pointer = wl_seat_get_pointer(window->seat);
	  	wl_pointer_add_listener(window->pointer, &pointer_listener, NULL);

	  	window->touch = wl_seat_get_touch(window->seat);
	  	wl_touch_add_listener(window->touch, &touch_listener, NULL);
	}
}

/** 
 *Handle removal of global wayland objects.
 * @param data Structure that holds all the window data. 
 * @param registry The wayland registry.
 * @param id Identifier of the object. 
 */
static void registry_remove(void *data, struct wl_registry *registry, uint32_t id) { 
	ALOGI("registry_remove(): name=%d", id);
}

static const struct wl_registry_listener registry_listener = {
   .global = registry_global,
   .global_remove = registry_remove
};


/**
 * Callback to retrieve the available pixel formats.
 * Stores the format in the window structure.
 * @param data The window structure, that contains all the window data.
 * @param shm The wayland shm interface.
 * @param format Additional format, that is supported. 
 */
static void shm_format(void* data, struct wl_shm* shm, uint32_t format) {
	struct SWindowData* window = (struct SWindowData*)data;
	window->formats |= (1 << format);
	ALOGI("shm_format(): format=%d", format);
}

/// Listener for the shared memory callback, that retrieves the available pixel formats.
static const struct wl_shm_listener shm_listener = { 
	.format = shm_format 
};

static void output_geometry(void *data, struct wl_output *wl_output,
		 int32_t x, int32_t y,
		 int32_t physical_width, int32_t physical_height,
		 int32_t subpixel, const char *make, const char *model,
		 int32_t transform) {
	ALOGI("output_geometry(): output=%p, x=%d, y=%d, "
		"physical_width=%d, physical_height=%d, subpixel=%d, make=%s, model=%s",
		wl_output, x, y, physical_width, physical_height, subpixel, make, model);
	
	struct SWindowData* window = (struct SWindowData* )data;
	uint32_t i = 0;
	for ( ; i < window->outputCount; i++) {
		if (window->outputs[i].output == wl_output) {
			window->outputs[i].x = x;
			window->outputs[i].x = y;
			break;
		}
	}
}

static void output_mode(void *data, struct wl_output *wl_output,
		 uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
	ALOGI("output_mode(): output=%p, flags=%d, width=%d, height=%d, refresh=%d",
		wl_output, flags, width, height, refresh);
	
	struct SWindowData* window = (struct SWindowData* )data;
	uint32_t i = 0;
	for ( ; i < window->outputCount; i++) {
		if (window->outputs[i].output == wl_output && (flags & WL_OUTPUT_MODE_CURRENT)) {
			window->outputs[i].width = width;
			window->outputs[i].height = height;	
			break;
		}
	}
}

static void output_done(void *data, struct wl_output *wl_output) {
	 ALOGI("output_done(): output=%p", wl_output);
}

static void output_scale(void *data, struct wl_output *wl_output, int32_t factor) {
	ALOGI("output_scale(): output=%p, factor=%d", wl_output, factor);
}

static const struct wl_output_listener output_listener = {
	.geometry = output_geometry,
	.mode = output_mode,
	.done = output_done,
	.scale = output_scale
};

static void shell_surface_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial) {
   wl_shell_surface_pong(shell_surface, serial);
}

static void shell_surface_configure(void *data, struct wl_shell_surface *shell_surface, 
   uint32_t edges, int32_t width, int32_t height) {
   ALOGI("shell_surface_configure(): edges=%d, width=%d, height=%d", edges, width, height);
}

static const struct wl_shell_surface_listener shell_surface_listener = {
   .ping = shell_surface_ping,
   .configure = shell_surface_configure,
};


/**
 * Function, called by the frame listener callback to render one frame.
 * @param data Structure that holds all the window data.
 * @param callback The callback of this function call.
 * @param time Time in milliseconds. 
 */
static void onDrawn(void *data, struct wl_callback *callback, uint32_t time)
{
	struct SWindowData* window = (struct SWindowData*)data;
	int damageWidth, damageHeight;
	int b0State, b1State;
	int currentBuffer, displayType;
	static int lastBuffer;
	static int dummyFlag;
	
	/// Make sure, this function is not the first called and no dummy set
	if (!dummyFlag && callback != NULL && time != 0xffffffff ) {
		setBufferState(window->bufferIndexs[lastBuffer], BUFFER_SHOWN, window->bufferStates);
	}

	if (!dummyFlag && callback == NULL && time == 0xffffffff) {
		/// In first time calling the function, we should attach buffer to the surface even if
		/// there is no scanning buffer. After the buffer has been attched, we invalid
		/// the first frame and install the callback which will call this function again.
		currentBuffer = lastBuffer;
		damageWidth = window->width;
		damageHeight = window->height;
		dummyFlag = 1;
		displayType = getBufferDisplay(window->bufferIndexs[currentBuffer], window->bufferStates);
		debugFrameCount(0, displayType, "normal", "dummy");
	} else {
		/// Wait at least one buffer becoming Scanning state
		for(b0State=getBufferState(window->bufferIndexs[0], window->bufferStates), 
			b1State=getBufferState(window->bufferIndexs[1], window->bufferStates);
			b0State != BUFFER_SCANING && b1State != BUFFER_SCANING;
			b0State=getBufferState(window->bufferIndexs[0], window->bufferStates), 
			b1State=getBufferState(window->bufferIndexs[1], window->bufferStates)) {
			usleep(1000);
		}
		
		if (b0State != BUFFER_SCANING) {
			currentBuffer = 1;
		} else if (b1State != BUFFER_SCANING) {
			currentBuffer = 0;
		} else {
			currentBuffer = lastBuffer ? 0 : 1;
		}
		damageWidth = window->width;
		damageHeight = window->height;
		dummyFlag = 0;
		displayType = getBufferDisplay(window->bufferIndexs[currentBuffer], window->bufferStates);
		debugFrameCount(1, displayType, "normal", "dummy");
	}

	wl_surface_attach(window->surface, window->buffers[currentBuffer], 0, 0);
	lastBuffer = currentBuffer;
	
	/// Invalidate the full content of the surface.
	wl_surface_damage(window->surface, 0, 0, damageWidth, damageHeight);

	/// Destroy old callback and create new frame callback to render next frame.
	if (callback) {
		wl_callback_destroy(callback);
	}
	window->callback = wl_surface_frame(window->surface);
	/// Set the queue for receiving callback event
	wl_proxy_set_queue((struct wl_proxy *)window->callback, window->frameQueue);
	/// Set listener of callback upon finish of redraw.
	wl_callback_add_listener(window->callback, &callback_listener, window);

	/// Commit surface requests of attach, damage and frame.
	wl_surface_commit(window->surface);
}

/// Frame listener callback, that calls the redraw() method above.
static const struct wl_callback_listener callback_listener = { 
	.done = onDrawn 
};

 
/** 
  *  Creates wayland shared memory buffer objects.
  *  @param window The window structure, that contains all the window data.
  *  @return 0 - success, others - failed.
  */
static int createShmBuffer(struct SWindowData* window) {
	struct wl_shm_pool* pool;
	struct wl_buffer* buffer;
	uint32_t i, j, disp, width, height, stride, offset;
	int fd, bufSize, buffersSize, statesSize;
	void* buffers;
	int* states;
	int32_t offsets[NUM_DISPLAY_BUFFERS] = {0};

	/// Compute the shared buffer size and the related-states size
	statesSize = window->displayCount*NUM_DISPLAY_BUFFERS * sizeof(int);
	buffersSize = 0;
	for (i = 0; i < window->displayCount; i++) {
		bufSize = roundUpToPageSize(window->displays[i].width * window->displays[i].height * 4);
		buffersSize += NUM_DISPLAY_BUFFERS * bufSize;
	}

	/// Open the shared buffer file, get the buffers and related-states
	if (getSharedBuffer(0, buffersSize, statesSize, &fd, &buffers) < 0) {
		return -1;
	}
	window->bufferStates = (int*)((char*)buffers + buffersSize);

	/// Wait until the shared buffer indexs aviable
	if (getSharedIndexs(1, window->currentDisplay, window->bufferStates, 
		window->displayCount*NUM_DISPLAY_BUFFERS, 
		window->bufferIndexs, NUM_BUILTIN_DISPLAYS) < 0) {
		return -1;
	}

	/// Compute the buffer offsets for window->currentDisplay
	buffersSize = 0;
	for (i = 0; i < window->displayCount*NUM_DISPLAY_BUFFERS; i++) {
		disp = i / NUM_DISPLAY_BUFFERS;
		for (j = 0;  j < NUM_DISPLAY_BUFFERS; j++) {
			if (window->bufferIndexs[j] == i) {
				offsets[j] = buffersSize;
				break;
			}
		}
		buffersSize += roundUpToPageSize(window->displays[disp].width*window->displays[disp].height*4);
	}
	
	/// Create wayland shm pool and buffer objects
	pool = wl_shm_create_pool(window->shm, fd, buffersSize);
	for (i = 0; i < NUM_DISPLAY_BUFFERS; i++) {
		window->buffers[i] = wl_shm_pool_create_buffer(pool, offsets[i], 
			window->displays[window->currentDisplay].width, 
			window->displays[window->currentDisplay].height,
			window->displays[window->currentDisplay].stride,
		    window->displays[window->currentDisplay].format);
		if (window->buffers[i] == NULL) {
			ALOGE("createShmBuffer(): wl_shm_pool_create_buffer() failed: "
				"pool=%p, offset=%d, width=%d, height=%d, stride=%d, format=%d",
				pool, offsets[i],
				window->displays[window->currentDisplay].width, 
				window->displays[window->currentDisplay].height,
				window->displays[window->currentDisplay].stride,
				window->displays[window->currentDisplay].format);
			return -1;
		} 
	}

	/// Destroy the pool object and close the file discriptor
	wl_shm_pool_destroy(pool);
	close(fd);
	return 0;
}


static int getDisplaysData(struct SWindowData* window) {
	
	int displayCount, displayIndex;
	int width, height, format, bpp;
	const int isWait = 1;
	
	displayCount = getDisplayCount(isWait);
	if (displayCount < 0) {
		return -1;
	}
	window->displayCount = displayCount;
	for (displayIndex=0; displayIndex < displayCount; displayIndex++) {

		if (getDisplayProperties(displayIndex, &width, &height, &format) < 0) {
			return -1;
		}
		// Set the defalut bpp as 4 bytes. 
		bpp = 4;
		if (format== FORMAT_RGBX8888) {
			bpp = 4;
			format = WL_SHM_FORMAT_XRGB8888;
		}
		window->displays[displayIndex].width = width;
		window->displays[displayIndex].height = height;
		window->displays[displayIndex].stride = width * bpp;
		window->displays[displayIndex].format = format;
	}

	return 0;
}
	

static int selectOutput(struct SWindowData* window) {
	uint32_t op = 0;
	uint32_t disp = window->currentDisplay;

	if (!window->outputCount) {
		ALOGE("selectOutput(): there is no output");
		return -1;
	}

	// First, try to find the output met with (x,y) == (0, 0)
	for (op = 0; op < window->outputCount; op++) {
		if (window->outputs[op].x == 0 && window->outputs[op].y == 0) {
			break;
		}
	}
	// If not found,  use the first output
	if (op >= window->outputCount) {
		op = 0;
	}

	// If multi-displays and multi-outputs, use the other output
	if (disp > 0 && window->outputCount > 1) {
		op = !op;
	}

	// Here, we got the output
	window->currentOutput = &window->outputs[op];
		
	// Ajust the width and height and decide whether full-screen enabled.
	if (window->displays[disp].width > window->currentOutput->width 
		|| window->displays[disp].height > window->currentOutput->height) {
		ALOGE("selectOutput(): can't select a suitable width(or height) for the output"
			" request=[%d x %d] actual=[%d x %d]",
			window->displays[disp].width,
			window->displays[disp].height,
			window->currentOutput->width,
			window->currentOutput->height);
		return -1;
	}
	window->width = window->displays[disp].width;
	window->height = window->displays[disp].height;
	if (window->width == window->currentOutput->width 
		&& window->height == window->currentOutput->height) {
		window->fullScreen = 1;	
	}
	
	return 0;
}

/** 
 * Creates the wayland window.
 * @param display The android display whose buffer will be displayed 
 * @return The created window structure.
 */
static struct SWindowData* createWindow(int display) {
	/// Allocate memory
	struct SWindowData* window = malloc(sizeof(struct SWindowData));
	if (!window) {
		ALOGE("createWindow(): malloc() failed: %m");
		exit(-1); 
	}
	memset(window, 0, sizeof(struct SWindowData));
	window->currentDisplay = display;

	/// Get display properties
	if (getDisplaysData(window) < 0) {
		exit(-1);
	}

	/// Connect to wayland server
	window->display = wl_display_connect(NULL);
	if (!window->display) {
		ALOGE("createWindow(): wl_display_connect() error");
		exit(-1); 
	}
	/// Create registry to listen for globals.
	window->registry = wl_display_get_registry(window->display);
	/// Add listener to get the wayland compositor, shell and the shm interface.
	/// Installs a shm listener to retrieve the format.
	wl_registry_add_listener(window->registry, &registry_listener, window);
	/// Process connection events.
	wl_display_roundtrip(window->display);
	/// Ensure, that format is received by the shm listener.
	wl_display_roundtrip(window->display);
	/// Ensure, that output is received by the output listener.
	wl_display_roundtrip(window->display);
	if (!(window->formats & (1 << WL_SHM_FORMAT_XRGB8888))) {
		ALOGE("createWindow(): WL_SHM_FORMAT_XRGB8888 not available");
		exit(-1);
	}

	/// Select a suitable output
	if (selectOutput(window) < 0) {
		ALOGE("createWindow(): can't select a suitable output");
		exit(-1);
	}

    /// Wait gralloc to set displays enable
	if (getDisplaysEnable(1) < 0) {
		exit(-1);
	}

	/// Cretae the wayland share buffer objects
	if (createShmBuffer(window) < 0) {
		ALOGE("createWindow(): can't create share buffer");
		exit(-1);
	}

	/// Creates a wayland surface.
	window->surface = wl_compositor_create_surface(window->compositor);
	/// Gets the shell surface for the newly created wayland surface.
	window->shellSurface = wl_shell_get_shell_surface(window->shell, window->surface);
	// On alpine/TI, Need to add shell surface listerner,otherwise, the pointer event will never 
	// dispatched to wayland-client from the server, but need not to do this on conti platform
	wl_shell_surface_add_listener(window->shellSurface, &shell_surface_listener, 0);
	wl_shell_surface_set_title(window->shellSurface, "YgomiDroid");
	wl_shell_surface_set_toplevel(window->shellSurface);
	/// Set the shell surface fullscreen display
	if (window->fullScreen) {
		wl_shell_surface_set_fullscreen(window->shellSurface, 
			WL_SHELL_SURFACE_FULLSCREEN_METHOD_SCALE, 0, window->currentOutput->output);
	}
	
	/// Create this queue for receiving frame finish callback event.
	window->frameQueue = wl_display_create_queue(window->display);
	if (!window->frameQueue) {
		ALOGE("createWindow(): wl_display_create_queue() error");
		exit(-1);
	}
		
	return window;
}


/// Destory the wayland objects and free the window strcut memory 
static void destroyWindow(struct SWindowData *window)
{
	uint32_t i = 0;
	
	/// Destroy the frame callback event queue and the callback object
	wl_event_queue_destroy(window->frameQueue);
	if (window->callback) {
		wl_callback_destroy(window->callback);
	}

	/// Destroy the shared memory buffer.
	for (i = 0; i < NUM_DISPLAY_BUFFERS; i++) {
		wl_buffer_destroy(window->buffers[i]);
	}

	/// Destroy the shell surface object.
	wl_shell_surface_destroy(window->shellSurface);

	/// Destroy the window surface object.
	wl_surface_destroy(window->surface);

	if (window->shm) {
		wl_shm_destroy(window->shm);
	}
	
	/// Destroy the shell interface object.
	if (window->shell) {
		wl_shell_destroy(window->shell);
	}

	/// Destroy the output object
	for(i = 0; i < window->outputCount; i++){
		wl_output_destroy(window->outputs[i].output);
	}

	/// Destory the touch object
	if (window->touch) {
		wl_touch_destroy(window->touch);
	}

	/// Destory the pinter object
	if (window->pointer) {
		wl_pointer_destroy(window->pointer);
	}

	/// Destory the seat object
	if (window->seat) {
		wl_seat_destroy(window->seat);
	}

	/// Destroy the compositor interface object.
	if (window->compositor) {
		wl_compositor_destroy(window->compositor);
	}

	wl_registry_destroy(window->registry);

	/// Flush out buffered data.
	wl_display_flush(window->display);
	wl_display_disconnect(window->display);
	free(window);
}


/// Running flag, cleared by the signal interrupt.
static int running = 1;


/// Signal interrupt causes the main rendering loop to terminate.
static void signalInt(int signum) {
   running = 0;
}

/// This thread is responsible for dispatching the frame callback events in frame queue.
/// It invokes onDrawn() function whenever there is a frame callback event in the queue.
static void* dispatchFrame(void* wnd) {	
	struct SWindowData* window = (struct SWindowData*)wnd;

	/// Request to draw the first frame
	onDrawn(window, NULL, 0xffffffff);

	/// Dispatch the frame queue events for the display
	while (running) {
		if (wl_display_dispatch_queue(window->display, window->frameQueue) == -1) {
			ALOGE("wl_display_dispatch_queue() error, drawn thread exit!");
			running = 0;
		}
	}
	return NULL;
}


/// main function.
int main(int argc, char** argv)
{
	struct sigaction sigint;
	struct SWindowData* window;
	pthread_t tid = -1;
	int display = 0;

	if (argc == 1) {
		display = 0;
	} else if (argc == 2 && !strcmp(argv[1], "-e")) {
		display = 1;
	} else {
		fprintf(stderr, "usage: %s [-e]\n", argv[0]);
		return -1;
	}

	/// Create the wayland window and shm buffer.
	window = createWindow(display);

	/// Install signal handler for exiting application.
	sigint.sa_handler = signalInt;
	sigemptyset(&sigint.sa_mask);
	sigint.sa_flags = SA_RESETHAND;
	sigaction(SIGINT, &sigint, NULL);

	/// Create a thread for dispatch the frame finish event
	if (pthread_create(&tid, NULL, dispatchFrame, window)) {
		ALOGE("pthread_create() error, main thread exit!");
		running = 0;
	}

	/// Dispatch the main queue events for the display
	while (running) {
		if (wl_display_dispatch(window->display) == -1) {
			ALOGE("wl_display_dispatch() error, main thread exit!");
			running = 0;
		}
	}
	
	/// Wait the child thread.
	if (tid != -1) {
		pthread_join(tid, NULL);
	}

	/// Clean up everything.
	destroyWindow(window);
	return 0;
} 

