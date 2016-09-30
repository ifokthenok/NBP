#ifndef _HELPERS_H_
#define _HELPERS_H_


typedef uint32_t pixel;

void setup_wayland();
void cleanup_wayland();

struct wl_shm_pool* create_memory_pool(int fd);
void free_memory_pool(struct wl_shm_pool* pool);

struct wl_buffer* create_buffer(struct wl_shm_pool* pool, unsigned width, unsigned height);
void free_buffer(struct wl_buffer* buffer);

struct wl_shell_surface* create_shell_surface();
void free_shell_surface(struct wl_shell_surface* surface);
void bind_shell_surface(struct wl_buffer* buffer, struct wl_shell_surface* shell_surface);


#endif /* _HELPERS_H_ */
