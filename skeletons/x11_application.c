#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>

int main(void) {
	Display *d;
	Window w;
	XEvent e;
	GC g;
	int s;

	d = XOpenDisplay(NULL);
	if (d == NULL) {
		fprintf(stderr, "Cannot open display\n");
		exit(1);
	}

	s = DefaultScreen(d);
	w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, 100, 100, 1, BlackPixel(d, s), WhitePixel(d, s));
	XStoreName(d, w, "Example X11"); // Set Window Manager Title Text
	XMapWindow(d, w);
	g = XCreateGC(d, w, 0, 0);
	XSetBackground(d, g, 0x00000000);
	XSelectInput(d, w, KeyPressMask | KeyReleaseMask |
	                   ButtonPressMask | ButtonReleaseMask |
	                   PointerMotionMask |
	                   ExposureMask |
	                   StructureNotifyMask);

	// Capture WM close button click in addition to the above selected input events
	Atom wm_delete_window = XInternAtom(d, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(d, w, &wm_delete_window, 1);

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

				unsigned int key_code = event.xkey.keycode;
				// TODO: Handle Key Release
			} else if (event.type == KeyPress) {
				unsigned int key_code = event.xkey.keycode;
				// TODO: Handle Key Press
				goto close_application;
			} else if (event.type == ButtonPress) {
				// TODO: Handle Button Press
			} else if (event.type == Expose) {
				// Redraw the frame
				const char *msg = "Hello, World!";
				XDrawString(d, w, DefaultGC(d, s), 10, 50, msg, strlen(msg));
				XFillRectangle(d, w, DefaultGC(d, s), 20, 20, 10, 10);
			} else if (event.type == ConfigureNotify) {
				// Handle window resize
			} else if (event.type == ClientMessage) {
				// Handle window close by window manager close button click
				if ((Atom)event.xclient.data.l[0] == wm_delete_window) {
					goto close_application;
				}
			}
		}
	}
	close_application:

	XFreeGC(d, g);
	XUnmapWindow(d, w);
	XDestroyWindow(d, w);
	XCloseDisplay(d);

	return 0;
}
