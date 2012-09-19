/*
	fbmngplay - fb console MNG player.
	(c) 2001 by Stefan Reinauer, <stepan :at: suse.de>

	This program is based on mngplay, part of libmng, written and (C) by
	Ralph Giles <giles :at: ashlu.bc.ca>

	This program my be redistributed under the terms of the
	GNU General Public Licence, version 2, or at your preference,
	any later version.
*/

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include "fbmngplay.h"
#include "messages.h"
#include "console.h"
#include "mng.h"

volatile int run = 1;
int verbose = 0;
int buffered = 0;
int dynpos = 0;
int waitsignal = 0;
int delta = 16;
int sconly=0;

void sigint_handler(int signal);
void sigterm_handler(int signal);
void sigusr1_handler(int signal);

void sigint_handler(int signal)
{
	run = 2;
}

void sigterm_handler(int signal)
{
	restore_area();
	run = 0;
}

void sigusr1_handler(int signal)
{
	run = 0;
}

int main(int argc, char *argv[])
{
	int fbdev,c,option_index;
	unsigned int alpha;
	struct fb_var_screeninfo var;

	/* Check which console we're running on */
	init_consoles();
		
	/* allocate our stream data structure */
	mng = (mngstuff *) calloc(1, sizeof(*mng));
	if (mng == NULL) {
		fprintf(stderr, "could not allocate stream structure.\n");
		exit(0);
	}
	alpha = 100;
	mng->alpha = 100;
	mng->fbx = 15;
	mng->fby = 15;
	mng->background = NULL;

	while (1) {
		static struct option long_options[] = {
			{"help", 0, 0, 'h'},
			{"verbose", 0, 0, 'v'},
			{"alpha", 1, 0, 'a'},
			{"buffered", 0, 0, 'b'},
			{"signal", 0, 0, 's'},
			{"delta", 0, 0, 'd'},
			{"position", 0, 0, 'p'},
			{"version", 0, 0, 'V'},
			{"start-console",0,0,'S'},
			{"console",1,0,'c'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "a:x:y:bh?vsd:pVSc:",
				long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 'a':
			alpha = atoi(optarg);
			if (alpha > 100)
				alpha = 100;
			mng->alpha = alpha;
			break;
		case 'x':
			mng->fbx = atoi(optarg);
			break;
		case 'y':
			mng->fby = atoi(optarg);
			break;
		case 'd':
			delta = atoi(optarg);
			break;
		case '?':
		case 'h':
			usage(argv[0]);
			exit(0);
		case 'v':
			verbose = 1;
			break;
		case 's':
			waitsignal = 1;
			break;
		case 'b':
			buffered = 1;
			break;
		case 'p':
			dynpos = 1;
			break;
		case 'V':
			version();
			exit(0);
		case 'c':
			start_console=atoi(optarg);
		case 'S':
			sconly=1;
			break;
		default:
			break;
		}
	}

	if (optind >= argc) {
		printf("Which files do you want to play?\n");
		exit(0);
	}

	//init_consoles();
	
	/* Initialize framebuffer */
	fbdev = open("/dev/fb0", O_RDWR);
	if (fbdev < 0) {
		fprintf(stderr, "error while opening framebuffer.\n");
		exit(fbdev);
	}

	ioctl(fbdev, FBIOGET_VSCREENINFO, &var);
	mng->fbwidth = var.xres;
	mng->fbheight = var.yres;
	mng->fbbpp = var.bits_per_pixel;

	mng->display =
	    mmap(NULL, var.xres * var.yres * (var.bits_per_pixel >> 3),
		 PROT_WRITE | PROT_READ, MAP_SHARED, fbdev, 0);

	/* arrange to call the shutdown routine before we exit */
	atexit(&cleanup);

	while (optind < argc) {
		// leftover arguements are filenames.
		mng->filename = argv[optind++];

		/* set up the mng decoder for our stream */
		mng->mng = mng_initialize(mng, mngalloc, mngfree, MNG_NULL);
		if (mng->mng == MNG_NULL) {
			fprintf(stderr, "could not initialize libmng.\n");
			exit(1);
		}

		/* set the callbacks */
		mng_setcb_errorproc(mng->mng, mngerror);
		mng_setcb_openstream(mng->mng, mngopenstream);
		mng_setcb_closestream(mng->mng, mngclosestream);
		mng_setcb_readdata(mng->mng, mngreadstream);
		mng_setcb_gettickcount(mng->mng, mnggetticks);
		mng_setcb_settimer(mng->mng, mngsettimer);
		mng_setcb_processheader(mng->mng, mngprocessheader);
		mng_setcb_getcanvasline(mng->mng, mnggetcanvasline);
		mng_setcb_refresh(mng->mng, mngrefresh);
		/* FIXME: should check for errors here */

		signal(SIGINT, sigint_handler);
		signal(SIGTERM, sigterm_handler);

		mng_readdisplay(mng->mng);

		/* loop though the frames */
		while (mng->delay && run) {
			mdelay(mng->delay);
			mng->delay = 0;
			mng_display_resume(mng->mng);
			if (run == 2) {
				if (mng->alpha == 0)
					run = 0;
				mng->alpha -= delta;
				if (mng->alpha < 0)
					mng->alpha = 0;
			}
		}

		if (waitsignal && optind < argc) {
			signal(SIGUSR1, sigusr1_handler);
			run = 1;
			while (run) {
				sleep(2);
			}
		}

		memset(mng->copybuffer, 0,
		       4 * mng->width * mng->height);
		run = 1;
		mng->alpha = alpha;
		if (optind == argc) {	/* last file */
			restore_area();
		}
	}

	/* cleanup and quit */
	return mngquit(mng->mng);
}
