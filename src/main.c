/*
 * ASCII PORT TCP FORWARDER  (test it as POP3 transparent proxy)
 * SAVES ALL CONNECTION IN A FILE LOCATED IN SZSIZE
 *
 * (c) 2001 Juan F. Codagnone <juam [at] users.sourceforge.net>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <unistd.h>

#include "process.h"
#include "trace.h"

struct hostent *gethostbyname();

struct opt
{	short lport;		/* listening port */
	short rport;		/* remote port    */
	const char * server;	/* remote machine */
	char *exec;		/* filter */
};

const char *rs_program_name;

/*
 * creates a socket to `szServer:port'
 */
static int
createconnection(const char *szServer,short port)
{ 	struct sockaddr_in server;
	struct hostent *hp;
	int sock;
	char buf[1024];

	/*  Create socket  */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{	perror("-ERR socket\r\n");
		return -1;
	}

	/*  Connect socket  */
	server.sin_family = AF_INET;
	strcpy(buf,szServer);
	hp = gethostbyname(buf);
	if (hp == 0)
	{	fprintf(stderr, "-ERR unknown host\r\n");
		return -1;
	}
	memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

	server.sin_port = htons((short) port );
	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
	{	perror("-ERR connecting stream socket\r\n");
		return -1;
	}

	return sock;
}

/*
 * creates a listening socket on port `port'
 */
static int
makeserver(short port)
{	struct sockaddr_in servAddr;
	int sd;

	/* create socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if( sd<0 )
	{	perror("-ERR socket ");
 		exit(1);
	 }
 
	/* bind server port */
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);
 
	if(bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0)
	{	perror("-ERR bind");
	  	exit(1);
	}
	
	listen(sd,5);

	return sd;
}

static void
usage ( const char * const progname )
{	printf( "Usage: %s lport server rport [filter]\n"
		"\tlport\tlocal port to listen connections\n"
		"\tserver\tserver to connect\n"
		"\trport\tremote port to connect\n"
		"\tfilter\tcommand that filters a message\n"
		"\n"
		"Send bugs to <juam at users dot sourceforge dot net>\n",
		progname);

	return ;

}

static int
parseOptions( int argc, char * const * argv, struct opt *opt)
{	if( opt == NULL )
		return -1;
	else if( argc < 4 )
	{	usage(*argv);
		return -1;
	}

	opt->lport  = atoi( argv[1] );
	opt->server = argv[2];
	opt->rport  = atoi( argv[3] );
	opt->exec = ( argc >= 5 ) ? argv[4] : NULL;

	if( opt->lport==0 || opt->rport == 0)
	{	usage(*argv);
		return -1;
	}
	return 0;
}

static int
child( int local, struct opt *opt )
{	int remote;

	remote = createconnection(opt->server,opt->rport);
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
	if( pid != 0 )
	{	pid=fork();
		if( pid != 0 )
			exit(0);
		else
			child(local,opt);
	}
	close(local);
}

int
main(int argc, char **argv)
{	int server,local;
	struct sockaddr_in cliAddr;
	struct opt opt;
	unsigned int size;

	rs_program_name = *argv;

	if( parseOptions( argc, argv, &opt ) < 0 )
		return 1;

	server = makeserver( opt.lport );

	size = sizeof(cliAddr);
	while( (local=accept(server,(struct sockaddr *) &cliAddr,&size)) >=0 )
	{	smart_fork(local,&opt);	
	}

	close(server);

	return 0;
}

