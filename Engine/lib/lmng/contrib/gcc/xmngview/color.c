/*
 * This code is mainly code I have found in
 *   ida-0.14  Gerd Knorr <kraxel@bytesex.org>
 *          http://bytesex.org/ida
 * Ida is a small and fast image viewer, motif-based.
 *
 * Copyright (C) 2002 Gerd Knorr <kraxel@bytesex.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <libmng.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/XShm.h>

#include "xmng.h"

static void (*dither_line)(unsigned char *src, unsigned char *dest, 
	unsigned int y, unsigned int width);
static void dither_line_gray(unsigned char *src, unsigned char *dest, 
	unsigned int y, unsigned int width);
static void dither_line_color(unsigned char *src, unsigned char *dest, 
	unsigned int y, unsigned int width);
static void init_dither(int shades_r, int shades_g, int shades_b, 
    int shades_gray);

static XVisualInfo     *vis_info;

/* PseudoColor: ditherresult => colormap-entry 
*/
static int             x11_colors;
static int             x11_grays;
static unsigned long   *x11_map;
static unsigned long   x11_map_color[256];
static unsigned long   x11_map_gray[64];

static unsigned long   x11_red;
static unsigned long   x11_green;
static unsigned long   x11_blue;

static int      try_red[] =    {4, 6, 6, 5, 4};
static int      try_green[] =  {8, 6, 6, 5, 4};
static int      try_blue[] =   {4, 6, 4, 5, 4};

/* TrueColor: r,g,b => X11-color 
*/
static unsigned long   x11_lut_red[256];
static unsigned long   x11_lut_green[256];
static unsigned long   x11_lut_blue[256];
static unsigned long   x11_lut_gray[256];

#define x11_black         x11_map_gray[0]
#define x11_gray          x11_map_gray[47*x11_grays/64]
#define x11_lightgray     x11_map_gray[55*x11_grays/64]
#define x11_white         x11_map_gray[63*x11_grays/64]

static int x11_alloc_grays(Display * dpy, Colormap cmap, unsigned long *colors,
	int gray)
{
    XColor          akt_color;
    int             i, upb;
	upb = gray - 1;

    for (i = 0; i < gray; i++) 
   {
/* FIXME: original code
	akt_color.red = i * 65535 / upb;
	akt_color.green = i * 65535 / upb;
	akt_color.blue = i * 65535 / upb;
*/
	akt_color.red = i * 255 / upb;
	akt_color.green = i * 255 / upb;
	akt_color.blue = i * 255 / upb;

	if (!XAllocColor(dpy, cmap, &akt_color)) 
  {
/* no free color cell available
*/
    XFreeColors(dpy, cmap, colors, i, 0);
    return 1;
  }
	colors[i] = akt_color.pixel;
   }
    return 0;
}

static int x11_alloc_colorcube(Display * dpy, Colormap cmap, 
	unsigned long *colors, int red, int green, int blue)
{
    XColor          akt_color;
    int             i, upb_r, upb_g, upb_b;

	upb_r = red - 1; upb_g = green - 1; upb_b = blue - 1;

    for (i = 0; i < red * green * blue; i++) 
   {
	akt_color.red = ((i / (green * blue)) % red) * 65535 / upb_r;
	akt_color.green = ((i / blue) % green) * 65535 / upb_g;
	akt_color.blue = (i % blue) * 65535 / upb_b;

	if (!XAllocColor(dpy, cmap, &akt_color)) 
  {
/* no free color cell available
*/
    XFreeColors(dpy, cmap, colors, i, 0);
    return 1;
  }
	colors[i] = akt_color.pixel;
   }
    return 0;
}

static unsigned long x11_alloc_color(Display * dpy, Colormap cmap, int red, 
	int green, int blue)
{
    XColor          akt_color;

    akt_color.red = red;
    akt_color.green = green;
    akt_color.blue = blue;

    XAllocColor(dpy, cmap, &akt_color);
    return akt_color.pixel;
}

