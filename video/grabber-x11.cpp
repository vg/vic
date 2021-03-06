/*
 * Copyright (c) 1998 Luigi Rizzo
 * grabber-x11.cpp for vic
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "grabber.h"
#include "vic_tcl.h"
#include "device-input.h"
#include "module.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <tk.h>

/*
	Netvideo version 3.3
	Written by Ron Frederick <frederick@parc.xerox.com>

	Machine-specific sized integer type definitions
	Video utility definitions
*/

/*
 * Copyright (c) Xerox Corporation 1992. All rights reserved.
 *  
 * License is granted to copy, to use, and to make and to use derivative
 * works for research and evaluation purposes, provided that Xerox is
 * acknowledged in all documentation pertaining to any such copy or derivative
 * work. Xerox grants no other licenses expressed or implied. The Xerox trade
 * name should not be used in any advertising without its written permission.
 *  
 * XEROX CORPORATION MAKES NO REPRESENTATIONS CONCERNING EITHER THE
 * MERCHANTABILITY OF THIS SOFTWARE OR THE SUITABILITY OF THIS SOFTWARE
 * FOR ANY PARTICULAR PURPOSE.  The software is provided "as is" without
 * express or implied warranty of any kind.
 *  
 * These notices must be retained in any copies of any part of this software.
 */

typedef signed char	int8;		/*  8 bit signed int */
typedef short		int16;		/* 16 bit signed int */
typedef int		int32;		/* 32 bit signed int */
#if defined(__alpha)
typedef long		int64;		/* 64 bit signed int */
#endif

typedef unsigned char	uint8;		/*  8 bit unsigned int */
typedef unsigned short	uint16;		/* 16 bit unsigned int */
typedef unsigned int	uint32;		/* 32 bit unsigned int */
#if defined(__alpha)
typedef unsigned long	uint64;		/* 64 bit unsigned int */
#endif

extern int use_shm;	/* from main.cc */

/* Mildly gross but moderately portable test for littleendian machines */
#define LITTLEENDIAN (ntohl(0x12345678) != 0x12345678)

typedef struct {
    XImage	*image;
    void	*shminfo;
} ximage_t;

/*************************/

#define VID_SMALL               0x01
#define VID_MEDIUM              0x02
#define VID_LARGE               0x04
#define VID_SIZEMASK            0x07 
 
#define VID_GREYSCALE           0x08
#define VID_COLOR               0x10

#define X11GRAB_FIXED   0 
#define X11GRAB_POINTER 1
#define X11GRAB_WINDOW  2
 

/*XXX*/
#define NTSC_WIDTH 320
#define NTSC_HEIGHT 240
#define PAL_WIDTH 384
#define PAL_HEIGHT 288
#define CIF_WIDTH 352
#define CIF_HEIGHT 288


class X11Grabber : public Grabber {
 public:
	X11Grabber(const char* name, const char* format);
	virtual ~X11Grabber();
	virtual void start();
	virtual void stop();
 protected:
	virtual int command(int argc, const char*const* argv);
	virtual int capture();
	virtual int grab();
	void format();
	void setsize();

	void X11Grab_ComputeYUVTable(void) ;
	int X11Grab_MSBWhite1(void);
	int X11Grab_LSBWhite1(void);
	int X11Grab_MSBBlack1(void);
	int X11Grab_LSBBlack1(void);
	int X11Grab_Pseudo8(void);
	int X11Grab_RGB16(void);
	int X11Grab_TrueXBGR24(void);
	int X11Grab_TrueXRGB24(void);

    	int X11Grab_Initialize(Window rw, int w, int h);
        int (X11Grabber::*c_grab)(void);

	uint8 *rgb2y_ ;
	uint8 *rgb2u_ ;
	uint8 *rgb2v_ ;

        ximage_t *ximage_ ;

	Display *dpy_ ;
	int mode_;		/* input mode */
	Window theroot_ ;

/*	Tcl_Interp *interp_=NULL;*/

	int screen, xerror ;
  	Window vRoot_ ;
	Window rootwin_ ;
	Colormap colormap;
	Visual *root_vis;
	XVisualInfo root_visinfo;
	 
	int ncolors_ ;
	int black, white;
	XColor *color ;
	uint8 *col2y_ ;
	uint16 *col2rgb16_ ;

	u_int basewidth_;	/* Height of frame to be captured */
	u_int baseheight_;	/* Width of frame to be captured */
	u_int decimate_;	/* division of base sizes */

	int x_origin_, y_origin_, width_, height_;
	int root_depth_, root_width, root_height;
};

class X11Device : public InputDevice {
 public:
	X11Device(const char* nickname);
	virtual int command(int argc, const char*const* argv);
 protected:
	const char* name_;
};

static X11Device find_x11_devices("x11");

X11Device::X11Device(const char* nickname):
			InputDevice(nickname), name_(nickname)
{
	//if (free)
	if (name_) //SV-XXX: Debian
		attributes_ = "\
size {large normal small cif} \
format {420}" ;
	else
		attributes_ = "disabled";
}

