CC = gcc
CFLAGS=-O2 -Wall `sdl-config --cflags`
SDL_LIBS=`sdl-config --libs`
LIBS = $(SDL_LIBS) -lSDL_ttf 
NOSE_SDL_LIBS=\`sdl-config --libs\` -lSDL_ttf

ALLFILES=buzz/*.e *.e *.c *.h Makefile HISTORY version.h loadpath.se README LICENSE

EXTCLIBS=speed.o external.o SDL_gfxPrimitives.o SDL_rotozoom.o

.PHONY : target clean

target : bliss

speed.o : speed.c
external.o : external.c
SDL_gfxPrimitives.o : SDL_gfxPrimitives.c
SDL_rotozoom.o : SDL_rotozoom.c

bliss.make : *.e buzz/*.e
	compile_to_c -boost -O2 -o bliss bliss $(EXTCLIBS) $(LIBS)

bliss : bliss.make $(EXTCLIBS)
	. bliss.make

noboost : *.e $(EXTCLIBS)
	compile_to_c -O2 -o slow bliss $(EXTCLIBS) $(LIBS)
	. bliss.make

nose: *.e
	compile_to_c -boost -O2 -o bliss bliss.e $(EXTCLIBS) \
$(NOSE_SDL_LIBS)

projname := $(shell awk '/BLISS_VERSION/ { print $$3 }' version.h )

dist: $(ALLFILES) clean nose
	-rm -rf $(projname)
	mkdir $(projname)
	cp -rl --parents $(ALLFILES) $(projname)
	tar chfz $(projname).tgz $(projname)
	-rm -rf $(projname)

clean :
	clean bliss
	-rm *.o
