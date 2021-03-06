/*
 * main -- filtered transparent pop3 proxy implementation
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

#ifdef HAVE_CONFIG_H
   #include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#ifdef HAVE_SYSLOGD
 #include <syslogd.h>
#endif

#include <libcrash/sigsegv.h>

#include "config.h"
#include "main.h"
#include "process.h"
#include "trace.h"
#include "newopt.h"


const char *rs_program_name; 	/* for the logs */
const char *progname;

/* uggh. i need to mantain a global var with the server socket so we proper
 * clean it when some one kill us
 */
static int serverSocket = -1;

static void
help ( void )
{	printf (
	"Usage: %s [OPTION] server rport lport [filter]\n"
	"Creates a POP3 proxy using filter if available\n"
	"\n"
	"ARGUMENTS\n"
	"        server              server to connect\n"
	"        rport               remote port to connect\n"
	"        lport               local port to listen connections\n"
	"        filter              command that filters a message\n"
	"\n",progname);
	printf (
	"OPTIONS\n"
	/* X   X                      X */
	" -V   --version              print the version info and dies\n"
	" -h   --help                 prints this message\n"
	" -f   --fork                 fork to the background\n"
	" -e file                     write filter stderr to file\n"
	"      --listen <address>     listen to an specific interface\n"
	"      --user   <username>    run server as user <username>\n"
	"      --group  <groupname>   run server as group <groupname>\n"
	"\n"
	"Send bugs to <juam at users dot sourceforge dot net>\n"
	"\n");

	exit( EXIT_SUCCESS );
}