static void x11_create_lut(unsigned long red_mask,
	       unsigned long green_mask,
	       unsigned long blue_mask)
{
    int             rgb_red_bits = 0;
    int             rgb_red_shift = 0;
    int             rgb_green_bits = 0;
    int             rgb_green_shift = 0;
    int             rgb_blue_bits = 0;
    int             rgb_blue_shift = 0;
    int             i;
    unsigned long   mask;

    for (i = 0; i < 24; i++) 
   {
	mask = (1 << i);
	if (red_mask & mask)
	    rgb_red_bits++;
	else 
	if (!rgb_red_bits)
	  rgb_red_shift++;
	if (green_mask & mask)
	  rgb_green_bits++;
	else 
	if (!rgb_green_bits)
	  rgb_green_shift++;

	if (blue_mask & mask)
	  rgb_blue_bits++;
	else 
	if (!rgb_blue_bits)
	  rgb_blue_shift++;
   }

    for (i = 0; i < 256; i++) 
   {
	x11_lut_red[i] = (i >> (8 - rgb_red_bits)) << rgb_red_shift;
	x11_lut_green[i] = (i >> (8 - rgb_green_bits)) << rgb_green_shift;
	x11_lut_blue[i] = (i >> (8 - rgb_blue_bits)) << rgb_blue_shift;
	x11_lut_gray[i] =
	  x11_lut_red[i] | x11_lut_green[i] | x11_lut_blue[i];
   }
}

void x11_init_color(ImageInfo *img)
{
    Colormap        cmap;
    XVisualInfo     vis_template;
    int found, vis_class;
	unsigned int i;
	Display *dpy;

	dpy = img->dpy;
    cmap = DefaultColormap(dpy, DefaultScreen(dpy));
    if (0 == x11_grays)
	  x11_grays = 8;

/* Ask for visual type 
*/
    vis_template.screen = XDefaultScreen(dpy);
    vis_template.visualid =
	  XVisualIDFromVisual(img->visual);
    vis_info = 
	  XGetVisualInfo(dpy, VisualIDMask | VisualScreenMask, &vis_template,
			  &found);
    if (XShmQueryExtension(dpy)) 
	  img->have_shmem = 1;

#if defined(__cplusplus) || defined(c_plusplus)
	vis_class = vis_info->c_class;
#else
    vis_class = vis_info->class;
#endif
	if(vis_class == TrueColor)
   {
	img->gray = 0;		/* XXX testing... */
	img->display_depth = 4;
	img->display_type = TRUECOLOR;

	x11_create_lut(vis_info->red_mask, vis_info->green_mask, 
	  vis_info->blue_mask);
	x11_black = x11_alloc_color(dpy, cmap, 0, 0, 0);
	x11_gray = x11_alloc_color(dpy, cmap, 0xc000, 0xc000, 0xc000);
	x11_lightgray = x11_alloc_color(dpy, cmap, 0xe000, 0xe000, 0xe000);
	x11_white = x11_alloc_color(dpy, cmap, 0xffff, 0xffff, 0xffff);
   } 
	else 
	if(vis_class == PseudoColor && vis_info->depth == 8) 
   {
	img->display_depth = 1;
	img->display_type = PSEUDOCOLOR;
	if (0 != x11_alloc_grays(dpy, cmap, x11_map_gray, x11_grays)) 
  {
    fprintf(stderr, "%s:%d:Sorry, can't allocate %d grays\n", 
	  __FILE__,__LINE__,x11_grays);
	Viewer_postlude();
    exit(1);
  }
	if (!img->gray) 
  {
    for (i = 0; i < sizeof(try_red) / sizeof(int); i++) 
 {
		if (0 == x11_alloc_colorcube
		    (dpy, cmap, x11_map_color,
		     try_red[i], try_green[i], try_blue[i])) 
	   {
	    x11_colors = try_red[i] * try_green[i] * try_blue[i];

	    init_dither(try_red[i], try_green[i], try_blue[i], x11_grays);

	    break;
	   }
 }
    if (i == sizeof(try_red) / sizeof(int)) 
 {
	img->gray = 1;
	fputs("failed to allocate colors, using grayscaled\n", stderr);
 }
  }
	if (img->gray)
	    init_dither(2, 2, 2, x11_grays);
   } 
	else 
	if(vis_class == StaticGray || vis_class == GrayScale) 
   {
	img->display_depth = 1;
	img->display_type = PSEUDOCOLOR;
	x11_grays = 64;
	img->gray = 1;

	init_dither(2, 2, 2, x11_grays);

	if (0 != x11_alloc_grays(dpy, cmap, x11_map_gray, x11_grays)) 
  {
    fprintf(stderr, "%s:%d: Sorry, can't allocate %d grays\n", 
	  __FILE__,__LINE__, x11_grays);
	Viewer_postlude();
    exit(1);
  }
   } 
	else 
   {
	fprintf(stderr, "%s:%d: Sorry, can't handle visual\n", __FILE__,__LINE__);
	Viewer_postlude();
	exit(1);
   }
/* some common colors 
*/
    x11_red = x11_alloc_color(dpy, cmap, 65535, 0, 0);
    x11_green = x11_alloc_color(dpy, cmap, 0, 65535, 0);
    x11_blue = x11_alloc_color(dpy, cmap, 0, 0, 65535);

    if (img->gray) 
   {
	x11_map = x11_map_gray;

	dither_line = dither_line_gray;
   } 
	else 
   {
	x11_map = x11_map_color;

	dither_line = dither_line_color;
   }
}

