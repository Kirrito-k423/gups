# Makefile for Linux

SHELL = /bin/sh

# System-specific settings

CC =		mpicc
CCFLAGS =	-O3 -DMPICH_IGNORE_CXX_SEEK -DLONG64 -DNODEF \
#CCFLAGS =	-O3 -fastsse -DMPICH_IGNORE_CXX_SEEK -DLONG64 -DNODEF -DCHECK \
#		-Msafeptr -Mipa=fast 
LINK =		mpicc
LINKFLAGS =	-O
LIB =


# Link target

ALL= gups gups_vanilla
all: $(ALL)

gups: gups_single.c
	gcc -O3 -g gups_single.c -o gups

gups_vanilla:	gups_vanilla.o
	$(LINK) $(LINKFLAGS) gups_vanilla.o $(LIB) -o gups_vanilla

gups_nonpow2:	gups_nonpow2.o
	$(LINK) $(LINKFLAGS) gups_nonpow2.o $(LIB) -o gups_nonpow2

gups_opt:	gups_opt.o
	$(LINK) $(LINKFLAGS) gups_opt.o $(LIB) -o gups_opt

# Compilation rules

%.o:%.c
	$(CC) $(CCFLAGS) -c $<

.PHONY : clean

clean :
	rm gups gups_vanilla
