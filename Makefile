VERSION := 0.1.2
DEMOFILES := demo.bl shepard.bl stomper.bl pluck.bl dist.bl
ALLFILES := *.bmp *.[ch] Makefile linux/*.[ch] win32/*.[ch] \
	LICENSE README NEWS $(DEMOFILES)
PROJNAME := bliss
DISTNAME := $(PROJNAME)-$(VERSION)
OS ?= linux

ifeq ("$(OS)", "win32")
CC := i586-mingw32msvc-gcc
CFLAGS=-O2 -pipe -Wall -I /home/ben/cross/SDL/include/SDL -mwindows
SDL_LIBS=-L /home/ben/cross/SDL/lib -lmingw32 -lSDLmain -lSDL
else
CC := gcc
CFLAGS := -O2 -pipe -Wall -fomit-frame-pointer `sdl-config --cflags`
SDL_LIBS:=`sdl-config --libs`
endif
LIBS := $(SDL_LIBS)

.PHONY: target dist clean

UNITS := out.o dummy.o \
	funk.o \
	osc.o noise.o shepard.o random_wave.o stomperosc.o \
	adsr.o stomperenv.o seg.o \
	butterlpf.o butterhpf.o \
	lp4pole.o \
	onezero.o onepole.o twopole.o \
	delay.o clipper.o
ADTOBJS := darray.o htable.o graph.o
AUDIOOBJS := audio.o midi.o ins.o voice.o note.o gen.o
GFXOBJS := SDL_gfxPrimitives.o colour.o \
	widget.o menu.o checkbox.o button.o label.o textbox.o window.o
BLISSOBJS := $(UNITS) $(ADTOBJS) $(AUDIOOBJS) $(GFXOBJS) \
	layout.o about.o file_window.o compan.o canvas.o config.o

ifeq ("$(OS)", "win32")
BINARIES := bliss test
else
BINARIES := bliss test diffy
endif

target : version.h $(BINARIES)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $^

midi.c : $(OS)/midi.c
	-rm $@
	ln -s $^ $@

version.h : Makefile
	echo '#define VERSION_STRING "'$(VERSION)'"' > version.h

bliss : bliss.c $(BLISSOBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

sine :sine.c audio.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

test :test.c $(AUDIOOBJS) $(ADTOBJS) $(UNITS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

diffy : diffy.c $(GFXOBJS) $(ADTOBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

dist: $(ALLFILES) clean
	-rm -rf $(DISTNAME)
	mkdir $(DISTNAME)
	cp -rl --parents $(ALLFILES) $(DISTNAME)
	tar chfz $(DISTNAME).tgz $(DISTNAME)
	-rm -rf $(DISTNAME)

ifeq ("$(OS)", "win32")
zip : target
	-rm -rf $(DISTNAME)
	mkdir $(DISTNAME)
	cp -l $(DEMOFILES) $(DISTNAME)
	cp -l *.bmp $(DISTNAME)
	cp -l bliss $(DISTNAME)/bliss.exe
	cp -l /home/ben/cross/SDL/bin/SDL.dll $(DISTNAME)
	echo "latency 2048" > $(DISTNAME)/config.txt
	zip $(DISTNAME)-win.zip $(DISTNAME)/*
	-rm -rf $(DISTNAME)
endif

clean :
	-rm version.h midi.c $(BINARIES) *.o
