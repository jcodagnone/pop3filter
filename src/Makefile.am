AM_CFLAGS = @CFLAGS@ -I$(srcdir)/../lib -Ddebug=1
AM_LDFLAGS = @LDFLAGS@
LIBS = @LIBS@ -L../lib -lmisc

bin_PROGRAMS = pop3filter

pop3filter_SOURCES =	access.c	\
			access.h	\
			main.c  	\
			main.h  	\
			pop.c   	\
			pop.h   	\
			process.c	\
			process.h	\
			string.c	\
			mstring.h	\
			queue.c  	\
			queue.h  	