static int      mitshm_bang = 0;

static int x11_error_dev_null(Display * dpy, XErrorEvent * event)
{
    mitshm_bang = 1;
    return 0;
}

XImage *x11_create_ximage(ImageInfo *img)
{
    XImage         *ximage = NULL;
    unsigned char  *data;
    XShmSegmentInfo *shminfo = NULL;
    int (*old_handler)(Display * dpy, XErrorEvent * event);

    if (img->have_shmem) 
   {
	old_handler = XSetErrorHandler(x11_error_dev_null);
	img->shm = shminfo = (XShmSegmentInfo*)malloc(sizeof(XShmSegmentInfo));
	memset(shminfo, 0, sizeof(XShmSegmentInfo));
	ximage = XShmCreateImage(img->dpy,
				 img->visual,
				 img->depth,
				 ZPixmap, NULL,
				 shminfo, img->img_width, img->img_height);
	if (ximage) 
  {
    shminfo->shmid = shmget(IPC_PRIVATE,
				    ximage->bytes_per_line * ximage->height,
				    IPC_CREAT | 0777);

    if (-1 == shminfo->shmid) 
 {
	fprintf(stderr,"shmget(%dMB): %s\n",
			ximage->bytes_per_line * ximage->height / 1024 / 1024,
			strerror(errno));
	goto oom;
 }
    shminfo->shmaddr = (char *) shmat(shminfo->shmid, 0, 0);

    if ((void *) -1 == shminfo->shmaddr) 
 {
	perror("shmat");
	goto oom;
 }
    ximage->data = shminfo->shmaddr;
    shminfo->readOnly = False;

    XShmAttach(img->dpy, shminfo);
    XSync(img->dpy, False);
    shmctl(shminfo->shmid, IPC_RMID, 0);

    if (mitshm_bang) 
 {
	img->have_shmem = 0;
	shmdt(shminfo->shmaddr);
	free(shminfo);
	img->shm = shminfo = NULL;
	XDestroyImage(ximage);
	ximage = NULL;
 }
  } 
	else 
  {
    img->have_shmem = 0;
    free(shminfo);
    img->shm = shminfo = NULL;
  }
	XSetErrorHandler(old_handler);
   }

    if (ximage == NULL) 
   {
	img->shm = NULL;
	if (NULL == (data = (unsigned char*)
		malloc(img->img_width * img->img_height * img->display_depth))) 
  {
    fprintf(stderr,"Oops: out of memory\n");
    goto oom;
  }
	ximage = XCreateImage(img->dpy,
			      img->visual,
			      img->depth,
			      ZPixmap, 0, (char*)data,
			      img->img_width, img->img_height,
			      8, 0);
   }
    memset(ximage->data, 0, ximage->bytes_per_line * ximage->height);

    return ximage;

  oom:
    if (shminfo) 
   {
	if (shminfo->shmid && shminfo->shmid != -1)
	    shmctl(shminfo->shmid, IPC_RMID, 0);
	free(shminfo);
   }
    if (ximage)
	  XDestroyImage(ximage);
	img->shm = 0;
    return NULL;
}

