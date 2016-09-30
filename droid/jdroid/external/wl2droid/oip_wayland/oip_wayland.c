/*******************************************************************************
 * Project         xse-wayland-demo-apps
 * (c) copyright   2013
 * Company         XS Embedded GmbH
 *                 All rights reserved.
 * Secrecy Level   STRICTLY CONFIDENTIAL
 ******************************************************************************/
/**
 * @file           xse-wayland-demo-egl.c
 * @ingroup        xse-wayland-demo-apps
 * @author         Alexander Irion
 * @brief          Demo application for:
 *                 + Opening a Wayland window
 *                 + Allocating a shared memory buffer for the window content.
 *                 + Rendering to the shared memory buffer.
 */

/*
 * Copyright © 2011 Benjamin Franzke
 * Copyright © 2010 Intel Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <wayland-client.h>

/// Stores the data structures used for the window.
/** Contains wayland and shm data. */
struct SWindowData
{
   /// The wayland display connection.
   struct wl_display *display;

   /// Registry for global wayland objects.
   struct wl_registry *registry;

   /// The wayland compositor.
   /** The wl_compositor_create_surface() function is used from this interface.
    *  The compositor is retrieved in the function displayHandleGlobal(). */
   struct wl_compositor *compositor;

   /// The wayland shell.
   /** The wl_shell_get_shellSurface() function is used from this interface.
    *  The shell is retrieved in the function displayHandleGlobal(). */
   struct wl_shell *shell;

   /// The shm interface.
   struct wl_shm *shm;

   /// The wayland surface.
   struct wl_surface *surface;

   /// The wayland shell surface interface.
   /** The wl_shell_surface_set_toplevel() function is used from this interface. */
   struct wl_shell_surface *shellSurface;

   /// The frame callback.
   struct wl_callback *callback;

   /// Wayland object that represents the shared memory buffer containing the rendered window content.
   struct wl_buffer *buffer;

   /// Pointer to the shared memory buffer (buffer).
   void *shmData;

   /// Wayland event mask.
   uint32_t mask;

   /// Available color formats for the shared memory buffer.
   uint32_t formats;

   /// Width and height of the window.
   int width, height;

};

/// Sets the file descriptor flag FD_CLOEXEC to a file handle.
/** Closes the file handle on error.
 * @param fd The file handle.
 * @return The file handle fd, when successful and -1 otherwise.  */
static int setCloExecOrClose(int fd)
{
   long flags;

   if (fd == -1)
      return -1;

   flags = fcntl(fd, F_GETFD);
   if (flags == -1)
   {
      close(fd);
      return -1;
   }

   if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
   {
      close(fd);
      return -1;
   }

   return fd;
}

/// Creates a unique temp file, opens it and sets the FD_CLOEXEC flag.
/** @param tmpname Template name for the temp file.
 *  @return The file descriptor, if successful or a negative value, otherwise. */
static int createTmpFileCloExec(char *tmpname)
{
   int fd;

   fd = mkstemp(tmpname);
   if (fd >= 0)
   {
      fd = setCloExecOrClose(fd);
      unlink(tmpname);
   }

   return fd;
}

/// Create a new, unique, anonymous file of the given size.
/** The file descriptor is set CLOEXEC. The file is immediately suitable for mmap()'ing
 *  the given size at offset zero.
 *  The file should not have a permanent backing store like a disk but may have if
 *  XDG_RUNTIME_DIR is not properly implemented in OS.
 *  The file name is deleted from the file system.
 *  The file is suitable for buffer sharing between processes by transmitting the file
 *  descriptor over Unix sockets using the SCM_RIGHTS methods.
 *
 *  @param size Size of the buffer.
 *  @return Returns the file handle to the created file. */
static int osCreateAnonymousFile(off_t size)
{
   static const char template[] = "/weston-shared-XXXXXX";
   const char *path;
   char *name;
   int fd;

   path = getenv("XDG_RUNTIME_DIR");
   if (!path)
   {
      errno = ENOENT;
      return -1;
   }

   name = malloc(strlen(path) + sizeof(template));
   if (!name)
      return -1;

   strcpy(name, path);
   strcat(name, template);

   fd = createTmpFileCloExec(name);

   free(name);

   if (fd < 0)
      return -1;

   if (ftruncate(fd, size) < 0)
   {
      close(fd);
      return -1;
   }

   return fd;
}

/// Creates a wayland shared memory buffer.
/** Stores a pointer to the buffer in window->shmData, used for rendering later.
 *  @param window The window structure, that contains all the window data.
 *  @param width Width of the buffer in pixel.
 *  @param height Height of the buffer in pixel.
 *  @param format Buffer pixel format.
 *  @return The allocated wayland buffer object. */
