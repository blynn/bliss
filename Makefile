ALLFILES = *.[ch] Makefile LICENSE README NEWS helmetr.ttf
DYNOS = version.o util.o darray.o
PLUGINS = sine.so master.so nop.so bbass2.so atracker.so
PROJNAME = bliss
VERSION = 0.01

ifdef WIN32
CC = i586-mingw32msvc-gcc
EFLAGS=-boost -O2 -I /home/ben/cross/SDL/include/SDL
CFLAGS=-O2 -Wall -I /home/ben/cross/SDL/include/SDL -mwindows
SDL_LIBS=-L /home/ben/cross/SDL/lib -lmingw32 -lSDLmain -lSDL
LIBS = $(SDL_LIBS) -lSDL_ttf
PLAT=win32
else
CC = gcc
CFLAGS=-Wall -O2 -fomit-frame-pointer `sdl-config --cflags`
SDL_LIBS=`sdl-config --libs`
LIBS = $(SDL_LIBS) -lSDL_ttf 
PLAT=linux
endif

OBJS= audio.o main.o \
    mlist.o \
    base64.o \
    machine.o pattern.o track.o song.o wave.o \
    root.o machine_area.o pattern_area.o song_area.o \
    about.o filewin.o tbwin.o \
    convert_buzz.o \
    colour.o font.o \
    widget.o label.o listbox.o textbox.o button.o \
    container.o grid.o spreadsheet.o menu.o window.o \
    SDL_gfxPrimitives.o pl.o

TARGET: $(PROJNAME) $(PLUGINS)

pl.c : pl.$(PLAT).c
	cp $^ $@

$(OBJS): %.o: %.c 
	$(CC) $(CFLAGS) -c $^

$(DYNOS): %.o: %.c 
	$(CC) $(CFLAGS) -fPIC -c $^

$(PLUGINS): %.so: %.c $(DYNOS)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^

$(PROJNAME) : $(OBJS) $(DYNOS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

DISTNAME=$(PROJNAME)-$(VERSION)

dist: $(ALLFILES)
	-rm -rf $(DISTNAME)
	mkdir $(DISTNAME)
	cp -rl --parents $(ALLFILES) $(DISTNAME)
	tar chfz $(DISTNAME).tgz $(DISTNAME)
	-rm -rf $(DISTNAME)

ifdef WIN32
zip : $(PROJNAME) $(PLUGINS)
	-rm -rf $(DISTNAME)
	mkdir $(DISTNAME)
	cp -l $(PROJNAME) $(DISTNAME)/$(PROJNAME).exe
	cp -l $(PLUGINS) $(DISTNAME)
	cp -l LICENSE $(DISTNAME)
	cp -l *.ttf $(DISTNAME)
	cp -l /home/ben/cross/SDL/lib/SDL.dll $(DISTNAME)
	cp -l /home/ben/cross/SDL/lib/SDL_ttf.dll $(DISTNAME)
	zip $(DISTNAME)-win.zip $(DISTNAME)/*
	-rm -rf $(DISTNAME)
endif

clean :
	-rm $(PROJNAME) $(PLUGINS) *.o pl.c
