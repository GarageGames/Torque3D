/*
	fbmngplay - fb console MNG player.
	(c) 2001 by Stefan Reinauer, <stepan :at: suse.de>

	This program is based on mngplay, part of libmng, written and (C) by
	Ralph Giles <giles :at: ashlu.bc.ca>

	This program my be redistributed under the terms of the
	GNU General Public Licence, version 2, or at your preference,
	any later version.
*/

#include <stdio.h>

#include "fbmngplay.h"
#include "messages.h"

void usage(char *name) 
{
	fprintf(stderr, "\nusage: %s [ -x <val> ] [ -y <val> ] [ -a <val> ] [-b] [-v]"
			" [-s] [file.mng [file.mng [...]]]\n", name);
	fprintf(stderr, "\n	-x: x coordinate\n");
	fprintf(stderr, "	-y: y coordinate\n");
	fprintf(stderr, "	-a, --alpha: default alpha channel 1..100\n");
	fprintf(stderr, "	-v, --verbose: verbose mode\n");
	fprintf(stderr, "	-b, --buffered: buffered mode\n");
	fprintf(stderr, "	-s, --signal: wait for SIGUSR1 between animations\n");
	fprintf(stderr, "	-p, --position: dynamically select position\n");
	fprintf(stderr, "	-V, --version: show version and exit\n");
	fprintf(stderr, "	-?, -h, --help: print this help.\n\n");
	fprintf(stderr, "	-S --start-console: only output animation on console it was started on.\n");
}

void version(void)
{
	fprintf(stderr, "fbmngplay v%s, Copyright (C) 2001 Stefan Reinauer\n",FBMNGPLAY_VERSION);
	fprintf(stderr, "fbmngplay comes with ABSOLUTELY NO WARRANTY;\n");
	fprintf(stderr,"This is free software, and you are welcome to redistribute it\n");
	fprintf(stderr,"under certain conditions; Check the GPL for details.\n");
}
