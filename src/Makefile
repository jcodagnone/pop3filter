# $Id: Makefile,v 1.12 2002/06/24 02:35:00 juam Exp $ 

CFLAGS = -Wall  -g -pedantic -pedantic-errors -I../lib -I..
OBJS = main.o pop.o process.o string.o access.o
OUT = pop3filter


all: $(OBJS) ../lib/libmisc.a
	$(CC) $(OBJS) ../lib/libmisc.a  -o  $(OUT)

clean:
	- $(RM) $(OBJS) $(OUT) core 

# deps
main.o:		process.h ../lib/trace.h ../lib/newopt.h main.h
access.o:	access.h ../lib/trace.h main.h
process.o:	pop.h string.h main.h
pop.o:		pop.h main.h ../lib/itoa.h
string.o:	string.h
