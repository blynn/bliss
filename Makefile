SOURCEFILES := *.[ch] Makefile
OTHERFILES := LICENSE README NEWS font.ttf bminfo.txt
ALLFILES := $(SOURCEFILES) $(OTHERFILES)
DYNOS := version.o util.o darray.o cell.o
BLISSPLUGINS := sine.so decay.so
MISCPLUGINS := nop.so atracker.so
BUZZPLUGINS := bbass2.so bdelay.so bdistortion.so bnoise.so
PLUGINS := $(BLISSPLUGINS) $(BUZZPLUGINS) $(MISCPLUGINS)
PROJNAME := bliss
VERSION := snapshot

ifdef WIN32
CC := i586-mingw32msvc-gcc
EFLAGS:=-boost -O2 -I /home/ben/cross/SDL/include/SDL
CFLAGS:=-O2 -Wall -I /home/ben/cross/SDL/include/SDL -mwindows
SDL_LIBS:=-L /home/ben/cross/SDL/lib -lmingw32 -lSDLmain -lSDL
LIBS := $(SDL_LIBS) -lSDL_ttf
PLAT:=win32
else
CC := gcc
CFLAGS:=-Wall -O2 -fomit-frame-pointer `sdl-config --cflags`
SDL_LIBS:=`sdl-config --libs`
LIBS := $(SDL_LIBS) -lSDL_ttf 
PLAT:=linux
endif

OBJS:= audio.o main.o \
    parse.o \
    master.o \
    mlist.o \
    base64.o \
    unit.o machine.o pattern.o track.o song.o wave.o \
    bmachine.o \
    sidebar.o machine_area.o \
    unit_sidebar.o unit_area.o unit_window.o \
    cell_area.o \
    root.o machine_window.o pattern_area.o song_area.o \
    about.o filewin.o tbwin.o \
    convert_buzz.o \
    colour.o font.o \
    widget.o label.o combobox.o textbox.o button.o \
    container.o grid.o spreadsheet.o menu.o window.o \
    SDL_gfxPrimitives.o pl.o

TARGET: $(PROJNAME) $(PLUGINS) dumpbuzz

dumpbuzz : dumpbuzz.c
	$(CC) $(CFLAGS) -o $@ $<

pl.c : pl.$(PLAT).c
	cp $^ $@

$(OBJS): %.o: %.c 
	$(CC) $(CFLAGS) -c $^

buzz_machine.o: buzz_machine.c
	$(CC) $(CFLAGS) -fPIC -c $^

machine_plugin.o: machine_plugin.c
	$(CC) $(CFLAGS) -fPIC -c $^

$(DYNOS): %.o: %.c 
	$(CC) $(CFLAGS) -fPIC -c $^

$(BUZZPLUGINS): %.so: %.c $(DYNOS) buzz_machine.o machine_plugin.o
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^

$(BLISSPLUGINS): %.so: %.c $(DYNOS) unit_plugin.o
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^

$(MISCPLUGINS): %.so: %.c $(DYNOS) machine_plugin.o
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^

$(PROJNAME) : $(OBJS) $(DYNOS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

DISTNAME:=$(PROJNAME)-$(VERSION)

dist: $(ALLFILES)
	cp helmetr.ttf font.ttf
	-rm pl.c
	-rm -rf $(DISTNAME)
	mkdir $(DISTNAME)
	cp -rl --parents $(ALLFILES) $(DISTNAME)
	tar chfz $(DISTNAME).tgz $(DISTNAME)
	-rm -rf $(DISTNAME)

ifdef WIN32
zip : $(PROJNAME) $(PLUGINS) $(OTHERFILES)
	cp helmetr.ttf font.ttf
	-rm -rf $(DISTNAME)
	mkdir $(DISTNAME)
	cp -l $(PROJNAME) $(DISTNAME)/$(PROJNAME).exe
	cp -l $(PLUGINS) $(DISTNAME)
	cp -l $(OTHERFILES) $(DISTNAME)
	cp -l *.ttf $(DISTNAME)
	cp -l /home/ben/cross/SDL/lib/SDL.dll $(DISTNAME)
	cp -l /home/ben/cross/SDL/lib/SDL_ttf.dll $(DISTNAME)
	zip $(DISTNAME)-win.zip $(DISTNAME)/*
	-rm -rf $(DISTNAME)
endif

clean :
	-rm $(PROJNAME) $(PLUGINS) *.o pl.c
