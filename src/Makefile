# $Id: Makefile,v 1.13 2002/06/27 15:56:38 juam Exp $ 

CFLAGS = -Wall  -g -pedantic -pedantic-errors -I../lib -I.. -Ddebug=1
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
