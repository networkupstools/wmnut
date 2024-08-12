/*
	Best viewed with vim5, using ts=4

	wmgeneral was taken from wmppp.

	It has a lot of routines which most of the wm* programs use.

	------------------------------------------------------------

	Author: Martijn Pieterse (pieterse@xs4all.nl)

	---
	CHANGES:
	---
	02/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* changed the read_rc_file to parse_rcfile,
		  as suggested by Marcelo E. Magallon
		* debugged the parse_rc file.
	30/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Ripped similar code from all the wm* programs,
		  and put them in a single file.
	12/08/2024 (Jim Klimov, jimklimov+nut@gmail.com)
		* Addressed modern compiler warnings, coding style

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <X11/Xatom.h>

#include "wmgeneral.h"

extern int	Verbose;
extern char	*upshost;
/*  extern rckeys	*wmnut_keys; */
char *wname;

  /*****************/
 /* X11 Variables */
/*****************/

Window		Root;
int			screen;
int			x_fd;
int			d_depth;
XSizeHints	mysizehints;
XWMHints	mywmhints;
Pixel		back_pix, fore_pix;
char		*Geometry = "";
Window		iconwin, win;
GC			NormalGC;
XpmIcon		wmgen;
Pixmap		pixmask;
Atom		wm_delete_window;
Atom		wm_protocols;
Display		*display;

  /*****************/
 /* Mouse Regions */
/*****************/

typedef struct {
	int		enable;
	int		top;
	int		bottom;
	int		left;
	int		right;
} MOUSE_REGION;

#define MAX_MOUSE_REGION (8)
MOUSE_REGION	mouse_region[MAX_MOUSE_REGION];


  /***********************/
 /* Function Prototypes */
/***********************/

static void GetXPM(XpmIcon *, char **);
static Pixel GetColor(char *);
void RedrawWindow(void);
void AddMouseRegion(int, int, int, int, int);
int CheckMouseRegion(int, int);
void CheckX11Events(void);

/*******************************************************************************\
|* SetWindowName                                                               *|
\*******************************************************************************/
void SetWindowName(char *name) {

	char *fullname = NULL;

	if (strcmp(name, wname)) {
		fullname = (char *)malloc(strlen(name) + strlen(wname) + 3);

		if (fullname != NULL) {
			sprintf(fullname, "%s:%s", wname, name);
			XStoreName(display, win, fullname);
			XSetIconName(display, win, fullname);
			free(fullname);
		}
		else {
			XStoreName(display, win, wname);
			XSetIconName(display, win, wname);
		}
	}
	XMapWindow(display, win);

/*	XStoreName(display, win, wname);
	XSetIconName(display, win, wname);*/
}

/*******************************************************************************\
|* GetXPM                                                                      *|
\*******************************************************************************/
static void GetXPM(XpmIcon *wmgen, char *pixmap_bytes[]) {

	XWindowAttributes	attributes;
	int					err;

	/* For the colormap */
	XGetWindowAttributes(display, Root, &attributes);

	wmgen->attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions);

	err = XpmCreatePixmapFromData(display, Root, pixmap_bytes, &(wmgen->pixmap),
					&(wmgen->mask), &(wmgen->attributes));

	if (err != XpmSuccess) {
		fprintf(stderr, "Not enough free colorcells.\n");
		exit(1);
	}
}

/*******************************************************************************\
|* GetColor                                                                    *|
\*******************************************************************************/
static Pixel GetColor(char *name) {

  XColor				color;
  XWindowAttributes	attributes;

  if (name != NULL) {
    XGetWindowAttributes(display, Root, &attributes);

    color.pixel = 0;
    if (!XParseColor(display, attributes.colormap, name, &color)) {
      fprintf(stderr, "wm.app: can't parse %s.\n", name);
    } else if (!XAllocColor(display, attributes.colormap, &color)) {
      fprintf(stderr, "wm.app: can't allocate %s.\n", name);
    }
    return color.pixel;
  }
  else
    {
      fprintf(stderr,"GetColor:name is null\n");
      return (-1);
    }
}

/*******************************************************************************\
|* flush_expose								       *|
\*******************************************************************************/
static int flush_expose(Window w) {

  XEvent 		dummy;
  int			i=0;

  while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
    i++;

  return i;
}

/*******************************************************************************\
|* RedrawWindow						       		       *|
\*******************************************************************************/
void RedrawWindow(void) {

  flush_expose(iconwin);
  XCopyArea(display, wmgen.pixmap, iconwin, NormalGC,
	    0,0, wmgen.attributes.width, wmgen.attributes.height, 0,0);
  flush_expose(win);
  XCopyArea(display, wmgen.pixmap, win, NormalGC,
	    0,0, wmgen.attributes.width, wmgen.attributes.height, 0,0);
}