extern "C" {
/*** most of this taken from nv:x11-grab.c  ***/
extern ximage_t *VidUtil_AllocXImage(Display *dpy, Visual *vis, int depth,
				     int width, int height, int readonly);
extern void VidUtil_DestroyXImage(Display *dpy, ximage_t *ximage);

#if 0 /* debugging stuff */
static int my_Tcl_Eval(Tcl_Interp *interp, char *cmd)
{
	fprintf(stderr,"Tcl_Eval <%s>\n", cmd);
	Tcl_Eval(interp, cmd);
}
#define	Tcl_Eval my_Tcl_Eval
#endif

static Window
VirtualRootWindow(Display *dpy, int screen)
{
    static Display *last_dpy=(Display *)NULL;
    static int last_screen = -1;    
    static Window vRoot=None;

    Atom __SWM_VROOT=None;
    Window rw, p, *child;
    unsigned int i,  nChildren;

    if ((dpy != last_dpy) || (screen != last_screen)) {
        vRoot = RootWindow(dpy, screen);

        /* go look for a virtual root */
        __SWM_VROOT = XInternAtom(dpy, "__SWM_VROOT", False);
        XQueryTree(dpy, vRoot, &rw, &p, &child, &nChildren);
        for (i=0; i<nChildren; i++) {
            Atom actual_type;
            int actual_format;
            unsigned long nitems, bytesafter;
            Window *newRoot=NULL;
    
            if ((XGetWindowProperty(dpy, child[i], __SWM_VROOT, 0, 1, False,
                                    XA_WINDOW, &actual_type, &actual_format,
                                    &nitems, &bytesafter,
                                    (unsigned char **)&newRoot) == Success)
                && (newRoot != NULL)) {
                vRoot = *newRoot;
                XFree((void *)newRoot);
                break;
            }
        }
        XFree((void *)child);

        last_dpy = dpy;
        last_screen = screen;
    }

    return vRoot;
}

} /* end extern C */


extern "C" {

#include <sys/ipc.h>
#ifdef USE_SHM
#include <sys/shm.h>
#if defined(sun) && !defined(__svr4__)
int shmget(key_t, int, int);
char *shmat(int, char*, int);
int shmdt(char*);
int shmctl(int, int, struct shmid_ds*);
#endif
#ifdef __osf__
int XShmGetEventBase(struct _XDisplay *);
#else
int XShmGetEventBase(Display *);
#endif
#ifdef sgi
#define XShmAttach __XShmAttach__
#define XShmDetach __XShmDetach__
#define XShmPutImage __XShmPutImage__
#endif
#include <X11/extensions/XShm.h>
#ifdef sgi
#undef XShmAttach
#undef XShmDetach
#undef XShmPutImage
int XShmAttach(Display*, XShmSegmentInfo*);
int XShmDetach(Display*, XShmSegmentInfo*);
int XShmPutImage(Display*, Drawable, GC, XImage*, int, int, int, int,
                 int, int, int);
#endif
#endif


/*ARGSUSED*/
static int
ErrHandler(ClientData clientData, XErrorEvent *errevp)
{
    //SV-XXX: unused
    UNUSED(clientData);
    UNUSED(errevp);

    return 0;
}

ximage_t *
VidUtil_AllocXImage(Display *dpy, Visual *vis, int depth, int width,
			      int height, int readonly)
{
    ximage_t *ximage;
    int ximage_size;
    Tk_ErrorHandler handler;

    ximage = (ximage_t *) malloc(sizeof(ximage_t));
    if (ximage == NULL)
	return NULL;

#ifdef USE_SHM
    if (use_shm) {
	XShmSegmentInfo *shminfo;

	ximage->shminfo = shminfo =
	    (XShmSegmentInfo *) malloc(sizeof(XShmSegmentInfo));

	ximage->image = XShmCreateImage(dpy, vis, depth, ZPixmap, 0, shminfo,
					width, height);
	ximage_size = ximage->image->bytes_per_line * ximage->image->height;

	shminfo->shmid = shmget(IPC_PRIVATE, ximage_size, IPC_CREAT|0777);
	if (shminfo->shmid != -1) {
	    shminfo->shmaddr = ximage->image->data =
		(char *) shmat(shminfo->shmid, 0, 0);
	    shminfo->readOnly = readonly;

	    handler = Tk_CreateErrorHandler(dpy, -1, -1, -1, ErrHandler, NULL);
	    XShmAttach(dpy, shminfo);
	    XSync(dpy, False);
	    shmctl(shminfo->shmid, IPC_RMID, 0); /* so it goes away on exit */
	    Tk_DeleteErrorHandler(handler);
	    if (0) { /* so it goes away on exit... */
		shmdt(shminfo->shmaddr);
		shmctl(shminfo->shmid, IPC_RMID, 0);
		XDestroyImage(ximage->image);
		free(shminfo);
	    }
	    return ximage;
	} else {
	    XDestroyImage(ximage->image);
	    free(shminfo);
	    ximage->shminfo = NULL ;
	    /* XXX hmmm... something more ? */
	}
    }
#endif
    {
	ximage->image = XCreateImage(dpy, vis, depth, ZPixmap, 0, NULL, width,
				     height, (depth == 24) ? 32 : depth, 0);
	ximage_size = ximage->image->bytes_per_line * ximage->image->height;
	ximage->image->data = (char *) malloc(ximage_size);

	ximage->shminfo = NULL;
    }

    return ximage;
}

void
VidUtil_DestroyXImage(Display *dpy, ximage_t *ximage)
{
#ifdef USE_SHM
    if (use_shm && ximage->shminfo != NULL) {
	XShmSegmentInfo *shminfo=(XShmSegmentInfo *)ximage->shminfo;

	XShmDetach(dpy, shminfo);
	shmdt(shminfo->shmaddr);
	shmctl(shminfo->shmid, IPC_RMID, 0);
	free(shminfo);
    }
    ximage->shminfo = NULL ;
#endif

    XDestroyImage(ximage->image);
    free(ximage);
}


} /* end extern "C" block */


