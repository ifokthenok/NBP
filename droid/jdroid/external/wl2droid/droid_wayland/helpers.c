#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <wayland-client.h>
#include "helpers.h"

struct wl_display* display;
struct wl_compositor* compositor;
struct wl_shell* shell;
struct wl_shm* shm;
struct wl_seat* seat;
struct wl_pointer* pointer;
struct wl_output* output;

uint32_t output_width = 0;
uint32_t output_height = 0;

static const struct wl_shell_surface_listener shell_surface_listener;
static const struct wl_pointer_listener pointer_listener;
static const struct wl_output_listener output_listener;

static void registry_global(void *data,
    struct wl_registry *registry, uint32_t name,
    const char *interface, uint32_t version)
{

	fprintf(stdout, "registry_global(): name=%d, interface=%s, version=%d \n", 
		name, interface, version);

    if (strcmp(interface, wl_compositor_interface.name) == 0) { 
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
        shm = wl_registry_bind(registry, name, &wl_shm_interface, version);
    } else if (strcmp(interface, wl_shell_interface.name) == 0) {
        shell = wl_registry_bind(registry, name, &wl_shell_interface, version);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
        pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(pointer, &pointer_listener, NULL);
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
		output = wl_registry_bind(registry, name, &wl_output_interface, version);
		wl_output_add_listener(output, &output_listener, NULL);

	}
}

static void registry_global_remove(void *a,
    struct wl_registry *b, uint32_t c) { 
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove
};

void setup_wayland()
{
    struct wl_registry *registry;
	
    display = wl_display_connect(0);
    if (display == NULL) {
		fprintf(stderr, "wl_display_connect() error\n");
        exit(-1);
    }

    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, &compositor);
    wl_display_roundtrip(display);
    wl_registry_destroy(registry);
}



void cleanup_wayland()
{
	wl_output_destroy(output);
	wl_pointer_destroy(pointer);
	wl_seat_destroy(seat);
    wl_shell_destroy(shell);
    wl_shm_destroy(shm);
    wl_compositor_destroy(compositor);
    wl_display_disconnect(display);
}

struct pool_data {
    int fd;
    pixel* memory;
    unsigned capacity;
    unsigned size;
};

struct wl_shm_pool* create_memory_pool(int file)
{
    struct pool_data* data;
    struct wl_shm_pool* pool;
    struct stat stat;
	unsigned mapsize;
	int i, j;
	unsigned* pixels;
	char* bytes;

    data = malloc(sizeof(struct pool_data));

    if (data == NULL)
        return NULL;

    data->capacity = output_width*output_height*sizeof(pixel);
    data->size = 0;
    data->fd = file;
    data->memory = mmap(0, data->capacity,
        PROT_READ | PROT_WRITE , MAP_SHARED, data->fd, 0);
    if (data->memory == MAP_FAILED) {
		fprintf(stderr, "create_memory_pool(): mmap() failed\n");
        goto cleanup_alloc;

	}

	//memset(data->memory, 0xff, data->capacity);
	bytes = (char*)data->memory;
	pixels = (unsigned*)data->memory;
	for(i=0; i < output_height; i++) {
		for(j=0; j < output_width; j++) {
		
			*pixels++ = i < output_height/2 ? 
				      (j < output_width/2 ? 0xffffffff : 0xffff0000) :
				      (j < output_width/2 ? 0xff00ff00 : 0xff0000ff);

			/* 
			if (i < output_height/2) {
				*bytes++ = 0x00;
				*bytes++ = 0x00; 
				*bytes++ = 0xff; 
				*bytes++ = 0x00;
			} else {
				*bytes++ = 0xff;
				*bytes++ = 0xff;
				*bytes++ = 0xff;
				*bytes++ = 0x00;
			}
			*/
				     
		}
	}
	
	
	
    pool = wl_shm_create_pool(shm, data->fd, data->capacity);
    if (pool == NULL) {
		fprintf(stderr, "create_memory_pool(): wl_shm_create_pool() failed\n");
        goto cleanup_mmap;
    }

    wl_shm_pool_set_user_data(pool, data);
    return pool;

cleanup_mmap:
    munmap(data->memory, mapsize);
cleanup_alloc:
    free(data);
    return NULL;
}

void free_memory_pool(struct wl_shm_pool* pool)
{
    struct pool_data* data;
	unsigned mapsize;
    data = wl_shm_pool_get_user_data(pool);
    wl_shm_pool_destroy(pool);
    munmap(data->memory, data->capacity);
    free(data);
}

struct wl_buffer* create_buffer(struct wl_shm_pool* pool,
    unsigned width, unsigned height) {
    struct pool_data* pool_data;
    struct wl_buffer* buffer;
	
    pool_data = wl_shm_pool_get_user_data(pool);
    buffer = wl_shm_pool_create_buffer(pool,
        pool_data->size*sizeof(pixel), width, height,
        width*sizeof(pixel), WL_SHM_FORMAT_XRGB8888/*WL_SHM_FORMAT_ARGB8888*/);
           
    if (buffer == NULL)
        return NULL;

    pool_data->size += width*height;
    return buffer;
}