static void 
usage ( void )
{
	printf(
"%s [-hVf] [-e file] [--help] [--version] [--fork] [--listen <address>] rhost rport lport [filter]\n",
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
	{/*00*/	{"help",	OPT_NORMAL, 0,	OPT_T_FUNCT, (void *) help },
	 /*01*/	{"h",		OPT_NORMAL, 1,  OPT_T_FUNCT, (void *) help },
	 /*02*/	{"version",	OPT_NORMAL, 0,  OPT_T_FUNCT, (void *) version},
	 /*03*/	{"V",		OPT_NORMAL, 1,  OPT_T_FUNCT, (void *) version},
	 /*04*/	{"fork",	OPT_NORMAL, 0,  OPT_T_FLAG,  NULL },
	 /*05*/	{"f",		OPT_NORMAL, 1,  OPT_T_FLAG,  NULL },
	 /*06*/	{"e",		OPT_NORMAL, 1,	OPT_T_GENER, NULL },
	 /*07*/	{"listen",	OPT_NORMAL, 0,	OPT_T_GENER, NULL },
	 /*08*/	{"user",	OPT_NORMAL, 0,	OPT_T_GENER, NULL },
	 /*09*/	{"group",	OPT_NORMAL, 0,	OPT_T_GENER, NULL },
	 	{NULL}
	};	 lopt[4].data = lopt[5].data = (void *)  &(opt->fork);
	         lopt[6].data = (void *) &(opt->fstderr);
		 lopt[7].data = &(opt->listen_addr);
		 lopt[8].data = &(opt->username);
		 lopt[9].data = &(opt->groupname);
	
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
	opt->server =       argv[i+0]  ;
	opt->rport  = atoi( argv[i+1] );
	opt->lport  = atoi( argv[i+2] );
	opt->exec = ( argc - i >= 4 ) ? argv[i+3] : NULL;

	if( argc - i >= 5 )
		rs_log_warning("parameters after the filter. "
		               "If they are for the filter please escape them");
		
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
connectHost(const char *szServer, short port)
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
	{	rs_log_error("connectig to `%s': %s",szServer, strerror(errno));
		return -1;
	}

	return sock;
}

/** creates a listening socket on port `port' on listen_addr (if != null) */
static int
pop3filter_socket_listen(const char *listen_addr, short port)
{	struct sockaddr_in servAddr;
	int sd = -1;

	/* create socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if( sd<0 )
	{	rs_log_error("creating socket: %s",strerror(errno));
 		sd = -1;
	}
 	else
	{	servAddr.sin_family = AF_INET;
		servAddr.sin_port = htons(port);
		if( listen_addr ) 
		{	if (!inet_aton(listen_addr, &servAddr.sin_addr))
			{	rs_log_error("invalid IPv4 address: %s",
				              listen_addr);
				close(sd);
				sd = -1;
			}
		}
		else
			servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		if( sd == -1 )
			;
		else if(bind(sd, (struct sockaddr *) &servAddr, 
		             sizeof(servAddr)) == -1 )
	 	{	if( listen_addr )
				rs_log_error("binding socket (%s): %s",
				             listen_addr, strerror(errno));
			else
				rs_log_error("binding socket: %s",
				             strerror(errno));
			close(sd);
			sd = -1;
		} 
		else if( listen(sd,5) == -1 )
		{	rs_log_error("listening socket: %s",strerror(errno));
			close(sd);
			return -1;
		}
	}
	
	return sd;
}

/* procedure for each incomming connection
 */
static int
child( int local, const struct opt *opt )
{	int remote;

	remote = connectHost(opt->server, opt->rport);
	if( remote != -1 )
		proxy_run(local, remote, opt);
	close(local);
	close(remote);

	exit(0);
}

/* 
 * run child() as a grandson process so we don't create zombies
 * (and we can work with a blocking accept(2) )
 */
static void
smart_fork( int local, const struct opt *opt )
{	pid_t pid;

	pid = fork();
	if( pid == 0 )
	{	pid=fork();
		if( pid != 0 )
			exit(0);
		else
		{	signal(SIGTERM, SIG_DFL);
			signal(SIGINT, SIG_DFL);
			child(local, opt);
		}
	}
	wait(NULL);
	close(local);
}

static void
fork_to_background ( void )
{	pid_t pid;

	rs_trace_to(rs_trace_syslog);
	pid = fork();
	if( pid > 0 )
		exit( EXIT_SUCCESS );
	else if ( pid == -1 )
		rs_log_error("going to the background: %s",strerror(errno));

	/* anythig else? */
	freopen ("/dev/null", "r", stdin);
	freopen ("/dev/null", "w", stdout);
	freopen ("/dev/null", "w", stderr);
	
	setsid();

	rs_log_info("running in the background");

	return;
}

static void 
hndl_sigterm( int signal )
{	
	/* Note: we don't kill our childs so transactions are finished
	 */

	rs_log_info("signal %d, cleaning up and exiting",signal);
	
	/* proper close of the server socket */
	close(serverSocket);

	exit(EXIT_SUCCESS);
}

static int
standalone_server( const struct opt *opt)
{ 	socklen_t size;
	struct sockaddr_in cliAddr;
	int server, local;
	int ret = EXIT_FAILURE;
	
	if( (server = pop3filter_socket_listen(opt->listen_addr, opt->lport))<0)
		ret = EXIT_FAILURE;
	else {	
		serverSocket = server;	
		signal(SIGTERM, hndl_sigterm);
		signal(SIGINT,  hndl_sigterm);

		if( opt->fork )
			fork_to_background();

		size = sizeof(cliAddr);
		while( (local = accept(server,(void *) &cliAddr,&size)) >=0 ) 
		{	rs_log_info("incomming connection from %s:%d",
			             inet_ntoa(cliAddr.sin_addr),
				     ntohs(cliAddr.sin_port));

			smart_fork(local, opt);	
		}
		
		close(server);
		ret = EXIT_SUCCESS;
	}
	
	return ret;
}

static int
inetd_server( const struct opt* opt)
{
	rs_trace_to(rs_trace_syslog);
	rs_log_info("stdin is socket; assuming inetd mode");

	if( opt->fork )
		rs_log_info("inetd mode: ignoring fork flag");
	
	return child( STDIN_FILENO, opt );
}

static int
is_a_socket(int fd)
{ 	int v;
	socklen_t l;
	
	l = sizeof(int);
	return (getsockopt(fd, SOL_SOCKET, SO_TYPE, (char *) &v, &l) == 0);
}


int
main( int argc, char **argv )
{	struct opt opt;
	int nRet = EXIT_SUCCESS;

	/* logger support */
	open_syslogd();
	progname = rs_program_name = *argv;
	rs_trace_to(rs_trace_stderr); 
	
	signal(SIGSEGV, sigsegv_handler_bt_full_fnc);
	
	if( parseOptions( argc, argv, &opt ) < 0 )
		nRet = EXIT_FAILURE;
	else if( (opt.username || opt.groupname) && 
	    switch_to(opt.username, opt.groupname) == -1 )
	{
	    	rs_log_error("switching to username");
		nRet = EXIT_FAILURE;
	}
	else if( is_a_socket(STDIN_FILENO))
		nRet = inetd_server( &opt );
	else
		nRet = standalone_server( &opt );

	close_syslogd();

	return nRet;
}