void
X11Grabber::X11Grab_ComputeYUVTable(void)
{
    int i;

    switch (root_visinfo.c_class) {
    case StaticColor:
    case PseudoColor:
    case StaticGray:
    case GrayScale:
        for (i=0; i<ncolors_; i++) color[i].pixel = i;
        XQueryColors(dpy_, colormap, color, ncolors_);
        for (i=0; i<ncolors_; i++) {
            color[i].red = (color[i].red & 0xf800) ;
            color[i].green = (color[i].green & 0xfc00) >> 5 ;
            color[i].blue = (color[i].blue & 0xf800) >> 11 ;
            col2rgb16_[i] = color[i].red + color[i].green + color[i].blue;
            col2y_[i] = rgb2y_[col2rgb16_[i]];
        }
        break;
    case TrueColor:
	//fprintf(stderr, "TrueColor...\n");
        break;
    case DirectColor:
	//fprintf(stderr, "DirectColor...\n");
        break;
    }
}

/*
 * these are the grabbing functions for the various video formats
 */

int
X11Grabber::X11Grab_MSBWhite1()
{
    int x, y, row;
    uint8 *data=(uint8 *)ximage_->image->data, *yp= frame_;

    for (y=0; y<height_; y++) {
        for (x=0; x<width_; x+=8) {
            row = *data++;

            yp[0] = 255 * ((row & 0x80)>>7);
            yp[1] = 255 * ((row & 0x40)>>6);
            yp[2] = 255 * ((row & 0x20)>>5);
            yp[3] = 255 * ((row & 0x10)>>4);
            yp[4] = 255 * ((row & 0x08)>>3);
            yp[5] = 255 * ((row & 0x04)>>2);
            yp[6] = 255 * ((row & 0x02)>>1);
            yp[7] = 255 *  (row & 0x01);
            yp += 8;
        }
    }

    return 1;
}

int
X11Grabber::X11Grab_MSBBlack1()
{
    int x, y, row;
    uint8 *data=(uint8 *)ximage_->image->data, *yp= frame_;

    for (y=0; y<height_; y++) {
        for (x=0; x<width_; x+=8) {
            row = *data++;

            yp[0] = 255 - 255 * ((row & 0x80)>>7);
            yp[1] = 255 - 255 * ((row & 0x40)>>6);
            yp[2] = 255 - 255 * ((row & 0x20)>>5);
            yp[3] = 255 - 255 * ((row & 0x10)>>4);
            yp[4] = 255 - 255 * ((row & 0x08)>>3);
            yp[5] = 255 - 255 * ((row & 0x04)>>2);
            yp[6] = 255 - 255 * ((row & 0x02)>>1);
            yp[7] = 255 - 255 *  (row & 0x01);
            yp += 8;
        }
    }

    return 1;
}

int
X11Grabber::X11Grab_LSBWhite1()
{
    int x, y, row;
    uint8 *data=(uint8 *)ximage_->image->data, *yp= frame_ ;

    for (y=0; y<height_; y++) {
        for (x=0; x<width_; x+=8) {
            row = *data++;

            yp[7] = 255 * ((row & 0x80)>>7);
            yp[6] = 255 * ((row & 0x40)>>6);
            yp[5] = 255 * ((row & 0x20)>>5);
            yp[4] = 255 * ((row & 0x10)>>4);
            yp[3] = 255 * ((row & 0x08)>>3);
            yp[2] = 255 * ((row & 0x04)>>2);
            yp[1] = 255 * ((row & 0x02)>>1);
            yp[0] = 255 *  (row & 0x01);
            yp += 8;
        }
    }

    return 1;
}

int
X11Grabber::X11Grab_LSBBlack1()
{
    int x, y, row;
    uint8 *data=(uint8 *)ximage_->image->data, *yp= frame_;

    for (y=0; y<height_; y++) {
        for (x=0; x<width_; x+=8) {
            row = *data++;

            yp[7] = 255 - 255 * ((row & 0x80)>>7);
            yp[6] = 255 - 255 * ((row & 0x40)>>6);
            yp[5] = 255 - 255 * ((row & 0x20)>>5);
            yp[4] = 255 - 255 * ((row & 0x10)>>4);
            yp[3] = 255 - 255 * ((row & 0x08)>>3);
            yp[2] = 255 - 255 * ((row & 0x04)>>2);
            yp[1] = 255 - 255 * ((row & 0x02)>>1);
            yp[0] = 255 - 255 *  (row & 0x01);
            yp += 8;
        }
    }

    return 1;
}

int
X11Grabber::X11Grab_Pseudo8()
{
    int x, y, p0, p1, p2, p3 ;
    uint8 *data=(uint8 *)ximage_->image->data, *yp=frame_ ;
    uint8 *up= (uint8 *)yp + framesize_ ;
    uint8 *vp= up + (framesize_ >> 2) ;

    X11Grab_ComputeYUVTable();

    for (y=0; y<height_; y += 2) {
        for (x=0; x<width_ ; x += 2) {
            yp[0] = col2y_[data[0]];
            p0 = col2rgb16_[data[0]];
            yp[1] = col2y_[data[1]];
            p1 = col2rgb16_[data[1]];

            p2 = col2rgb16_[data[width_]];
            p3 = col2rgb16_[data[width_ + 1]];
#if 0 /* average */
  	    p0 = ( (p0 >> 1) & 0x7bef ) + ( (p1 >> 1) & 0x7bef ) ;
  	    p2 = ( (p2 >> 1) & 0x7bef ) + ( (p3 >> 1) & 0x7bef ) ;
  	    p0 = ( (p0 >> 1) & 0x7bef ) + ( (p2 >> 1) & 0x7bef ) ;
#else /* take the darkest... */
	    if (yp[1] < yp[0]) p0 = p1 ;
	    if (rgb2y_[p2] < rgb2y_[p0]) p0 = p2 ;
	    if (rgb2y_[p3] < rgb2y_[p0]) p0 = p3 ;
#endif
	    *up++ = rgb2u_[ p0 ];
	    *vp++ = rgb2v_[ p0 ];

            data += 2;
	    yp += 2 ;
        }
        for (x=0; x<width_; x += 8) {
            yp[0] = col2y_[data[0]];
            yp[1] = col2y_[data[1]];
            yp[2] = col2y_[data[2]];
            yp[3] = col2y_[data[3]];
            yp[4] = col2y_[data[4]];
            yp[5] = col2y_[data[5]];
            yp[6] = col2y_[data[6]];
            yp[7] = col2y_[data[7]];
            data += 8;
	    yp += 8 ;
        }
    }

    return 1;
}