static struct wl_buffer *
createShmBuffer(struct SWindowData *window, int width, int height, uint32_t format)
{
   struct wl_shm_pool *pool;
   struct wl_buffer *buffer;
   int fd, size, stride;
   void *data;

   stride = width * 4;
   size = stride * height;

   // fd = osCreateAnonymousFile(size);


	fd = open("/tmp/externalbuffer", O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	if (fd < 0) {
		fprintf(stderr, "createShmBuffer(): can't open /tmp/backbuffer: %m");
		return -1;
	}
	if (ftruncate(fd, size) < 0) {
		fprintf(stderr, "createShmBuffer(): ftruncate() failed: %m");
	}
	
   if (fd < 0)
   {
      fprintf(stderr, "creating a buffer file for %d B failed: %m\n", size);
      return NULL ;
   }

   data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if (data == MAP_FAILED )
   {
      fprintf(stderr, "mmap failed: %m\n");
      close(fd);
      return NULL ;
   }

   /// Create s shm pool object.
   pool = wl_shm_create_pool(window->shm, fd, size);

   /// Create a buffer from the pool.
   buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, format);

   /// Destroy the pool object.
   wl_shm_pool_destroy(pool);

   close(fd);

   /// Store pointer to the buffer data, used for rendering.
   window->shmData = data;

   return buffer;
}

/// Callback to retrieve the available pixel formats.
/** Stores the format in the window structure.
 *  @param data The window structure, that contains all the window data.
 *  @param shm The wayland shm interface.
 *  @param format Additional format, that is supported. */
static void shmFormat(void *data, struct wl_shm* shm, uint32_t format)
{
   struct SWindowData *d = data;

   d->formats |= (1 << format);
}

/// Listener for the shared memory callback, that retrieves the available pixel formats.
struct wl_shm_listener shmListener = { shmFormat };

/// Retrieves the wayland compositor and shell.
/** @param data Structure that holds all the window data. 
 *  @param registry The wayland registry.
 *  @param id Identifier of the object.
 *  @param interface Name of the interface.
 *  @param version Wayland version of the object. */
static void registryHandleGlobal(void *data, struct wl_registry *registry, uint32_t id, const char *interface,
      uint32_t version)
{
   struct SWindowData* window = data;

   if (strcmp(interface, "wl_compositor") == 0)
   {
      window->compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
   }
   else if (strcmp(interface, "wl_shell") == 0)
   {
      window->shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
   }
   else if (strcmp(interface, "wl_shm") == 0)
   {
      window->shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
      wl_shm_add_listener(window->shm, &shmListener, window);
   }
}

/// Handle removal of global wayland objects.
/** @param data Structure that holds all the window data. 
 *  @param registry The wayland registry.
 *  @param id Identifier of the object. */
static void registryHandleGlobalRemove(void *data, struct wl_registry *registry, uint32_t id)
{
}

static const struct wl_registry_listener registryListener = {
   registryHandleGlobal,
   registryHandleGlobalRemove,
};

/// Creates the wayland window.
/** @param width Width of the window in pixel.
 *  @param height Height of the window in pixel.
 *  @return The created window structure. */
static struct SWindowData* createWindow(int width, int height)
{
   struct SWindowData* window = malloc(sizeof(struct SWindowData));

   window->display = wl_display_connect(NULL );

   window->formats = 0;

   /// Create registry to listen for globals.
   window->registry = wl_display_get_registry(window->display);

   /// Add listener to get the wayland compositor, shell and the shm interface.
   /// Installs a shm listener to retrieve the format.
   wl_registry_add_listener(window->registry, &registryListener, window);

   /// Process connection events.
   wl_display_roundtrip(window->display);

   /// Ensure, that format is received by the shm listener.
   wl_display_roundtrip(window->display);

   if (!(window->formats & (1 << WL_SHM_FORMAT_XRGB8888)))
   {
      fprintf(stderr, "WL_shmFormat_XRGB32 not available\n");
      exit(1);
   }

   /// Create the shared memory buffer and the wayland buffer object.
   window->buffer = createShmBuffer(window, width, height, WL_SHM_FORMAT_XRGB8888);

   if (!window->buffer)
   {
      free(window);
      return NULL ;
   }

   window->callback = NULL;
   window->width = width;
   window->height = height;

   /// Creates a wayland surface.
   window->surface = wl_compositor_create_surface(window->compositor);

   /// Gets the shell surface for the newly created wayland surface.
   window->shellSurface = wl_shell_get_shell_surface(window->shell, window->surface);
   wl_shell_surface_set_title(window->shellSurface, "YgomiDroid");
   /// Makes the shell surface top-level.
   wl_shell_surface_set_toplevel(window->shellSurface);

