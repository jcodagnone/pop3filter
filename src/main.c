/*
 * main -- filtered transparent pop3 proxy implementation
 * $Id: main.c,v 1.6 2002/06/19 15:49:06 juam Exp $
 *
 * Copyright (C) 2001,2002 by Juan F. Codagnone <juam@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <unistd.h>

#include "main.h"
#include "process.h"
#include "trace.h"
#include "newopt.h"

#ifndef VERSION 
 #define VERSION  "0.0.0"
#endif

const char *rs_program_name; 	/* for the logs */
const char *progname;
static void
help ( void )
{	printf (
	"Usage: %s [OPTION] lport server rport [filter]\n"
	"Creates a POP3 proxy using filter if available\n"
	"\n"
	"ARGUMENTS\n"
	"        lport               local port to listen connections\n"
	"        server              server to connect\n"
	"        rport               remote port to connect\n"
	"        filter              command that filters a message\n"
	"\n"
	"OPTIONS\n"
	/* X   X                      X */
	" -V   --version              print the version info and dies\n"
	" -h   --help                 prints this message\n"
	" -f   --fork                 fork to the background\n"
	"\n"
	"Send bugs to <juam at users dot sourceforge dot net>\n"
	"\n",progname);

	exit( EXIT_SUCCESS );

}

static void 
usage ( void )
{
	printf(
"%s [-hvf] [--help] [--version] [--fork] lport rhost remoteport [filter]\n",
	progname);

	exit( EXIT_SUCCESS );
}

static void
version( void )
{
	printf( "%s %s\n"
		"\n"
		"This is free software:\n"
		" There is NO warranty; not even for MERCHANTABILITY or\n"
		" FITNESS FOR A PARTICULAR PURPOSE\n",progname,VERSION);

	exit( EXIT_SUCCESS );
}

static int
parseOptions( int argc, char * const * argv, struct opt *opt)
{	int i;
	static optionT lopt[]=
	{	{"help",	OPT_NORMAL, 0,	OPT_T_FUNCT, (void *) help },
		{"h",		OPT_NORMAL, 1,  OPT_T_FUNCT, (void *) help },
		{"version",	OPT_NORMAL, 0,  OPT_T_FUNCT, (void *) version},
		{"V",		OPT_NORMAL, 1,  OPT_T_FUNCT, (void *) version},
		{"fork",	OPT_NORMAL, 0,  OPT_T_FLAG,  NULL },
		{"f",		OPT_NORMAL, 1,  OPT_T_FLAG,  NULL },
		{NULL}
	};	 lopt[5].data = lopt[6].data = (void *)  &(opt->fork);
	
	assert( argv && opt );
	memset(opt,0,sizeof(*opt) );
	i = GetOptions( argv, lopt, 0, NULL);
	if( i < 0 )
	{	rs_log_error("parsing options");
		return -1;
	}
	else if( argc - i < 3 )
	{	usage();
		return -1;
	}

	opt->lport  = atoi( argv[i] );
	opt->server = argv[i+1];
	opt->rport  = atoi( argv[i+2] );
	opt->exec = ( argc - i >= 4 ) ? argv[i+3] : NULL;

	if( opt->lport==0 || opt->rport == 0)
	{	rs_log_error("error port numbers are not integers");
		usage();
		return -1;
	}
	return 0;
}
/***************************************************************************/

/* Creates a socket to `szServer:port'
 */
static int
connectHost(const char *szServer,short port)
{ 	struct sockaddr_in server;
	struct hostent *hp;
	int sock;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{	rs_log_error("creating socket: %s",strerror(errno));
		return -1;
	}

	server.sin_family = AF_INET;
	hp = gethostbyname(szServer);
	if (hp == 0)
	{	rs_log_error("can't resolv `%s'",szServer);
		return -1;
	}
	memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

	server.sin_port = htons((short) port );
	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
	{	rs_log_error("connectig to `%s': %s",szServer,strerror(errno));
		return -1;
	}

	return sock;
}

/* Creates a listening socket on port `port'
 */
static int
createServer(short port)
{	struct sockaddr_in servAddr;
	int sd;

	/* create socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if( sd<0 )
	{	rs_log_error("creating socket: %s",strerror(errno));
 		return -1;
	}
 
	/* bind server port */
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);
 
	if(bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0)
	{	rs_log_error("binding socket: %s",strerror(errno));
	  	return -1;
	}
	
	if( listen(sd,5) == -1 )
	{	rs_log_error("listening socket: %s",strerror(errno));
		return -1;
	}
	
	return sd;
}

static int
child( int local, struct opt *opt )
{	int remote;

	remote = connectHost(opt->server,opt->rport);
	if( remote != -1 )
		do_server(local,remote,opt->exec);
	close(local);
	close(remote);

	exit(0);
}

/* 
 * run child() as a grandson process so we don't create zombies
 * (and we can work with a blocking accept(2) )
 */
static void
smart_fork( int local, struct opt *opt )
{	pid_t pid;

	pid = fork();
	if( pid == 0 )
	{	pid=fork();
		if( pid != 0 )
			exit(0);
		else
			child(local,opt);
	}
	close(local);
}

static void
fork_to_background ( void )
{	pid_t pid;

	pid = fork();
	if( pid > 0 )
		exit( EXIT_SUCCESS );
	else if ( pid == -1 )
		rs_log_error("going to the background: %s",strerror(errno));

	rs_log_info("running in the background");
	/* anythig else? */
	freopen ("/dev/null", "r", stdin);
	freopen ("/dev/null", "w", stdout);
	/* freopen ("/dev/null", "w", stderr); */ /* we are login to stderr */
	
	setsid();
	
	return;
}

int
main( int argc, char **argv )
{	int server,local;
	struct sockaddr_in cliAddr;
	struct opt opt;
	unsigned int size;

	progname = rs_program_name = *argv;
	rs_trace_to(rs_trace_stderr); 

	if( parseOptions( argc, argv, &opt ) < 0 )
		return EXIT_FAILURE;

	if( (server = createServer( opt.lport )) < 0 )
		return EXIT_FAILURE;

	if( opt.fork )
		fork_to_background();

	size = sizeof(cliAddr);
	while( (local=accept(server,(struct sockaddr *) &cliAddr,&size)) >=0 )
		smart_fork(local,&opt);	
	
	close(server);

	return EXIT_SUCCESS;
}