void x11_destroy_ximage(ImageInfo *img)
{
    XShmSegmentInfo *shminfo = (XShmSegmentInfo*)img->shm;

    if (shminfo) 
   {
	XShmDetach(img->dpy, shminfo);
	XDestroyImage(img->ximage);
	shmdt(shminfo->shmaddr);
	free(shminfo);
   } 
	else
	  XDestroyImage(img->ximage);
}
/*
 * ordered dither routines
 *
 * stolen from The GIMP and trimmed for speed
 */
#define DITHER_LEVEL 8

static long     red_mult, green_mult;
static long     red_dither[256];
static long     green_dither[256];
static long     blue_dither[256];
static long     gray_dither[256];

typedef unsigned long vector[DITHER_LEVEL];
typedef vector  matrix[DITHER_LEVEL];

#if DITHER_LEVEL == 8
#define DITHER_MASK 7
static matrix   origDM =
{
    {0, 32, 8, 40, 2, 34, 10, 42},
    {48, 16, 56, 24, 50, 18, 58, 26},
    {12, 44, 4, 36, 14, 46, 6, 38},
    {60, 28, 52, 20, 62, 30, 54, 22},
    {3, 35, 11, 43, 1, 33, 9, 41},
    {51, 19, 59, 27, 49, 17, 57, 25},
    {15, 47, 7, 39, 13, 45, 5, 37},
    {63, 31, 55, 23, 61, 29, 53, 21}
};
static matrix   DM;

#endif /* DITHER_LEVEL == 8 */

#if DITHER_LEVEL == 4
#define DITHER_MASK 3
static matrix   origDM =
{
    {0, 8, 2, 10},
    {12, 4, 14, 6},
    {3, 11, 1, 9},
    {15, 7, 13, 5}
};
static matrix   DM;

#endif

