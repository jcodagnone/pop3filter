#ifndef _PROCESS_H_
#define _PROCESS_H_

/* do_server()
 * 	Do what ever the proxie do
 * Params
 * 	lsock	local socket 
 * 	rsock	remote socket (server's socket)
 * 	data	extra data
 * Return 
 * 	TODO return
 */
int
do_server(int lsock, int rsock,void *data );


#endif
