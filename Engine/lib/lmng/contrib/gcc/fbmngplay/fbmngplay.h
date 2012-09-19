/*
	fbmngplay - fb console MNG player.
	(c) 2001 by Stefan Reinauer, <stepan :at: suse.de>

	This program is based on mngplay, part of libmng, written and (C) by
	Ralph Giles <giles :at: ashlu.bc.ca>

	This program my be redistributed under the terms of the
	GNU General Public Licence, version 2, or at your preference,
	any later version.
*/

#ifndef __FBMNGPLAY_H
#define __FBMNGPLAY_H

#include <libmng.h>

#define FBMNGPLAY_VERSION "0.3"

/* structure for keeping track of our mng stream inside the callbacks */
typedef struct {
        FILE *file;             /* pointer to the file we're decoding */
        char *filename;         /* pointer to the file's path/name */
        mng_uint32 delay;       /* ticks to wait before resuming decode */
        unsigned char *display; /* pointer to display */
        unsigned char *copybuffer;
        unsigned char *background;
        mng_handle mng;         /* mng handle */
        int width, height;
        int fbwidth, fbheight, fbbpp;
        int fbx, fby;
        int alpha;
} mngstuff;

extern volatile int run;
extern int verbose;
extern int buffered;
extern int dynpos;
extern int waitsignal;
extern int delta;
extern int sconly;

#endif
