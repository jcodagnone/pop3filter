#ifndef _MAIN_H_
#define _MAIN_H_

struct opt
{	short lport;		/* listening port */
	short rport;		/* remote port    */
	const char * server;	/* remote machine */
	const char *exec;	/* filter */
	int fork;		/* go to background ? */
};

#endif
