.PHONY = all statclib clean co

CFLAGS = -fPIC -I. -I.. -g -Wall -pedantic --pedantic-errors -I.

OBJS =  debug.o strdup.o newopt.o trace.o itoa.o
LIBOUT=libmisc.a

all:	staticlib 

Changelog: RCS/*
	rcs2log > Changelog
	
staticlib: $(OBJS)
	$(AR) cr $(LIBOUT) $(OBJS)

clean:
	- $(RM) -f $(OBJS) $(LIBOUT) 

co:
	$(CO) RCS/*