int
X11Grabber::X11Grab_RGB16(void)
{
    int x, y;
    uint8 *yp= (uint8 *)frame_ ;
    uint8 *up= (uint8 *)yp + framesize_ ;
    uint8 *vp= up + (framesize_ >> 2) ;
    uint16 *data=(uint16 *)ximage_->image->data, p0, p1, p2, p3;

    for (y=0; y<height_; y+=2) {
        for (x=0; x<width_; x += 2) {
            p0 = data[0] ;
	    p1 = data[1] ;
	    p2 = data[ width_] ;
	    p3 = data[ width_ + 1] ;
	    data += 2 ;
            yp[0] = rgb2y_[ p0 ] ; /* in 565 format */
            yp[1] = rgb2y_[ p1 ] ; /* in 565 format */
#if 0
	    /* average the four pixels... */
  	    p0 = ( (p0 >> 1) & 0x7bef ) + ( (p1 >> 1) & 0x7bef ) ;
  	    p2 = ( (p2 >> 1) & 0x7bef ) + ( (p3 >> 1) & 0x7bef ) ;
  	    p0 = ( (p0 >> 1) & 0x7bef ) + ( (p2 >> 1) & 0x7bef ) ;
#else /* take the darkest... */
	    if (yp[1] < yp[0]) p0 = p1 ;
	    if (rgb2y_[p2] < rgb2y_[p0]) p0 = p2 ;
	    if (rgb2y_[p3] < rgb2y_[p0]) p0 = p3 ;
#endif
	    *up++ = rgb2u_[ p0 ];
	    *vp++ = rgb2v_[ p0 ];
	    yp += 2 ;
	}
        for (x=0; x<width_; x += 8) {
	    yp[0] = rgb2y_[data[0] ];
	    yp[1] = rgb2y_[data[1] ];
	    yp[2] = rgb2y_[data[2] ];
	    yp[3] = rgb2y_[data[3] ];
	    yp[4] = rgb2y_[data[4] ];
	    yp[5] = rgb2y_[data[5] ];
	    yp[6] = rgb2y_[data[6] ];
	    yp[7] = rgb2y_[data[7] ];
	    yp += 8 ;
	    data += 8 ;
        }
    }

    return 1;
}

int
X11Grabber::X11Grab_TrueXBGR24()
{
    int x, y;
    uint8 *yp= (uint8 *)frame_ ;
    uint8 *up= (uint8 *)yp + framesize_ ;
    uint8 *vp= up + (framesize_ >> 2) ;
    uint16 p0, p1 ;
    uint32 d ;

    for (y=0; y<height_; y++) {
        for (x=0; x<width_; x+=2) {
            d = XGetPixel(ximage_->image,x,y);
            //p0 = ((d<<8) & 0xf100) | ((p0>>5) & 0x7e0) | ((p0>>19) & 0x1f);
            p0 = ((d<<8) & 0xf800) | ((d>>5) & 0x7e0) | ((d>>19) & 0x1f);
	    *yp++ = rgb2y_[ p0 ];

            d = XGetPixel(ximage_->image,x+1,y);
            //p1 = ((d<<8) & 0xf100) | ((p0>>5) & 0x7e0) | ((p0>>19) & 0x1f);
            p1 = ((d<<8) & 0xf800) | ((d>>5) & 0x7e0) | ((d>>19) & 0x1f);
	    *yp++ = rgb2y_[ p1 ];

	    /* average the two pixels... */
  	    p0 = ( (p0 >> 1) & 0x7bef ) + ( (p1 >> 1) & 0x7bef ) ;
	    *up++ = rgb2u_[ p0 ];
        }
	y++;
        for (x=0; x<width_; x+=2) {
            d = XGetPixel(ximage_->image,x,y);
            //p0 = ((d<<8) & 0xf100) | ((p0>>5) & 0x7e0) | ((p0>>19) & 0x1f);
            p0 = ((d<<8) & 0xf800) | ((d>>5) & 0x7e0) | ((d>>19) & 0x1f);
	    *yp++ = rgb2y_[ p0 ];

            d = XGetPixel(ximage_->image,x+1,y);
            //p1 = ((d<<8) & 0xf100) | ((p0>>5) & 0x7e0) | ((p0>>19) & 0x1f);
            p1 = ((d<<8) & 0xf800) | ((d>>5) & 0x7e0) | ((d>>19) & 0x1f);
	    *yp++ = rgb2y_[ p1 ];

	    /* average the two pixels... */
  	    p0 = ( (p0 >> 1) & 0x7bef ) + ( (p1 >> 1) & 0x7bef ) ;
	    *vp++ = rgb2v_[ p0 ];
        }
    }
    return 1;
}

