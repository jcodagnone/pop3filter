CFLAGS = -Wall  -g -pedantic -pedantic-errors
OBJS = proxy.o pop.o process.o string.o trace.o
OUT = popfilter

all: $(OBJS)
	$(CC) $(OBJS) -o  $(OUT)

clean:
	- $(RM) $(OBJS) $(OUT)

proxy.o:	process.h trace.h newopt.h
process.o:	pop.h string.h
pop.o:		pop.h
string.o:	string.h
trace.o:	trace.h
