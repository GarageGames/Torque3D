# Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

SHELL = /bin/sh
CC = gcc

SOURCES = fbmngplay.c
PROGRAMS =  fbmngplay fbmngplay.static
OBJECTS = fbmngplay.o messages.o mng.o console.o

LDFLAGS =  -L/usr/lib 
LIBSS = /usr/lib/libmng-mini.a /usr/lib/libz.a -lm
LIBSD = -lmng -lz -lm

CFLAGS = -Os -Wall -Wmissing-prototypes -Wstrict-prototypes -D_REENTRANT
COMPILE = $(CC) $(CFLAGS)
LINKS = $(CC) $(CFLAGS) $(LDFLAGS) -static -o $@
LINKD = $(CC) $(CFLAGS) $(LDFLAGS) -o $@

all: $(PROGRAMS)
	strip -s $(PROGRAMS)
.SUFFIXES:
.SUFFIXES: .S .c .o .s

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean:
	-rm -f *.o core
clean:	mostlyclean
	-rm -f fbmngplay fbmngplay.static

fbmngplay: $(OBJECTS) 
	@rm -f fbmngplay
	$(LINKD) $(LDFLAGS) $(OBJECTS) $(LIBSD)
fbmngplay.static: $(OBJECTS)
	@rm -f fbmngplay.static
	$(LINKS) $(LDFLAGS) $(OBJECTS) $(LIBSS)

.PHONY: mostlyclean clean fbmngplay fbmngplay.static all