/*******************************************************************************\
|* RedrawWindowXY							       *|
\*******************************************************************************/
void RedrawWindowXY(int x, int y) {

  flush_expose(iconwin);
  XCopyArea(display, wmgen.pixmap, iconwin, NormalGC,
	    x,y, wmgen.attributes.width, wmgen.attributes.height, 0,0);
  flush_expose(win);
  XCopyArea(display, wmgen.pixmap, win, NormalGC,
	    x,y, wmgen.attributes.width, wmgen.attributes.height, 0,0);
}

/*******************************************************************************\
|* AddMouseRegion                                                              *|
\*******************************************************************************/
void AddMouseRegion(int index, int left, int top, int right, int bottom) {

  if (index < MAX_MOUSE_REGION) {
    mouse_region[index].enable = 1;
    mouse_region[index].top = top;
    mouse_region[index].left = left;
    mouse_region[index].bottom = bottom;
    mouse_region[index].right = right;
  }
}

/*******************************************************************************\
|* CheckMouseRegion                                                            *|
\*******************************************************************************/
int CheckMouseRegion(int x, int y) {

  int		i;
  int		found;

  found = 0;

  for (i=0; i<MAX_MOUSE_REGION && !found; i++) {
    if (mouse_region[i].enable &&
	x <= mouse_region[i].right &&
	x >= mouse_region[i].left &&
	y <= mouse_region[i].bottom &&
	y >= mouse_region[i].top)
      found = 1;
  }
  if (!found) return -1;
  return (i-1);
}

/*******************************************************************************\
|* copyXPMArea	                                                               *|
\*******************************************************************************/
void copyXPMArea(int x, int y, int sx, int sy, int dx, int dy) {

  XCopyArea(display, wmgen.pixmap, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);

}

/*******************************************************************************\
|* copyXBMArea                                                                 *|
\*******************************************************************************/
void copyXBMArea(int x, int y, int sx, int sy, int dx, int dy) {

  XCopyArea(display, wmgen.mask, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);
}


/*******************************************************************************\
|* setMaskXY	                                                               *|
\*******************************************************************************/
void setMaskXY(int x, int y) {

	 XShapeCombineMask(display, win, ShapeBounding, x, y, pixmask, ShapeSet);
	 XShapeCombineMask(display, iconwin, ShapeBounding, x, y, pixmask, ShapeSet);
}

/*******************************************************************************\
|* openXwindow                                                                 *|
\*******************************************************************************/
void openXwindow(int argc, char *argv[], char *pixmap_bytes[], char *pixmask_bits,
		 int pixmask_width, int pixmask_height, int withdrawn) {

	unsigned int	borderwidth = 0;
	XClassHint     	classHint;
	char	       	*display_name = NULL;
	XTextProperty	name;
	XGCValues      	gcv;
	unsigned long	gcm;
#if 0
	Status			status;
#endif
	int				dummy=0, i;

	wname = PACKAGE;

	for (i=1; argv[i]; i++) {
		if ((!strcmp(argv[i], "-display")) || (!strcmp(argv[i], "-d")))
			display_name = argv[i+1];
	}

  if (!(display = XOpenDisplay(display_name))) {
    fprintf(stderr, "%s: can't open display %s\n",
	    wname, XDisplayName(display_name));
    exit(1);
  }

  if ( !withdrawn )
	borderwidth = 5;

  screen  = DefaultScreen(display);
  Root    = RootWindow(display, screen);
  d_depth = DefaultDepth(display, screen);
  x_fd    = XConnectionNumber(display);

  /* Convert XPM to XImage */
  GetXPM(&wmgen, pixmap_bytes);

  /* Create a window to hold the stuff */
  mysizehints.flags = USSize | USPosition;
  mysizehints.x = 0;
  mysizehints.y = 0;

  back_pix = GetColor("white");
  fore_pix = GetColor("black");

  XWMGeometry(display, screen, Geometry, NULL, borderwidth, &mysizehints,
	      &mysizehints.x, &mysizehints.y,&mysizehints.width,&mysizehints.height, &dummy);

  mysizehints.min_width =
    mysizehints.max_width =
    mysizehints.width = 64;

  mysizehints.min_height =
    mysizehints.max_height =
    mysizehints.height = 64;
  mysizehints.flags |= PMinSize|PMaxSize;

  win = XCreateSimpleWindow(display, Root, mysizehints.x, mysizehints.y,
			    mysizehints.width, mysizehints.height, borderwidth,
			    fore_pix, back_pix);

  iconwin = XCreateSimpleWindow(display, win, mysizehints.x, mysizehints.y,
				mysizehints.width, mysizehints.height, borderwidth,
				fore_pix, back_pix);

  /* Activate hints */
  XSetWMNormalHints(display, win, &mysizehints);
  XSetWMNormalHints(display, iconwin, &mysizehints); /* new AQ */
  classHint.res_name = wname;
  classHint.res_class = wname;
  XSetClassHint(display, win, &classHint);

  XSelectInput(display, win,
	       ButtonPressMask | ExposureMask |
	       ButtonReleaseMask | /*PointerMotionMask |*/ StructureNotifyMask);
  XSelectInput(display, iconwin,
	       ButtonPressMask | ExposureMask |
	       ButtonReleaseMask | /*PointerMotionMask |*/ StructureNotifyMask);


/*	if (XStringListToTextProperty(&fullname, 1, &name) == 0) { */
	if (XStringListToTextProperty(&wname, 1, &name) == 0) {
		fprintf(stderr, "%s: can't allocate window name\n", wname);
		exit(1);
	}

	XSetWMName(display, win, &name);
	if ( !withdrawn ) {
		XSetWMIconName(display, win, &name);
		SetWindowName(wname);
	}

  /* Create GC for drawing */

  gcm = GCForeground | GCBackground | GCGraphicsExposures;
  gcv.foreground = fore_pix;
  gcv.background = back_pix;
  gcv.graphics_exposures = 0;
  NormalGC = XCreateGC(display, Root, gcm, &gcv);

  /* ONLYSHAPE ON */

  if ( withdrawn ) {
    pixmask = XCreateBitmapFromData(display, win, pixmask_bits,
				    pixmask_width, pixmask_height);
    XShapeCombineMask(display, win, ShapeBounding, 0, 0, pixmask, ShapeSet);
    XShapeCombineMask(display, iconwin, ShapeBounding, 0, 0, pixmask, ShapeSet);
  }
  /* ONLYSHAPE OFF */

  mywmhints.initial_state = withdrawn ? WithdrawnState : NormalState;
  mywmhints.icon_window = iconwin;
  mywmhints.flags = StateHint | IconWindowHint;

  if ( withdrawn ) {
    mywmhints.window_group = win;
    mywmhints.flags |= WindowGroupHint | IconPositionHint;
    mywmhints.icon_x = mysizehints.x;
    mywmhints.icon_y = mysizehints.y;
  }

  XSetWMHints(display, win, &mywmhints);
  XSetCommand(display, win, argv, argc);

  /* Set up the event for quitting the window */
  wm_delete_window = XInternAtom(display,
				 "WM_DELETE_WINDOW",	/* atom_name */
				 False);		/* only_if_exists */

  wm_protocols = XInternAtom(display,
			     "WM_PROTOCOLS",		/* atom_name */
			     False);			/* only_if_exists */

#if 0
  status =
#endif
  XSetWMProtocols(display, win, &wm_delete_window, 1);

#if 0
  status =
#endif
  XSetWMProtocols(display, iconwin, &wm_delete_window, 1);

  XMapWindow(display, win);

}

