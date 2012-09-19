PROC=athlon
CFLAGS=-g -Wall -O2 -march=$(PROC) -mcpu=$(PROC)
MOTIFLIB=/usr/local/lesstif/lib
MOTIFINC=/usr/local/lesstif/include
MNGLIB=-lmng
LIBS=-L/usr/X11R6/lib -L$(MOTIFLIB) -lXm -lXt -lX11 $(MNGLIB) -lm
INC=-I/usr/X11R6/include -I$(MOTIFINC)
CC=gcc
LDFLAGS=

all: clean compile

compile:
	$(CC) $(CFLAGS) $(INC) color.c xmngview.c -o xmngview $(LIBS)

clean:
	rm -f xmngview core 

install:
	cp -a xmngview  /usr/local/bin/xmngview
