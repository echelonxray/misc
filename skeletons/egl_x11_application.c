/*
 * Build with: gcc ./egl_x11_application.c -o ./egl_x11_application.out -lX11 -lEGL -lGL
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <EGL/egl.h>
//#include <GLES/gl.h>
//#include <GLES2/gl2.h>
#include <GLES3/gl32.h>

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

#define CHECK_GL_ERRORS(arg0, arg1) { \
		GLenum __glerr; \
		unsigned int __hit_err = 0; \
		while ((__glerr = glGetError()) != GL_NO_ERROR) { \
			fprintf(stderr, "Error (%d, Line: %u): %s\n", \
			  (int)__glerr, (unsigned int)__LINE__, get_gl_error_text(__glerr)); \
			__hit_err = 1; \
		} \
		if (__hit_err) { \
			arg0 = 1; \
			goto arg1; \
		} \
	}

typedef struct {
	GLuint vert_shader;
	GLuint frag_shader;
} PipelineShaders;

typedef struct {
	unsigned char red;
	unsigned char gre;
	unsigned char blu;
	unsigned char alp;
} Pixel;

char* get_gl_error_text(GLenum err) {
	if        (err == GL_NO_ERROR) {
		return "No Error";
	} else if (err == GL_INVALID_ENUM) {
		return "Invalid Enum";
	} else if (err == GL_INVALID_VALUE) {
		return "Invalid Value";
	} else if (err == GL_INVALID_OPERATION) {
		return "Invalid Operation";
	} else if (err == GL_INVALID_FRAMEBUFFER_OPERATION) {
		return "Invalid Framebuffer Operation";
	} else if (err == GL_OUT_OF_MEMORY) {
		return "Out Of Memory";
	} else if (err == GL_STACK_UNDERFLOW) {
		return "Stack Underflow";
	} else if (err == GL_STACK_OVERFLOW) {
		return "Stack Overflow";
	}
	return "Unknown Error";
}

int create_shaders(GLuint program, PipelineShaders* shaders) {
	const char* vert_shader_text =
	  "#version 300 es                                               \n"
	  "in vec3 pos;                                                  \n"
	  "in vec4 color;                                                \n"
	  "in vec2 texcoord;                                             \n"
	  "out vec4 fcolor;                                              \n"
	  "out vec2 ftexcoord;                                           \n"
	  "void main() {                                                 \n"
	  "  gl_Position = vec4(pos.x, pos.y, pos.z, 1.0f);              \n"
	  "  fcolor = color;                                             \n"
	  "  ftexcoord = vec2(texcoord.x, texcoord.y);                   \n"
	  "}                                                             \n";
	const char* frag_shader_text =
	  "#version 300 es                                               \n"
	  "precision mediump float;                                      \n"
	  "out vec4 FragColor;                                           \n"
	  "in vec4 fcolor;                                               \n"
	  "in vec2 ftexcoord;                                            \n"
	  "uniform sampler2D mytexture;                                  \n"
	  "void main() {                                                 \n"
	  "  //FragColor = fcolor;                                       \n"
	  "  FragColor = texture(mytexture, ftexcoord) * fcolor;         \n"
	  "}                                                             \n";

	int retval = 0;
	GLint status_buf;

	GLuint vert_shader;
	GLuint frag_shader;

	vert_shader = glCreateShader(GL_VERTEX_SHADER);
	if (vert_shader == 0) {
		// Error: Creation of Vertex Shader Failed
		fprintf(stderr, "Error: Vertex Shader Creation Failed.\n");
		retval = 1;
		goto error_cleanup_00;
	}
	frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	if (frag_shader == 0) {
		// Error: Creation of Fragment Shader Failed
		fprintf(stderr, "Error: Frament Shader Creation Failed.\n");
		retval = 1;
		goto error_cleanup_01;
	}

	glShaderSource(vert_shader, 1, &vert_shader_text, NULL);
	glShaderSource(frag_shader, 1, &frag_shader_text, NULL);

	glCompileShader(vert_shader);
	glCompileShader(frag_shader);

	glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &status_buf);
	if (status_buf == 0) {
		// Error: Compilation of Vertex Shader Failed
		fprintf(stderr, "Error: Vertex Shader Compilation Failed.\n");

		char buff[1024];
		memset(buff, 0, 1024);
		glGetShaderInfoLog(vert_shader, 1024, NULL, buff);
		fprintf(stderr, "%s\n", buff);

		retval = 1;
		goto error_cleanup_02;
	}
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &status_buf);
	if (status_buf == 0) {
		// Error: Compilation of Fragment Shader Failed
		fprintf(stderr, "Error: Fragment Shader Compilation Failed.\n");

		char buff[1024];
		memset(buff, 0, 1024);
		glGetShaderInfoLog(frag_shader, 1024, NULL, buff);
		fprintf(stderr, "%s\n", buff);

		retval = 1;
		goto error_cleanup_02;
	}

	glAttachShader(program, vert_shader);
	glAttachShader(program, frag_shader);

	glBindAttribLocation(program, 0, "pos");
	glBindAttribLocation(program, 1, "color");
	glBindAttribLocation(program, 2, "texcoord");

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status_buf);
	if (status_buf == 0) {
		// Error: Linking shaders into program Failed
		fprintf(stderr, "Error: Shader Link Failed.\n");
		retval = 1;
		goto error_cleanup_04;
	}

	shaders->vert_shader = vert_shader;
	shaders->frag_shader = frag_shader;

	return 0;

	error_cleanup_04:
	glDetachShader(program, frag_shader);
	error_cleanup_03:
	glDetachShader(program, vert_shader);
	error_cleanup_02:
	glDeleteShader(frag_shader);
	error_cleanup_01:
	glDeleteShader(vert_shader);
	error_cleanup_00:

	return retval;
}

void destroy_shaders(GLuint program, PipelineShaders* shaders) {
	glDetachShader(program, shaders->frag_shader);
	glDetachShader(program, shaders->vert_shader);
	glDeleteShader(shaders->frag_shader);
	glDeleteShader(shaders->vert_shader);
	return;
}

void fill_framebuffer(size_t width, size_t height, Pixel* buffer) {
	uint32_t color = 0xFF000000;
	for (size_t y = 0; y < height; y += 20) {
		for (size_t x = 0; x < width; x += 20) {
			color >>= 8;
			for (size_t i = 0; (i + x) < width && i < 20; i++) {
				for (size_t j = 0; (j + y) < height && j < 20; j++) {
					size_t px = x + i;
					size_t py = y + j;
					Pixel pcolor;
					pcolor.alp = (color >> 24) & 0xFF;
					pcolor.red = (color >> 16) & 0xFF;
					pcolor.gre = (color >>  8) & 0xFF;
					pcolor.blu = (color >>  0) & 0xFF;
					buffer[width * py + px] = pcolor;
				}
			}
			if (color == 0x000000FF) {
				color = 0xFF000000;
			}
		}
	}
	return;
}

void fill_framebuffer_color(size_t width, size_t height, Pixel* buffer, uint32_t color) {
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			Pixel pcolor;
			pcolor.red = (color >> 24) & 0xFF;
			pcolor.gre = (color >> 16) & 0xFF;
			pcolor.blu = (color >>  8) & 0xFF;
			pcolor.alp = (color >>  0) & 0xFF;
			buffer[width * y + x] = pcolor;
		}
	}
	return;
}

int main(int argc, char* argv[]) {
	int exit_code = 0;
	size_t width = 500;
	size_t height = 500;

	Display *d;
	Window w;
	Window r;
	GC g;
	int s;
	int event_mask;

	int reti;
	EGLBoolean retv;
	EGLDisplay ed;
	EGLint edmajor;
	EGLint edminor;

	event_mask = KeyPressMask | KeyReleaseMask |
	             ButtonPressMask | ButtonReleaseMask |
	             PointerMotionMask |
	             ExposureMask |
	             StructureNotifyMask;

	d = XOpenDisplay(NULL);
	if (d == NULL) {
		fprintf(stderr, "Cannot open display\n");
		exit_code = 1;
		goto ecleanup_application_000;
	}
	ed = eglGetDisplay(d);
	if (ed == EGL_NO_DISPLAY) {
		fprintf(stderr, "Cannot open EGL display\n");
		exit_code = 1;
		goto ecleanup_application_001;
	}
	retv = eglInitialize(ed, &edmajor, &edminor);
	if (retv == EGL_FALSE) {
		fprintf(stderr, "Cannot init EGL display\n");
		exit_code = 1;
		goto ecleanup_application_001;
	}

	s = DefaultScreen(d);
	r = RootWindow(d, s);

	static const EGLint attribs[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_DEPTH_SIZE, 1,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
		EGL_NONE
	};
	EGLConfig config;
	EGLint egl_int = 0;
	retv = eglChooseConfig(ed, attribs, &config, 1, &egl_int);
	if (retv == EGL_FALSE || egl_int == 0) {
		fprintf(stderr, "Cannot find suitable EGL config\n");
		exit_code = 1;
		goto ecleanup_application_002;
	}

	retv = eglGetConfigAttrib(ed, config, EGL_NATIVE_VISUAL_ID, &egl_int);
	if (retv == EGL_FALSE) {
		printf("Error: eglGetConfigAttrib() failed\n");
		exit_code = 1;
		goto ecleanup_application_002;
	}

	XVisualInfo* visinfo;
	XVisualInfo visinfo_search_params;
	int visinfo_count;
	visinfo_search_params.visualid = egl_int;
	visinfo = XGetVisualInfo(d, VisualIDMask, &visinfo_search_params, &visinfo_count);
	if (visinfo == NULL || visinfo_count == 0) {
		printf("Error: XGetVisualInfo() failed\n");
		exit_code = 1;
		goto ecleanup_application_002;
	}

	Colormap colormap;
	XSetWindowAttributes attr;
	unsigned long mask;
	colormap = XCreateColormap(d, r, visinfo->visual, AllocNone); // Maybe need to check for error return?
	attr = (typeof(attr)) { // Set: Denotes the params that I am trying to set.
		.background_pixmap = 0,     /* background, None, or ParentRelative */
		.background_pixel = 0,      /* background pixel */                        // Set
		.border_pixmap = 0,         /* border of the window or CopyFromParent */
		.border_pixel = 0,          /* border pixel value */                      // Set
		.bit_gravity = 0,           /* one of bit gravity values */
		.win_gravity = 0,           /* one of the window gravity values */
		.backing_store = 0,         /* NotUseful, WhenMapped, Always */
		.backing_planes = 0,        /* planes to be preserved if possible */
		.backing_pixel = 0,         /* value to use in restoring planes */
		.save_under = 0,            /* should bits under be saved? (popups) */
		.event_mask = event_mask,   /* set of events that should be saved */      // Set
		.do_not_propagate_mask = 0, /* set of events that should not propagate */
		.override_redirect = 0,     /* boolean value for override_redirect */
		.colormap = colormap,       /* color map to be associated with window */  // Set
		.cursor = 0,                /* cursor to be displayed (or None) */
	};
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	w = XCreateWindow(d, r,          // Display, Root windows
	                  10, 10,        // X, Y
					  width, height, // Width, Height
					  0,             // Border Width
					  visinfo->depth, InputOutput,
					  visinfo->visual, mask, &attr);

	EGLContext ectx;
	EGLint ctx_attribs[] = {
		EGL_CONTEXT_MAJOR_VERSION, 3,
		EGL_NONE
	};
	ectx = eglCreateContext(ed, config, EGL_NO_CONTEXT, ctx_attribs);
	if (ectx == EGL_NO_CONTEXT) {
		printf("Error: eglCreateContext failed\n");
		exit_code = 1;
		goto ecleanup_application_005;
	}

	EGLSurface esurface;
	esurface = eglCreateWindowSurface(ed, config, w, NULL);
	if (esurface == EGL_NO_SURFACE) {
		printf("Error: eglCreateWindowSurface failed\n");
		exit_code = 1;
		goto ecleanup_application_006;
	}

	XStoreName(d, w, "Example X11"); // Set Window Manager Title Text
	XMapWindow(d, w);
	g = XCreateGC(d, w, 0, 0);
	XSetBackground(d, g, 0x00000000);
	XSelectInput(d, w, event_mask);

	// Capture WM close button click in addition to the above selected input events
	Atom wm_delete_window = XInternAtom(d, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(d, w, &wm_delete_window, 1);

	retv = eglMakeCurrent(ed, esurface, esurface, ectx);
	if (retv == EGL_FALSE) {
		printf("Error: eglMakeCurrent() failed\n");
		exit_code = 1;
		goto ecleanup_application_009;
	}

	GLuint program;
	program = glCreateProgram();
	if (program == 0) {
		printf("Error: glCreateProgram() failed\n");
		exit_code = 1;
		goto ecleanup_application_010;
	}

	PipelineShaders shaders;
	reti = create_shaders(program, &shaders);
	if (reti != 0) {
		printf("Error: create_shaders() failed\n");
		exit_code = 1;
		goto ecleanup_application_011;
	}

	glUseProgram(program);

	/*
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glFlush();
	eglSwapBuffers(ed, esurface);
	*/

	float vertices[] = { // X, Y, Z
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f,
		-1.0f, +1.0f, +1.0f,

		+1.0f, +1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,
	};
	float colors[] = { // R, G, B, A
		+1.0f, +0.0f, +0.0f, +1.0f,
		+0.0f, +1.0f, +0.0f, +1.0f,
		+0.0f, +0.0f, +1.0f, +1.0f,

		+1.0f, +0.0f, +0.0f, +1.0f,
		+0.0f, +1.0f, +0.0f, +1.0f,
		+0.0f, +0.0f, +1.0f, +1.0f,
	};
	float texture_coordinates[] = { // X, -Y
		+0.0f, +1.0f,
		+1.0f, +1.0f,
		+0.0f, +0.0f,

		+1.0f, +0.0f,
		+1.0f, +1.0f,
		+0.0f, +0.0f,
	};

	GLuint gl_buffer_objs[3];
	memset(gl_buffer_objs, 0, sizeof(gl_buffer_objs));
	glGenBuffers(sizeof(gl_buffer_objs) / sizeof(*gl_buffer_objs), gl_buffer_objs);
	for (size_t i = 0; i < (sizeof(gl_buffer_objs) / sizeof(*gl_buffer_objs)); i++) {
		if (gl_buffer_objs[i] == 0) {
			printf("Error: glGenBuffers() failed\n");
			exit_code = 1;
			goto ecleanup_application_012;
		}
	}

	// Setup input 0: Vertices
	glBindBuffer(GL_ARRAY_BUFFER, gl_buffer_objs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(*vertices), 0);
	CHECK_GL_ERRORS(exit_code, ecleanup_application_012);

	// Setup input 1: Colors
	glBindBuffer(GL_ARRAY_BUFFER, gl_buffer_objs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(*colors), 0);
	CHECK_GL_ERRORS(exit_code, ecleanup_application_012);

	// Setup input 2: Texture Coordinates
	glBindBuffer(GL_ARRAY_BUFFER, gl_buffer_objs[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coordinates), texture_coordinates, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(*texture_coordinates), 0);
	CHECK_GL_ERRORS(exit_code, ecleanup_application_012);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	CHECK_GL_ERRORS(exit_code, ecleanup_application_015);

	GLuint textures[1];
	memset(textures, 0, sizeof(textures));
	glGenTextures(1, textures);
	for (size_t i = 0; i < (sizeof(textures) / sizeof(*textures)); i++) {
		if (textures[i] == 0) {
			printf("Error: glGenTextures() failed\n");
			exit_code = 1;
			goto ecleanup_application_016;
		}
	}

	//glUniform1i(glGetUniformLocation(program, "mytexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	CHECK_GL_ERRORS(exit_code, ecleanup_application_016);

	Pixel* framebuffer = NULL;
	framebuffer = calloc(width * height, sizeof(*framebuffer));
	if (framebuffer == NULL) {
		exit_code = 1;
		goto ecleanup_application_016;
	}
	fill_framebuffer(width, height, framebuffer);

	/*
	int img_width;
	int img_height;
	unsigned char *img_data;
	img_data = stbi_load("xp_remaster_500x500.png", &img_width,
	                                                &img_height,
	                                                NULL, 4);
	if (img_data == NULL) {
		exit_code = 1;
		goto ecleanup_application_017;
	}
	if (img_width != 500 || img_height != 500) {
		exit_code = 1;
		goto ecleanup_application_018;
	}
	*/

	//glUniform1i(glGetUniformLocation(program, "mytexture"), 0);
	//CHECK_GL_ERRORS(exit_code, ecleanup_application_016);

	/*
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	eglSwapBuffers(ed, esurface);
	*/

	// Event Loop
	struct pollfd fds[1];
	fds[0].fd = XConnectionNumber(d);
	fds[0].events = POLLIN;
	fds[0].revents = 0;
	while (1) {
		signed int poll_ret;
		poll_ret = poll(fds, sizeof(fds) / sizeof(*fds), 5000);
		if (poll_ret <= 0) {
			continue;
		}

		// Xlib maintains a local event queue behind the scenes.  Event notifications are read off the
		// file descriptor and stored into an application local event queue.  Xlib may read in events
		// off this underlying connection to the X server during other Xlib API calls.  Therefore, it
		// is important to process all events in the queue, rather than immediately returning to the poll()
		// syscall after each event is handled.  Otherwise, there may still be events in the queue, but no
		// longer pending on the file descriptor.
		//
		// The other possibility is that the data gets stuck in the output buffer.  Using XPending
		// solves that too as it will Flush the output buffer for us.
		int ignore_event = 0;
		while (XPending(d)) {
			XEvent event;
			XNextEvent(d, &event);
			if (ignore_event) {
				ignore_event = 0;
				continue;
			}

			if        (event.type == KeyRelease) {
				// If this is a key repeat event, ignore it.  Disable key repeat.
				if (XEventsQueued(d, QueuedAfterReading)) {
					XEvent next_event;
					XPeekEvent(d, &next_event);
					if (next_event.type == KeyPress &&
					    next_event.xkey.time == event.xkey.time &&
					    next_event.xkey.keycode == event.xkey.keycode) {
						ignore_event = 1;
						continue;
					}
				}

				//unsigned int key_code = event.xkey.keycode;
				// TODO: Handle Key Release
			} else if (event.type == KeyPress) {
				//unsigned int key_code = event.xkey.keycode;
				// TODO: Handle Key Press
				goto close_application;
			} else if (event.type == ButtonPress) {
				// TODO: Handle Button Press
			} else if (event.type == Expose) {
				// Redraw the frame
				int triangle_count = 2;
				glClearColor(0.3, 0.3, 0.3, 1.0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				//glActiveTexture(GL_TEXTURE0);
				//glBindTexture(GL_TEXTURE_2D, textures[0]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer);
				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
				glDrawArrays(GL_TRIANGLES, 0, 3 * triangle_count);
				eglSwapBuffers(ed, esurface);
				//const char *msg = "Hello, World!";
				//XDrawString(d, w, DefaultGC(d, s), 10, 50, msg, strlen(msg));
				//XFillRectangle(d, w, DefaultGC(d, s), 20, 20, 10, 10);
			} else if (event.type == ConfigureNotify) {
				// Handle window resize
				width  = event.xconfigure.width;
				height = event.xconfigure.height;
				framebuffer = reallocarray(framebuffer, width * height, sizeof(*framebuffer));
				// TODO: Handle failure
				fill_framebuffer(width, height, framebuffer);
				glViewport(0, 0, width, height);
			} else if (event.type == ClientMessage) {
				// Handle window close by window manager close button click
				if ((Atom)event.xclient.data.l[0] == wm_delete_window) {
					goto close_application;
				}
			}
		}
		CHECK_GL_ERRORS(exit_code, close_application);
	}
	close_application:

	//ecleanup_application_018: stbi_image_free(img_data);
	ecleanup_application_017: free(framebuffer);
	ecleanup_application_016: glDeleteTextures(sizeof(textures) / sizeof(*textures), textures);
	ecleanup_application_015: glDisableVertexAttribArray(2);
	ecleanup_application_014: glDisableVertexAttribArray(1);
	ecleanup_application_013: glDisableVertexAttribArray(0);
	ecleanup_application_012: glDeleteBuffers(sizeof(gl_buffer_objs) / sizeof(*gl_buffer_objs), gl_buffer_objs);
	ecleanup_application_011: destroy_shaders(program, &shaders);
	ecleanup_application_010: glDeleteProgram(program);
	ecleanup_application_009: XFreeGC(d, g);
	ecleanup_application_008: XUnmapWindow(d, w);
	ecleanup_application_007: eglDestroySurface(ed, esurface);
	ecleanup_application_006: eglDestroyContext(ed, ectx);
	ecleanup_application_005: XDestroyWindow(d, w);
	ecleanup_application_004: XFreeColormap(d, colormap);
	ecleanup_application_003: XFree(visinfo);
	ecleanup_application_002: eglTerminate(ed);
	ecleanup_application_001: XCloseDisplay(d);
	ecleanup_application_000:

	return exit_code;
}
