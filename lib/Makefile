.PHONY = all statclib clean co

CFLAGS = -fPIC -I. -I.. -g -Wall -pedantic --pedantic-errors -I.

OBJS =  debug.o strdup.o newopt.o trace.o itoa.o
LIBOUT=libmisc.a

all:	staticlib 

staticlib: $(OBJS)
	$(AR) cr $(LIBOUT) $(OBJS)

clean:
	- $(RM) -f $(OBJS) $(LIBOUT) 

co:
	$(CO) RCS/*

debug.o:	debug.h
strdup.o:	strdup.h
newopt.o:	newopt.h
trace.o:	trace.h ../config.h
itoa.o:		itoa.h ../config.h