   return window;
}

/// Destroys the wayland window and the shm buffer.
static void destroyWindow(struct SWindowData *window)
{
   if (window->callback)
   {
      wl_callback_destroy(window->callback);
   }

   /// Destroy the shared memory buffer.
   wl_buffer_destroy(window->buffer);

   /// Destroy the shell surface object.
   wl_shell_surface_destroy(window->shellSurface);

   /// Destroy the widnwo surface object.
   wl_surface_destroy(window->surface);

   if (window->shm)
      wl_shm_destroy(window->shm);

   if (window->shell)
   {
      /// Destroy the shell interface object.
      wl_shell_destroy(window->shell);
   }

   if (window->compositor)
   {
      /// Destroy the compositor interface object.
      wl_compositor_destroy(window->compositor);
   }

   wl_registry_destroy(window->registry);

   /// Flush out buffered data.
   wl_display_flush(window->display);
   wl_display_disconnect(window->display);

   free(window);
}

/// Draw some moving square and cirlce bars to the buffer.
/** @param image Pointer to the buffer.
 *  @param width Width of the buffer in pixel.
 *  @param height Height of the buffer in pixel.
 *  @param time Time for animation. */
static void paintPixels(void *image, int width, int height, uint32_t time)
{
   const int halfh = height / 2;
   const int halfw = width / 2;
   int ir, or;
   uint32_t *pixel = image;
   int y;

   /* squared radii thresholds */
   or = (halfw < halfh ? halfw : halfh) - 8;
   ir = or - 32;
   or *= or;
   ir *= ir;

   for (y = 0; y < height; y++)
   {
      int x;
      int y2 = (y - halfh) * (y - halfh);

      for (x = 0; x < width; x++)
      {
         uint32_t v;

         /* squared distance from center */
         int r2 = (x - halfw) * (x - halfw) + y2;

         if (r2 < ir)
         {
            v = (r2 / 32 + time / 64) * 0x0080401;
         }
         else
         {
            if (r2 < or)
            {
               v = (y + time / 32) * 0x0080401;
            }
            else
            {
               v = (x + time / 16) * 0x0080401;
            }
            v &= 0x00ffffff;
         }

         /* cross if compositor uses X from XRGB as alpha */
         if (abs(x - y) > 6 && abs(x + y - height) > 6)
         {
            v |= 0xff000000;
         }

         *pixel++ = v;
      }
   }
}

/// Forward declaration of the frame listener callback.
static const struct wl_callback_listener frameListener;

/// Function, called by the frame listener callback to render one frame.
/** @param data Structure that holds all the window data.
 *  @param callback The callback of this function call.
 *  @param Time in milliseconds. */
static void redraw(void *data, struct wl_callback *callback, uint32_t time)
{
   struct SWindowData *window = data;

   /// Draw content to the shared memory buffer.
   //paintPixels(window->shmData, window->width, window->height, time);

   /// Attach buffer to surface.
   wl_surface_attach(window->surface, window->buffer, 0, 0);

   /// Invalidate the full content of the surface.
   wl_surface_damage(window->surface, 0, 78, window->width, window->height);

   /// Destroy old callback.
   if (callback)
   {
      wl_callback_destroy(callback);
   }

   /// Create new frame callback to render next frame.
   window->callback = wl_surface_frame(window->surface);

   /// Set listener of callback upon finish of redraw.
   wl_callback_add_listener(window->callback, &frameListener, window);

   /// Commit surface requests of attach, damage and frame.
   wl_surface_commit(window->surface);
}

/// Frame listener callback, that calls the redraw() method above.
static const struct wl_callback_listener frameListener = { redraw };

/// Running flag, cleared by the signal interrupt.
static int running = 1;

/// Signal interrupt.
/** Causes the main rendering loop to terminate. */
static void signalInt(int signum)
{
   running = 0;
}

/// main function.
/** @param argc Argument count.
 *  @param argv Program arguments. */
int main(int argc, char **argv)
{
   struct sigaction sigint;
   struct SWindowData *window;

   /// Create the wayland window and shm buffer.
   //window = createWindow(1024, 768);
   window = createWindow(400, 300);

   /// Install signal handler for exiting application.
   sigint.sa_handler = signalInt;
   sigemptyset(&sigint.sa_mask);
   sigint.sa_flags = SA_RESETHAND;
   sigaction(SIGINT, &sigint, NULL );

   /// Render first frame and install callback for rendering the next frame.
   redraw(window, NULL, 0);

   while (running)
   {
      /// Results in a callback to redraw() to render one frame and installs
      /// a callback for rendering the next frame.
      wl_display_dispatch(window->display);
   }

   /// Clean up everything.
   destroyWindow(window);

   return 0;
}
