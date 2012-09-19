LIBMNG_PREFIX = /usr/local

CC = gcc

CFLAGS = -g -O2 -Wall \
	`pkg-config --cflags gtk+-2.0` \
	-I$(LIBMNG_PREFIX)/include

LIBS = `pkg-config --libs gtk+-2.0` \
	-L$(LIBMNG_PREFIX)/lib -lmng -ljpeg -llcms -lz

OBJ = dummy.o \
      gtk-mng-view.o

EXE_BASENAME=gmngview

all: $(EXE_BASENAME)

$(EXE_BASENAME): $(OBJ)
	$(CC) -o $(EXE_BASENAME) $(OBJ) $(LIBS)

#$(EXE_BASENAME)-static: $(OBJ)
#	$(CC) -static -o $(EXE_BASENAME)-static $(OBJ) $(LIBS)

.c.o: gtk-mng-view.h
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f core $(EXE_BASENAME) $(EXE_BASENAME)-static $(OBJ)