void free_buffer(struct wl_buffer* buffer) {
    wl_buffer_destroy(buffer);
}

struct wl_shell_surface* create_shell_surface() {
    struct wl_surface* surface;
    struct wl_shell_surface* shell_surface;

    surface = wl_compositor_create_surface(compositor);
    if (surface == NULL)
        return NULL;
	
    shell_surface = wl_shell_get_shell_surface(shell, surface);
    if (shell_surface == NULL) {
        wl_surface_destroy(surface);
        return NULL;
    }

    wl_shell_surface_add_listener(shell_surface, &shell_surface_listener, 0);
    wl_shell_surface_set_fullscreen(shell_surface, 
    	WL_SHELL_SURFACE_FULLSCREEN_METHOD_SCALE, 0, NULL);
    wl_shell_surface_set_user_data(shell_surface, surface);
    wl_surface_set_user_data(surface, NULL);
    return shell_surface;
}


static void shell_surface_ping(void *data,
    struct wl_shell_surface *shell_surface, uint32_t serial) {
    wl_shell_surface_pong(shell_surface, serial);
}

static void shell_surface_configure(void *data,
    struct wl_shell_surface *shell_surface,
    uint32_t edges, int32_t width, int32_t height) { 
}

static const struct wl_shell_surface_listener shell_surface_listener = {
    .ping = shell_surface_ping,
    .configure = shell_surface_configure,
};


void free_shell_surface(struct wl_shell_surface *shell_surface) {
    struct wl_surface *surface;
    surface = wl_shell_surface_get_user_data(shell_surface);
    wl_shell_surface_destroy(shell_surface);
    wl_surface_destroy(surface);
}

void bind_shell_surface(struct wl_buffer *buffer, struct wl_shell_surface *shell_surface)
{
    struct wl_surface *surface;
    surface = wl_shell_surface_get_user_data(shell_surface);
    wl_surface_attach(surface, buffer, 0, 0);
	wl_surface_damage(surface, 0, 0, output_width, output_height);
    wl_surface_commit(surface);
}

static void pointer_enter(void *data, 
	struct wl_pointer *wl_pointer,
    uint32_t serial, struct wl_surface *surface,
    wl_fixed_t surface_x, wl_fixed_t surface_y) {

	fprintf(stderr, "pointer_enter()\n");
	//wl_pointer_set_cursor(wl_pointer, serial, NULL, 1,  1);
}

static void pointer_leave(void* data,
    struct wl_pointer* wl_pointer, uint32_t serial,
    struct wl_surface* wl_surface) { 
    fprintf(stderr, "pointer_leave()\n");
}

static void pointer_motion(void* data,
    struct wl_pointer* wl_pointer, uint32_t time,
    wl_fixed_t surface_x, wl_fixed_t surface_y) { 
    fprintf(stderr, "pointer_motion(): x=%d, y=%d\n", surface_x, surface_y);
}

static void pointer_button(void* data,
    struct wl_pointer* wl_pointer, uint32_t serial,
    uint32_t time, uint32_t button, uint32_t state) {

	fprintf(stderr, "pointer_button(): serial=%d, button=%d, state=%d\n", 
		serial, button, state);
}

static void pointer_axis(void* data,
    struct wl_pointer* wl_pointer, uint32_t time,
    uint32_t axis, wl_fixed_t value) { 
    fprintf(stderr, "pointer_axis(): axis=%d, value=%d\n", axis, value);
}


static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_enter,
    .leave = pointer_leave,
    .motion = pointer_motion,
    .button = pointer_button,
    .axis = pointer_axis
};


static void output_geometry(void *data, struct wl_output *wl_output,
		 int32_t x, int32_t y,
		 int32_t physical_width, int32_t physical_height,
		 int32_t subpixel, const char *make, const char *model,
		 int32_t transform) {
	fprintf(stderr, "output_geometry(): x=%d, y=%d, "
		"physical_width=%d, physical_height=%d, subpixel=%d, make=%s, model=%s\n",
		x, y, physical_width, physical_height, subpixel, make, model);
}

static void output_mode(void *data, struct wl_output *wl_output,
		 uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
	fprintf(stderr, "output_mode(): width=%d, height=%d, refresh=%d\n",
		width, height, refresh);
	output_width = width;
	output_height = height;
}


static void output_done(void *data, struct wl_output *wl_output) {
	 fprintf(stderr, "output_done()\n");
}


static void output_scale(void *data, struct wl_output *wl_output, int32_t factor) {
	fprintf(stderr, "output_scale(): factor=%d\n", factor);
}


static const struct wl_output_listener output_listener = {
	.geometry = output_geometry,
	.mode = output_mode,
	.done = output_done,
	.scale = output_scale
};

