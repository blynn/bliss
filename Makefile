VERSION := 0.1.4
DEMOFILES := demo.bl shepard.bl stomper.bl pluck.bl dist.bl
ALLFILES := *.bmp *.[ch] linux/*.[ch] win32/*.[ch] \
	LICENSE README NEWS $(DEMOFILES) Makefile SConstruct
PROJNAME := bliss
DISTNAME := $(PROJNAME)-$(VERSION)
OS ?= linux

ifeq ("$(OS)", "win32")
CC := i586-mingw32msvc-gcc
CFLAGS=-O2 -pipe -Wall -I /home/ben/cross/SDL/include/SDL -mwindows
SDL_LIBS=-L /home/ben/cross/SDL/lib -lmingw32 -lSDLmain -lSDL
else
CC := gcc-3.4
CFLAGS := -O2 -pipe -Wall -ffast-math -fomit-frame-pointer `sdl-config --cflags`
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
AUDIOOBJS := audio.o midi.o orch.o ins.o voice.o note.o gen.o track.o
GFXOBJS := SDL_gfxPrimitives.o colour.o \
	widget.o menu.o checkbox.o button.o label.o textbox.o window.o
BLISSOBJS := $(UNITS) $(ADTOBJS) $(AUDIOOBJS) $(GFXOBJS) \
	about.o file_window.o compan.o aux.o canvas.o config.o utable.o \
	file.o gui.o

ifeq ("$(OS)", "win32")
BINARIES := bliss test
else
BINARIES := bliss diffy test
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
