#ifndef _PROCESS_H_
#define _PROCESS_H_

/* proxy_request
 * 	Do what ever the proxie do
 * Params
 * 	lsock	local socket 
 * 	rsock	remote socket (server's socket)
 * 	opt	program options
 * Return 
 * 	TODO return
 */
int
proxy_request ( int lsock, int rsock, const struct opt *opt );


#endif
