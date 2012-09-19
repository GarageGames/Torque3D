#
# For conditions of distribution and use, see copyright notice in libmng.h
#
# makefile for libmng - THE MNG library
# this makefile is suitable for generic unix

# Configuration options are now in libmng_conf.h

# The name of your C compiler:
CC= cc

# Location of jpeg header files
JPEG_INC= /cs/include/jpeg

# Location of zlib header files
ZLIB_INC= /cs/include

# Location of lcms header files
# (switch on MNG_FULL_CMS in libmng_conf.h if you want to use this)
LCMS_INC= /ltmp/lcms-1.06/source

# compiler options:
CFLAGS= -O -I. -I$(ZLIB_INC) -I$(JPEG_INC) -I$(LCMS_INC)

# source files
SOURCES= \
	 libmng_callback_xs.c \
	 libmng_chunk_io.c \
	 libmng_chunk_descr.c \
	 libmng_chunk_prc.c \
	 libmng_chunk_xs.c \
	 libmng_cms.c \
	 libmng_display.c \
	 libmng_dither.c \
	 libmng_error.c \
	 libmng_filter.c \
	 libmng_hlapi.c \
	 libmng_jpeg.c \
	 libmng_object_prc.c \
	 libmng_pixels.c \
	 libmng_prop_xs.c \
	 libmng_read.c \
	 libmng_trace.c \
	 libmng_write.c \
	 libmng_zlib.c

# object files
OBJECTS= $(SOURCES:%.c=%.o)

# type dependancies
.c.o:
	$(CC) $(CFLAGS) -c $<

all: libmng.a

clean:
	/bin/rm -f $(OBJECTS)
	/bin/rm -f libmng.a
	/bin/rm -f *~ core

libmng.a: $(OBJECTS)
	ar r libmng.a $(OBJECTS)

depend:
	makedepend -- $(CFLAGS) $(IFLAGS) -- *.c

# DO NOT DELETE
