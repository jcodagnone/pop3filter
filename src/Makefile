# $Id: Makefile,v 1.3 2002/06/18 15:50:13 juam Exp $ 

CFLAGS = -Wall  -g -pedantic -pedantic-errors -I../lib
OBJS = main.o pop.o process.o string.o
OUT = popfilter


all: $(OBJS)
	$(CC) $(OBJS) ../lib/libmisc.a  -o  $(OUT)

clean:
	- $(RM) $(OBJS) $(OUT)

Changelog:	RCS/*
		rcs2log > Changelog

# deps
main.o:		process.h ../lib/trace.h ../lib/newopt.h
process.o:	pop.h string.h
pop.o:		pop.h
string.o:	string.h
trace.o:	trace.h