/*
 * This function clears up all X related
 * stuff and exits. It is called in case
 * of emergencies too.
 */
void Cleanup(void)
{
  /* do not clean on FreeBSD ! */
#ifndef FreeBSD
  if ( display )
    {
      XCloseDisplay(display);
    }
#endif

  /* Free all mallocs */
  CleanHosts();

  exit(0);
}

/*
 * This checks for X11 events. We distinguish the following:
 * - request to repaint the window
 * - request to quit (Close button)
 * - mouse request (click)
 */
void CheckX11Events(void)
{
  XEvent Event;

  while (XPending(display)) {
    XNextEvent(display, &Event);
    switch(Event.type) {
      case Expose:
	RedrawWindow();
	break;
      case ButtonPress:
	pressEvent(&Event.xbutton);
	break;
      case ButtonRelease:
	break;
      case ClientMessage:
	if ((Event.xclient.message_type == wm_protocols)
	    && (Event.xclient.data.l[0] == wm_delete_window))
	  Cleanup();
	break;
    }
  }
}

/*
 *  This routine handles button presses. Pressing the '<|' button
 *  invokes 'previous UPS' to change the monitored UPS (if multiple)
 *  pressing the '|>' button invokes 'next UPS'...
 *
 */
void pressEvent(XButtonEvent *xev){

  int x=xev->x;
  int y=xev->y;

  if(x>=5 && y>=48 && x<=17 && y<=58){

    /*
     *  look if multiple hosts are monitored
     */

    /*
     *  Standby Call.
     *
     *  Draw button as 'pushed'. Redraw window to show it.
     *  Call 'nut -S' to standby. Sleep for 2 seconds so that
     *  the button doesnt immediately redraw back to unpressed
     *  before the 'nut -S' takes effect.
     */
    copyXPMArea(5, 106, 13, 11, 5, 48);
    RedrawWindow();

    GetPrevHost();

    //	system("nut -S");

  } else if (x>=46 && y>=48 && x<=58 && y<=58){

    /*
     *  Suspend Call.
     *
     *  Draw button as 'pushed'. Redraw window to show it.
     *  Call 'nut -s' to suspend. Sleep for 2 seconds so that
     *  the button doesnt immediately redraw back to unpressed
     *  before the 'nut -s' takes effect.
     */
    copyXPMArea(21, 106, 13, 11, 46, 48);
    RedrawWindow();

    GetNextHost();

    //	system("nut -s");
  }

  //usleep(2000000L);
  usleep(500L);

  return;
}