/* X11Grab_TrueXRGB24() by Davide Cavagnino
			   Dipartimento di Informatica
			   Universita' degli Studi di Torino
   Fixes some bugs in X11Grab_TrueXBRG24() and makes it work for
   true color RGB devices
*/
int
X11Grabber::X11Grab_TrueXRGB24()
{
    int x, y;
    uint8 *yp= (uint8 *)frame_ ;
    uint8 *up= (uint8 *)yp + framesize_ ;
    uint8 *vp= up + (framesize_ >> 2) ;
    uint16 p0, p1 ;
    uint32 d ;

    for (y=0; y<height_; y++) {
        for (x=0; x<width_; x+=2) {
            d = XGetPixel(ximage_->image,x,y);
	    p0 = ((d>>3) & 0x1f) | ((d>>5) & 0x7e0) | ((d>>8) & 0xf800);
	    *yp++ = rgb2y_[ p0 ];

            d = XGetPixel(ximage_->image,x+1,y);
	    p1 = ((d>>3) & 0x1f) | ((d>>5) & 0x7e0) | ((d>>8) & 0xf800);
	    *yp++ = rgb2y_[ p1 ];

	    /* average the two pixels... */
  	    p0 = ( (p0 >> 1) & 0x7bef ) + ( (p1 >> 1) & 0x7bef ) ;
	    *up++ = rgb2u_[ p0 ];
        }
	y++;
        for (x=0; x<width_; x+=2) {
            d = XGetPixel(ximage_->image,x,y);
	    p0 = ((d>>3) & 0x1f) | ((d>>5) & 0x7e0) | ((d>>8) & 0xf800);
	    *yp++ = rgb2y_[ p0 ];

            d = XGetPixel(ximage_->image,x+1,y);
	    p1 = ((d>>3) & 0x1f) | ((d>>5) & 0x7e0) | ((d>>8) & 0xf800);
	    *yp++ = rgb2y_[ p1 ];

	    /* average the two pixels... */
  	    p0 = ( (p0 >> 1) & 0x7bef ) + ( (p1 >> 1) & 0x7bef ) ;
	    *vp++ = rgb2v_[ p0 ];
        }
    }
    return 1;
}

/*
 * initialization of frame grabber...
 */
int
X11Grabber::X11Grab_Initialize(Window rw, int w, int h)
{   
    int config = 0 ;
    XWindowAttributes wattr;
    
    if (theroot_ != rw) {
        theroot_ = rw;    
        XGetWindowAttributes(dpy_, theroot_, &wattr);
        screen = XScreenNumberOfScreen(wattr.screen);
        colormap = DefaultColormapOfScreen(wattr.screen);
        ncolors_ = CellsOfScreen(wattr.screen);
        black = BlackPixelOfScreen(wattr.screen);
        white = WhitePixelOfScreen(wattr.screen);
        root_depth_ = wattr.depth;
        root_width = wattr.width;
        root_height = wattr.height;
        root_vis = wattr.visual;
        vRoot_ = VirtualRootWindow(dpy_, screen);

        if (color != NULL) {
	    free(color);
	    free(col2y_);
	    free(col2rgb16_);
	}
        color = (XColor *) malloc(ncolors_*sizeof(XColor));
        col2y_ = (uint8 *) malloc(ncolors_*sizeof(uint8));
        col2rgb16_ = (uint16 *) malloc(ncolors_*sizeof(uint16));

        XMatchVisualInfo(dpy_, screen, root_depth_, root_vis->c_class,
                         &root_visinfo);
        switch (root_depth_) {
        case 1:
            if (white == 1) {
                c_grab = (LITTLEENDIAN) ? &X11Grabber::X11Grab_LSBWhite1 : &X11Grabber::X11Grab_MSBWhite1;
            } else {
                c_grab = (LITTLEENDIAN) ? &X11Grabber::X11Grab_LSBBlack1 : &X11Grabber::X11Grab_MSBBlack1;
            }
            config = VID_GREYSCALE;
            break;

        case 8:
            switch (root_visinfo.c_class) {
            case PseudoColor:
            case GrayScale:
            case StaticColor:
            case StaticGray:
                c_grab = &X11Grabber::X11Grab_Pseudo8;
                break;
            default:
                c_grab = (int)NULL;
                break;
            }
            config = VID_GREYSCALE|VID_COLOR;
            break;

        case 16:
	    c_grab = &X11Grabber::X11Grab_RGB16;
	    break ;

        case 24:
	    /* due to endianess we have two different 24 bit depth pixel
	     * layouts. little endian uses BGR, big uses RGB.
	     */
            if ((root_visinfo.c_class == TrueColor) &&
                (root_visinfo.blue_mask  == 0xff0000) &&
                (root_visinfo.green_mask == 0x00ff00) &&
                (root_visinfo.red_mask   == 0x0000ff)
            ) {
                c_grab = &X11Grabber::X11Grab_TrueXBGR24;
            }
	    else if ((root_visinfo.c_class == TrueColor) &&
		     (root_visinfo.red_mask == 0xff0000) &&
		     (root_visinfo.green_mask == 0xff00) &&
		     (root_visinfo.blue_mask == 0xff))
	    {
                c_grab = &X11Grabber::X11Grab_TrueXRGB24;
            } else {
	        fprintf(stderr, "don't know how to grab %d bits\n",
	        	root_depth_);
                c_grab = (int)NULL;
	    }
            config = VID_GREYSCALE|VID_COLOR;
            break;

        default:
	    fprintf(stderr, "don't know how to grab %d bits\n",
		root_depth_);
            c_grab = (int)NULL;
            break;
        }
    }
    
    if ((ximage_ == NULL) || (width_ != w) || (height_ != h)) {
        width_ = w;
        height_ = h;
        if (ximage_ != NULL)
	    VidUtil_DestroyXImage(dpy_, ximage_);
        ximage_ = VidUtil_AllocXImage(dpy_, root_vis, root_depth_, w, h, False);
    }
    return (c_grab == NULL) ? 0 : config|VID_SMALL|VID_MEDIUM|VID_LARGE;
}

