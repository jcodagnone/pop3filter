#ifndef _MAIN_H_
#define _MAIN_H_
#include <config.h>

#ifndef VERSION 
 #define VERSION  "0.0.0"
#endif

struct opt
{	short lport;		/* listening port */
	short rport;		/* remote port    */
	const char * server;	/* remote machine */
	const char *fstderr;	/* save child's stderr into a file */
	char *exec;		/* filter */
	int fork;		/* go to background ? */
	
};

#endif
