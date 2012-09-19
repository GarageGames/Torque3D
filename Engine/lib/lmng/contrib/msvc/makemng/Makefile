########################################################################
# This is a GNU makefile - tested on CYGWIN and Linux
#
# You may need to compile or install libmng before compiling this.
# Libmng also requires zlib, jpeg and lcms
#

TARGET = makemng
# These flags are needed to get it to compile with libmng.dll on CYGWIN
# CYGWINFLAGS = -DMNG_USE_DLL
CYGWINFLAGS =
MNGFLAGS = -I. -DMNG_SKIP_LCMS -DMNG_SKIP_IJG6B \
           -DMNG_SUPPORT_READ -DMNG_SUPPORT_WRITE -DMNG_SUPPORT_DISPLAY \
           -DMNG_ACCESS_CHUNKS

CC = gcc
CFLAGS = -W -Wall -O0 # -g
LIBS = -L/usr/local/lib -lmng -lm -lz -ljpeg -llcms
LDFLAGS=

BINS = $(TARGET) $(TARGET).exe
SRCS = makemng.c filelist.c
OBJS = makemng.o filelist.o

.SUFFIXES: .c .o

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $*.c $(MNGFLAGS) $(CYGWINFLAGS)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) $(LDFLAGS)

clean:
	rm -f $(BINS) *.o