extern "C" {
extern void VidUtil_Init(Display *dpy);
extern void VidUtil_DestroyXImage(Display *dpy, ximage_t *ximage);

#ifdef UNUSED__ /* not yet... */
static int
ErrHandler1(ClientData clientData, XErrorEvent *errevp)
{
    xerror = 1;
    return 0;
}

static int
X11Grab_MakeBox(unsigned int x1, unsigned int y1,
                           unsigned int x2, unsigned int y2,
                           int *xp, int *yp, int *wp, int *hp)
{
    int w, h;

    w = x2-x1;
    if (w < 0) {
        *xp = x2;
        *wp = -w;
    } else {
        *xp = x1;
        *wp = w;
    }
    
    h = y2-y1;
    if (h < 0) {
        *yp = y2;
        *hp = -h;
    } else {
        *yp = y1;
        *hp = h;
    }
}   

static int
X11Grab_UpdatePos(Window rw, int x, int y, int w, int h)
{
    static char cmd[256];

    if (w < 8) w = 8;
    if (h < 8) h = 8;

    if (w > root_width/8*8) w = root_width/8*8;
    if (h > root_height/8*8) h = root_height/8*8;

    w = (w+7)/8*8;
    h = (h+7)/8*8;

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    if (x > root_width-w) x = root_width-w;
    if (y > root_height-h) y = root_height-h;

    sprintf(cmd, "x11grabUpdatePos %d %d %d %d", x, y, w, h);
    (void) Tcl_Eval(interp, cmd);

    x_origin = x;
    y_origin = y;

    if ((root != rw) || (width != w) || (height != h)) {
        X11Grab_Initialize(rw, w, h);
        return 0;
    } else
        return 1;
}

static int
X11Grab_FollowPointer(void)
{
    Window rw, cw;
    int x, y, wx, wy;
    unsigned int mask;

    XQueryPointer(dpy, root, &rw, &cw, &x, &y, &wx, &wy, &mask);

    if (x < x_origin+width/4)
        x = x-width/4;
    else if (x >= x_origin+3*width/4)
        x = x-3*width/4;
    else
        x = x_origin;
    
    if (y < y_origin+height/4)
        y = y-height/4;
    else if (y >= y_origin+3*height/4)
        y = y-3*height/4;
    else
        y = y_origin;
    
    return X11Grab_UpdatePos(rw, x, y, width, height);
}

static int
X11Grab_FollowWindow(void)
{
    int x, y, w, h;
    XWindowAttributes wattr, vRoot_wattr;
    Tk_ErrorHandler handler;

    handler = Tk_CreateErrorHandler(dpy, -1, -1, -1, ErrHandler1, NULL);
    xerror = 0;
    XGetWindowAttributes(dpy, target, &wattr);
    XSync(dpy, False);
    Tk_DeleteErrorHandler(handler);
    if ((target == None) || xerror) {
        target = None;
        (void) Tcl_Eval(interp,
               ".grabControls.x11grab.row1.mode.window config -state disabled");
        (void) Tcl_Eval(interp, "set x11grabMode fixed");
        return 1;
    } else {
        XGetWindowAttributes(dpy, vRoot, &vRoot_wattr);
        x = wattr.x+vRoot_wattr.x;
        y = wattr.y+vRoot_wattr.y;
        w = wattr.width+2*wattr.border_width;
        h = wattr.height+2*wattr.border_width;

        return X11Grab_UpdatePos(root, x, y, w, h);
    }
}   
#endif /* UNUSED ... */


#ifdef UNUSED__
/*ARGSUSED*/
static int
X11Grab_SetXCmd(ClientData clientData, Tcl_Interp *interp,
                           int argc, char *argv[])
{
    int x;

    if (argc != 2) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
            " x\"", NULL);
        return TCL_ERROR;
    }
    
    x = atoi(argv[1]);
    (void) X11Grab_UpdatePos(root, x, y_origin, width, height);

    return TCL_OK;
}

/*ARGSUSED*/
static int
X11Grab_SetYCmd(ClientData clientData, Tcl_Interp *interp,
                           int argc, char *argv[])
{
    int y;

    if (argc != 2) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
            " y\"", NULL);
        return TCL_ERROR;
    }
    
    y = atoi(argv[1]);
    (void) X11Grab_UpdatePos(root, x_origin, y, width, height);

    return TCL_OK;
}

/*ARGSUSED*/
static int X11Grab_SetWCmd(ClientData clientData, Tcl_Interp *interp,
                           int argc, char *argv[])
{
    int w;

    if (argc != 2) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
            " width\"", NULL);
        return TCL_ERROR;
    }
    
    w = atoi(argv[1]);
    (void) X11Grab_UpdatePos(root, x_origin, y_origin, w, height);

    return TCL_OK;
}

/*ARGSUSED*/
static int
X11Grab_SetHCmd(ClientData clientData, Tcl_Interp *interp,
                           int argc, char *argv[])
{
    int h;

    if (argc != 2) {
        Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
            " height\"", NULL);
        return TCL_ERROR;
    }
    
    h = atoi(argv[1]);
    (void) X11Grab_UpdatePos(root, x_origin, y_origin, width, h);

    return TCL_OK;
}

