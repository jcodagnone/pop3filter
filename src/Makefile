# $Id: Makefile,v 1.4 2002/06/18 17:14:12 juam Exp $ 

CFLAGS = -Wall  -g -pedantic -pedantic-errors -I../lib
OBJS = main.o pop.o process.o string.o
OUT = popfilter


all: $(OBJS)
	$(CC) $(OBJS) ../lib/libmisc.a  -o  $(OUT)

clean:
	- $(RM) $(OBJS) $(OUT) core 

Changelog:	RCS/*
		rcs2log > Changelog

# deps
main.o:		process.h ../lib/trace.h ../lib/newopt.h
process.o:	pop.h string.h
pop.o:		pop.h
string.o:	string.h
trace.o:	trace.h
