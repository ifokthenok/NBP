#include <stdio.h>
#include <fcntl.h>
#include <cutils/ashmem.h>
#include <wayland-client.h>
#include "helpers.h"

extern struct wl_display* display;
extern uint32_t output_width;
extern uint32_t output_height;
extern uint32_t output_known;

int main(int argc, char* argv[])
{
	struct wl_shm_pool* pool;
	struct wl_buffer* buffer;
	struct wl_buffer* sceen_buffer;
	struct wl_shell_surface* shell_surface;
	int done = 0;
	int fd = -1;

	setup_wayland();

	// waiting for wayland output mode avaliable
	while (!output_width || !output_height) {
		if (wl_display_dispatch(display) < 0) {
            fprintf(stderr, "wl_display_dispatch() error\n");
			cleanup_wayland();
			return -1;
        }
		sleep(1);
	}

	fd = ashmem_create_region("wl2droid-buffer", output_width*output_height*sizeof(pixel));
    if (fd < 0) {
        fprintf(stdout, "couldn't create ashmem (%s)", strerror(-errno));
		cleanup_wayland();
		return -1;
    }
 
	pool = create_memory_pool(fd);
	buffer = create_buffer(pool, output_width, output_height);
	shell_surface = create_shell_surface();
	bind_shell_surface(buffer, shell_surface);

    while (!done) {
        if (wl_display_dispatch(display) < 0) {
            fprintf(stderr, "wl_display_dispatch() error\n");
            done = 1;
        }
    }
	
    free_shell_surface(shell_surface);
	free_buffer(buffer);
	free_memory_pool(pool);
	close(fd);
	cleanup_wayland();
    return 0;
}