/*ARGSUSED*/
static int
X11Grab_SetRegionCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, char *argv[])
{
    unsigned int rootx, rooty;
    int x, y, w, h, boxDrawn=0;
    GC xorGC;
    Cursor cursor;
    XEvent event;

    cursor = XCreateFontCursor(dpy, XC_cross);

    if (XGrabPointer(dpy, root, False, ButtonPressMask, GrabModeAsync,
                     GrabModeAsync, root, cursor, CurrentTime)!=GrabSuccess) {
        Tcl_AppendResult(interp, argv[0], ": can't grab mouse", NULL);
        return TCL_ERROR;
    }
    
    xorGC = XCreateGC(dpy, root, 0, NULL);
    XSetSubwindowMode(dpy, xorGC, IncludeInferiors);
    XSetForeground(dpy, xorGC, -1);
    XSetFunction(dpy, xorGC, GXxor);

    XMaskEvent(dpy, ButtonPressMask, &event);
    rootx = event.xbutton.x_root;
    rooty = event.xbutton.y_root;

    XChangeActivePointerGrab(dpy, ButtonMotionMask|ButtonReleaseMask, cursor,
                             CurrentTime);

    while (1) {
        XNextEvent(dpy, &event);
        switch (event.type) {
        case MotionNotify:
            if (boxDrawn) {
                XDrawRectangle(dpy, root, xorGC, x, y, w, h);
                boxDrawn = 0;
            }
            while (XCheckTypedEvent(dpy, MotionNotify, &event)) ;
            X11Grab_MakeBox(rootx, rooty, event.xbutton.x_root,
                            event.xbutton.y_root, &x, &y, &w, &h);
            XDrawRectangle(dpy, root, xorGC, x, y, w, h);
            boxDrawn = 1;
            break;
        case ButtonRelease:
            if (boxDrawn) {
                XDrawRectangle(dpy, root, xorGC, x, y, w, h);
                boxDrawn = 0;
            }
            XFlush(dpy);
            X11Grab_MakeBox(rootx, rooty, event.xmotion.x_root,
                            event.xmotion.y_root, &x, &y, &w, &h);
            XUngrabPointer(dpy, CurrentTime);
            XFreeGC(dpy, xorGC);
            XFreeCursor(dpy, cursor);
            (void) Tcl_Eval(interp, "set x11grabMode fixed");
            (void) X11Grab_UpdatePos(root, x, y, w, h);
            return TCL_OK;
        }
    }
}

/*ARGSUSED*/
static int X11Grab_SetWindowCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, char *argv[])
{
    int buttons=0;
    Cursor cursor;
    XEvent event;

    cursor = XCreateFontCursor(dpy, XC_crosshair);
    target = None;

    if (XGrabPointer(dpy, vRoot, False, ButtonPressMask|ButtonReleaseMask,
                     GrabModeSync, GrabModeAsync, root, cursor,
                     CurrentTime) != GrabSuccess) {
        Tcl_AppendResult(interp, argv[0], ": can't grab mouse", NULL);
        return TCL_ERROR;
    }
    
    while ((target == None) || (buttons != 0)) {
        XAllowEvents(dpy, SyncPointer, CurrentTime);
        XWindowEvent(dpy, vRoot, ButtonPressMask|ButtonReleaseMask, &event);
        switch (event.type) {
        case ButtonPress:
            if (target == None) target = event.xbutton.subwindow;
            buttons++;
            break;
        case ButtonRelease:
            if (buttons > 0) buttons--;
            break;
        }
    }
    
    XUngrabPointer(dpy, CurrentTime);
    XFreeCursor(dpy, cursor);

    (void) Tcl_Eval(interp,
               ".grabControls.x11grab.row1.mode.window config -state normal");
    (void) Tcl_Eval(interp, "set x11grabMode window");
    (void) X11Grab_FollowWindow();
    return TCL_OK;
}

int
X11Grab_Probe(Tk_Window tkMainWin)
{
    Window rw;
    interp = Tcl_CreateInterp();

    if (tkMainWin == NULL) return 0;

    Tcl_TraceVar(interp, "x11grabMode", TCL_TRACE_WRITES, X11Grab_TraceMode,
                 NULL);
    Tcl_CreateCommand(interp, "x11grabSetX", X11Grab_SetXCmd, 0, NULL);
    Tcl_CreateCommand(interp, "x11grabSetY", X11Grab_SetYCmd, 0, NULL);
    Tcl_CreateCommand(interp, "x11grabSetW", X11Grab_SetWCmd, 0, NULL);
    Tcl_CreateCommand(interp, "x11grabSetH", X11Grab_SetHCmd, 0, NULL);
    Tcl_CreateCommand(interp, "x11grabSetRegion", X11Grab_SetRegionCmd, 0,
                      NULL);
    Tcl_CreateCommand(interp, "x11grabSetWindow", X11Grab_SetWindowCmd, 0,
                      NULL);
    dpy = Tk_Display(tkMainWin);
    rootwin = rw = RootWindow(dpy, Tk_ScreenNumber(tkMainWin));
    VidUtil_Init(dpy);
    return X11Grab_Initialize(rw, width, height);
}
#endif /* UNUSED */

} /* end extern "C" block */


int
X11Device::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();

	if ((argc == 3) && (strcmp(argv[1], "open") == 0)) {
		TclObject* o = new X11Grabber(name_, argv[2]);
		if (o != 0)
			tcl.result(o->name());
		return (TCL_OK);
	}
	return (InputDevice::command(argc, argv));
}

X11Grabber::X11Grabber(const char* name, const char* format)
{
	//SV-XXX: unused
	UNUSED(name);

	c_grab = (int)NULL ; /* XXX */
	theroot_ = None ; /* XXX */
	ximage_ = NULL ;
	color = NULL ;
	col2y_ = NULL ;
	col2rgb16_ = NULL ;

	width_ = 320 ;
	height_ = 240 ;
	x_origin_ = y_origin_ = 0 ; /* XXX */

	if (strcmp(format, "420") && strcmp(format, "cif")) {
		fprintf(stderr,
			"vic: x11Grabber: unsupported format: %s\n",
			format);
		abort();
	}

	Tk_Window tkMainWin = Tcl::instance().tkmain() ;
	Window rw ;

	dpy_ = Tk_Display(tkMainWin);
	rootwin_ = rw = RootWindow(dpy_, Tk_ScreenNumber(tkMainWin));

	/* Initialize the RGB565 to YUV tables */
	int i, r, g, b, y, u, v;

	rgb2y_ = new uint8[65536] ;
	rgb2u_ = new uint8[65536] ;
	rgb2v_ = new uint8[65536] ;

	i = 0;
	for (r=4; r<256; r+=8) {
	    for (g=3; g<256; g+=4) { /* XXX */
		for (b=4; b<256; b+=8) {
		    y = (38*r+75*g+15*b+64)/128;
		    u = 74*(b-y)/128;
		    if (u > 127) u = 127 ;
		    else if (u< -128) u = -128 ;
		    v = (93*(r-y)/128);
		    if (v > 127) v = 127 ;
		    else if (v< -128) v = -128 ;
		    rgb2y_[i] = y ;
		    rgb2u_[i] = u ^ 0x80 ; /* was u */
		    rgb2v_[i] = v ^ 0x80 ;
		    i++;
		}
	    }
	}

	X11Grab_Initialize(rw, width_, height_);

	mode_ = X11GRAB_FIXED; /* XXX */
	decimate_ = 1; /* XXX */
	basewidth_ = PAL_WIDTH * 2;
	baseheight_ = PAL_HEIGHT * 2;
	
}

