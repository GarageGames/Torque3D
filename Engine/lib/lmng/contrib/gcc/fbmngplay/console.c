/*
	fbmngplay - fb console MNG player.
	(c) 2001 by Stefan Reinauer, <stepan :at: suse.de>

	This program is based on mngplay, part of libmng, written and (C) by
	Ralph Giles <giles :at: ashlu.bc.ca>

	This program my be redistributed under the terms of the
	GNU General Public Licence, version 2, or at your preference,
	any later version.

	This file is based on getfd.c from the kbd package.
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/ioctl.h>

#include "console.h"

int start_console = 0;

/*
 * getfd.c
 *
 * Get an fd for use with kbd/console ioctls.
 * We try several things because opening /dev/console will fail
 * if someone else used X (which does a chown on /dev/console).
 */

static int
is_a_console(const int fd) {
    char arg;

    arg = 0;
    return (ioctl(fd, KDGKBTYPE, &arg) == 0
	    && ((arg == KB_101) || (arg == KB_84)));
}

static int
open_a_console(const char * const fnam) {
    int fd;

    fd = open(fnam, O_RDONLY);
    if (fd < 0 && errno == EACCES)
      fd = open(fnam, O_WRONLY);
    if (fd < 0)
      return -1;
    if (!is_a_console(fd)) {
      close(fd);
      return -1;
    }
    return fd;
}

int getfd (const char * const nm) {
    int fd;

    if (nm) {
	fd = open_a_console(nm);
	if (fd >= 0)
	    return fd;
    } else {
	fd = open_a_console("/dev/tty");
	if (fd >= 0)
	    return fd;

	fd = open_a_console("/dev/tty0");
	if (fd >= 0)
	    return fd;

	fd = open_a_console("/dev/console");
	if (fd >= 0)
	    return fd;

	for (fd = 0; fd < 3; fd++)
	    if (is_a_console(fd))
		return fd;
    }
    fprintf(stderr,
	    "Couldnt get a file descriptor referring to the console\n");
    exit(1);		/* total failure */
}

int fd;
int current_console(void)
{
        struct vt_stat vtstat;
        if (ioctl(fd, VT_GETSTATE, &vtstat)) {
                fprintf(stderr,"fbmngplay: VT_GETSTATE\n");
                exit(1);
        }
        return vtstat.v_active;

}

void init_consoles(void)
{
        // get current tty
	fd = getfd(0);
	start_console=current_console();
}