static void init_dither(int shades_r, int shades_g, int shades_b, 
	int shades_gray)
{
    int             i, j;
    unsigned char   low_shade, high_shade;
    unsigned short  index;
    float           red_colors_per_shade;
    float           green_colors_per_shade;
    float           blue_colors_per_shade;
    float           gray_colors_per_shade;

    red_mult = shades_g * shades_b;
    green_mult = shades_b;

    red_colors_per_shade = 256.0 / (shades_r - 1);
    green_colors_per_shade = 256.0 / (shades_g - 1);
    blue_colors_per_shade = 256.0 / (shades_b - 1);
    gray_colors_per_shade = 256.0 / (shades_gray - 1);

/* this avoids a shift when checking these values 
*/
	memcpy(DM, origDM, sizeof(unsigned long)*DITHER_LEVEL*DITHER_LEVEL);
    for (i = 0; i < DITHER_LEVEL; i++)
	  for (j = 0; j < DITHER_LEVEL; j++)
	    DM[i][j] *= 0x10000;

/* setup arrays containing three bytes of information for red, green, & blue
 *  the arrays contain :
 *    1st byte:    low end shade value
 *    2nd byte:    high end shade value
 *    3rd & 4th bytes:    ordered dither matrix index
*/

    for (i = 0; i < 256; i++) 
  {
/*  setup the red information  
*/
    low_shade = (unsigned char) (i / red_colors_per_shade);
    high_shade = low_shade + 1;

    index = (unsigned short)
		(((i - low_shade * red_colors_per_shade) / red_colors_per_shade) *
		 (DITHER_LEVEL * DITHER_LEVEL + 1));

    low_shade *= red_mult;
    high_shade *= red_mult;

    red_dither[i] = (index << 16) + (high_shade << 8) + (low_shade);

/*  setup the green information  
*/
    low_shade = (unsigned char) (i / green_colors_per_shade);
    high_shade = low_shade + 1;

    index = (unsigned short)
		(((i - low_shade * green_colors_per_shade) / green_colors_per_shade) *
		 (DITHER_LEVEL * DITHER_LEVEL + 1));

    low_shade *= green_mult;
    high_shade *= green_mult;

    green_dither[i] = (index << 16) + (high_shade << 8) + (low_shade);

/*  setup the blue information  
*/
    low_shade = (unsigned char) (i / blue_colors_per_shade);
    high_shade = low_shade + 1;

    index = (unsigned short)
		(((i - low_shade * blue_colors_per_shade) / blue_colors_per_shade) *
		 (DITHER_LEVEL * DITHER_LEVEL + 1));

    blue_dither[i] = (index << 16) + (high_shade << 8) + (low_shade);

/*  setup the gray information  
*/
    low_shade = (unsigned char) (i / gray_colors_per_shade);
    high_shade = low_shade + 1;

    index = (unsigned short)
		(((i - low_shade * gray_colors_per_shade) / gray_colors_per_shade) *
		 (DITHER_LEVEL * DITHER_LEVEL + 1));

    gray_dither[i] = (index << 16) + (high_shade << 8) + (low_shade);
  }
}

static void dither_line_color(unsigned char *src, unsigned char *dest, 
	unsigned int y, unsigned int width)
{
    unsigned long a, b, dval, *ymod;
	unsigned int x;
    ymod = DM[y & DITHER_MASK];

	for(x = 0; x < width; x++)
   {
	dval = ymod[x & DITHER_MASK];

	b = red_dither[src[0]];
	if (dval < b)
	    b >>= 8;

	a = green_dither[src[1]];
	if (dval < a)
	    a >>= 8;
	b += a;

	a = blue_dither[src[2]];
	if (dval < a)
	    a >>= 8;
	b += a;
	src += RGB_SIZE;
	*dest++ = (unsigned char)(b & 0xff);
   }
}

static void dither_line_gray(unsigned char *src, unsigned char *dest, 
	unsigned int y, unsigned int width)
{
    unsigned long a, g, *ymod;
	unsigned int x;

    ymod = DM[y & DITHER_MASK];

	for(x = 0; x < width; x++)
   {
	g = (src[0]*3 + src[1]*6 + src[2]) / 10;
	a = gray_dither[g];
	src += RGB_SIZE;

	if (ymod[x & DITHER_MASK] < a)
	    a >>= 8;

	*dest++ = a & 0xff;
   }
}

void viewer_renderline(ImageInfo *img, unsigned char *scanline, 
	unsigned int row, unsigned int x, unsigned int width)
{
    unsigned char *src, *dst;
    unsigned long pix;
	XImage *ximage;
	unsigned int col, max_col;
	unsigned short mng_rgb_size;

	mng_rgb_size = img->mng_rgb_size;
	ximage = img->ximage;
    src = scanline;
	col = x;
	max_col = x + width;

    if (img->display_type == PSEUDOCOLOR) 
   {
    dst = img->dither_line;
    dither_line(src, dst, row, width);

	while(col < max_col)
  {
    XPutPixel(ximage, col, row, x11_map[dst[0]]);
	++col;
	++dst;
  }
	return;
   }
/* display_type == TRUECOLOR
*/ 
    while(col < max_col)
  {
    pix = x11_lut_red[src[0]] | x11_lut_green[src[1]] | x11_lut_blue[src[2]];

    XPutPixel(ximage, col, row, pix);

	++col;
	src += RGB_SIZE;
  }
}