X11Grabber::~X11Grabber()
{
        if (ximage_ != NULL)
	    VidUtil_DestroyXImage(dpy_, ximage_);
	free(rgb2y_);
	free(rgb2u_);
	free(rgb2v_);
}

void
X11Grabber::setsize()
{
	int rows, columns;

	rows = (baseheight_ / decimate_) &~0xf;	/* 0xf, ugh! */
	columns = (basewidth_ / decimate_)  &~0xf;

	/* XXX set size of captured window ? */

	set_size_420(columns, rows); /* was 422... */
	X11Grab_Initialize(rootwin_, columns, rows); /* XXX */

	allocref();	/* allocate reference frame */
}

void
X11Grabber::format()
{

	baseheight_ = CIF_HEIGHT * 2;
	basewidth_ = CIF_WIDTH * 2;
		
	setsize();
}


void
X11Grabber::start()
{
	format();
	/* XXX prepare for continuous capture */
	Grabber::start();
}

void
X11Grabber::stop()
{
	/* XXX stop capture */
	VidUtil_DestroyXImage(dpy_, ximage_);
	ximage_ = NULL ;
	Grabber::stop();
}

int
X11Grabber::command(int argc, const char*const* argv)
{
    if (argc >= 3) {
	if (strcmp(argv[1], "decimate") == 0) {
	    int dec = atoi(argv[2]);
	    Tcl& tcl = Tcl::instance();
	    if (dec <= 0) {
		tcl.resultf("%s: divide by zero", argv[0]);
		    return (TCL_ERROR);
	    }
	    if (dec != decimate_) {
		decimate_ = dec;
		if(running_) {
		    stop();
		    setsize();
		    start();
		}
	    }
	    return (TCL_OK);	
	} else if (strcmp(argv[1], "fixed") == 0) {
	    mode_ = X11GRAB_FIXED;

	    int x = atoi(argv[2]);
	    int y = atoi(argv[3]);
	    if (x >= 0 && *argv[2] != '-' && x + width_ <= root_width)
		x_origin_ = x ;
	    else if ( x <= 0 && -x + width_ <= root_width )
		x_origin_ = root_width + x - width_ ;
	    if (y >= 0 && *argv[3] != '-' && y + height_ <= root_height)
		y_origin_ = y ;
	    else if (y <= 0 && -y + height_ <= root_height )
		y_origin_ = root_height + y - height_ ;
	    fprintf(stderr, "x11 fixed %d %d (root %dx%d)\n",
		x_origin_, y_origin_, root_width, root_height);
	    return (TCL_OK);
	} else if (!strcmp(argv[2], "pointer")) {
	    mode_ = X11GRAB_POINTER;
	    return (TCL_OK);
	} else if (!strcmp(argv[2], "window")) {
	    mode_ = X11GRAB_WINDOW;
	    return (TCL_OK);
	} else if (strcmp(argv[1], "format") == 0 ||
		   strcmp(argv[1], "type") == 0) {
	    if (running_)
		format();
	    return (TCL_OK);	
	} else if (strcmp(argv[1], "contrast") == 0) {
	    contrast(atof(argv[2]));
	    return (TCL_OK);	
	}
    } else if (argc == 2) {
	if (strcmp(argv[1], "format") == 0 ||
	       strcmp(argv[1], "type") == 0) {
	    return (TCL_OK);
	}
    }
    return (Grabber::command(argc, argv));
}



/*
 * captures in CIF or 420 -- color info is half the luma info.
 */
int
X11Grabber::capture()
{
    int dograb = 0 ;

#define MY_T	uint8


    switch (mode_) {
    case X11GRAB_FIXED:
        dograb = 1;
        break;
#if 0 /* not yet... */
    case X11GRAB_POINTER:
        dograb = X11Grab_FollowPointer();
        break;
    case X11GRAB_WINDOW:
        dograb = X11Grab_FollowWindow();
        break;
#endif
    }
    
    if (1 || dograb) {
	XImage *image=ximage_->image;

#ifdef USE_SHM
	if (use_shm && ximage_->shminfo != NULL) 
	    XShmGetImage(dpy_, theroot_, image, x_origin_, y_origin_,AllPlanes);
	else
#endif
	    XGetSubImage(dpy_, theroot_, x_origin_, y_origin_,
		image->width, image->height, AllPlanes,
		     ZPixmap, image, 0, 0);
/* Davide Cavagnino: old version; gcc 2.8.1 hangs up
	     (X11Grabber::c_grab)();
   below new version that works with 2.8.1
*/
        if (this->c_grab)
		(this->*c_grab)();
	return 1 ;
    } else
        return 0;
}

int X11Grabber::grab()
{
    if (capture() == 0)
	return (0);
    suppress(frame_);
    saveblks(frame_);
    YuvFrame f(media_ts(), frame_, crvec_, outw_, outh_);
    return (target_->consume(&f));
}

