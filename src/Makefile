# $Id: Makefile,v 1.8 2002/06/19 17:39:35 juam Exp $ 

CFLAGS = -Wall  -g -pedantic -pedantic-errors -I../lib
OBJS = main.o pop.o process.o string.o
OUT = pop3filter


all: $(OBJS)
	$(CC) $(OBJS) ../lib/libmisc.a  -o  $(OUT)

clean:
	- $(RM) $(OBJS) $(OUT) core 

Changelog:	RCS/*
		rcs2log > Changelog

# deps
main.o:		process.h ../lib/trace.h ../lib/newopt.h main.h
process.o:	pop.h string.h main.h
pop.o:		pop.h main.h ../lib/itoa.h
string.o:	string.h
