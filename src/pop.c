/*
 * pop -- 
 * $Id: pop.c,v 1.2 2002/06/18 15:58:55 juam Exp $
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
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include "pop.h"

#define MAX_POPCMD	76
#define MAX_POP3_RESPONSE       80

#define debug printf

struct cmd_table
{	char *name;
	enum cmds id;
} table[] =
{	{ "retr",	CMD_RETR},
	{ NULL,		CMD_UNKN}
};

static pid_t getfds(int *p, const char *exec );

static char *
ToLower( char *s )
{	char *q;

	for( q=s ; (*q = tolower(*q)) ; q++)
		;

	return s;
}

enum cmds 
ascii2cmd ( const char *p )
{	unsigned i,j,bye;
	char *q;
	enum cmds ret = CMD_UNKN;

	/* trim left */
	for( ; *p && isspace(*p) ; p++)
		;
	if( *p == 0 )
		return ret;

	for( i = 0 ; p[i] && !isspace(p[i]) ; i++ )
		;
	q = malloc( i +1 );
	if( q != NULL )
	{	strncpy( q, p, i);
		q[i] = 0;

		ToLower( q );
		for( bye=0,j=0; !bye && table[j].name ; j++ )
		{	if( strncmp(q,table[j].name,i) == 0 )
			{	ret = table[j].id ;
				bye = 1;
			}
		}

		free(q);
	}

	return ret;
}


/* pop3_error()
 *	Log an error as a pop3 error
 */
void
pop3_error(int socket,const char *fmt,...)
{	char buff[MAX_POP3_RESPONSE];
	va_list ap;
	int len;

	buff[0] = '-';
	buff[1] = ' ';
	va_start(ap,fmt);
	len = vsnprintf(buff+2,sizeof(buff)-5,fmt,ap);
	strncat(buff,"\r\n",sizeof(buff));
	buff[sizeof(buff)-1]='\0';
	send(socket,buff,strlen(buff),0);
	va_end(ap);
}

			 	
/*
 * retr_state
 *	Handle the RETR command
 */
static void
retr_state( struct global *d, const char *buf, size_t len )
{
	switch( d->retr )
	{	case RT_RESPONSE:
			if( send(d->local,buf,len,0) < 0 ||(len && buf[0]=='-'))
			{	d->failed = 1;
				d->retr = RT_END;
			}
			else
				d->retr = RT_BODY;
			break;
		case RT_BODY:
			if( !strcmp(buf,".\r\n") )
				d->retr = RT_END;
			else
				if( write(d->fd[PIPE_PAREN_WRITE],buf,len) <=0)
					d->retr = RT_END;
			break;
		case RT_END:
		default:
			assert(0);
			break;
	}
}

/* pop_local_read
 * 	reads POP3 commands
 * Params
 *	d	global data
 * Return
 *	-1	on error
 */
int
pop_local_read( struct global  *d, const char *buf)
{	int  len;
	int ret = 0;

	len = strlen(buf);
	if( d->exec )
		d->last_cmd = ascii2cmd(buf);
	else
		d->last_cmd = CMD_UNKN;

	if( d->last_cmd == CMD_RETR )
	{	d->retr= RT_RESPONSE;
		d->pid = getfds(d->fd,d->exec);
		if( d->pid  == -1 )
		{	pop3_error(d->local,"error execin' child");
			d->last_cmd = 0;
		}
		else
			send(d->remote,buf,len,0);

	}
	else
		send(d->remote,buf,len,0);

	return ret;
}

int
pop_remote_read( struct global *d, const char *buf )
{	int ret = 0;
	int len;


	d->failed = 0;
	len = strlen(buf);
	
	if( d->last_cmd == CMD_RETR )
	{	retr_state(d,buf,len);
		if( d->retr ==  RT_END  )
		{ 	close(d->fd[PIPE_PAREN_WRITE]);
			d->last_cmd = 0;
		}
	}
	else
	{
		if ( send(d->local,buf,len,0) < 0 )
			ret = -1;
	}

	return ret;
}

/******************************************************************************/
/* getfds()
 *	Opens 2 pipes for comunication between this process and
 *	what resuls of exec'ing 'exec'
 * Params
 *	p		array of 4 ints. these would be the file descriptors
 *	exec		command line that is  exec with "bash -c"
 * Return
 *	-1		on error
 *	>0		pid_t
 */
static pid_t
getfds(int *p, const char *exec )
{	pid_t pid;
	unsigned i;
	
	if( p== NULL || exec == NULL ||  pipe(p) == -1 )
		return -1;
	if( pipe(p+2) == -1)
	{	for( i = 0; i<2; i++)
			close(p[i]);

		return -1;
	}
	pid = fork();
	if( pid == -1 )
	{	for( i=0; i<4 ; i++)
			close(p[i]);
		return -1;
	}
	else if( pid == 0 )
	{	/* child:
		 *  we try to exec the users filter
		 *  if that fails we send data back
		 */
		char buf[MAX_POP3_RESPONSE];
		int len;
		
		close(p[PIPE_PAREN_WRITE]);
		close(p[PIPE_CHILD_READ]);
		dup2(p[PIPE_PAREN_READ],0);
		dup2(p[PIPE_CHILD_WRITE],1);

		if( WEXITSTATUS(system(exec)));
		{	
			while(( (len=read(p[PIPE_PAREN_READ],buf,sizeof(buf)))
				>0))
				write(p[PIPE_CHILD_WRITE],buf,len);
		}
		exit(0);
	}
	else
	{	/* parent */
		close(p[PIPE_PAREN_READ]);
		close(p[PIPE_CHILD_WRITE]);
	}

	return pid;
}

int
pop_child_read(struct global *d, char *buf,size_t size)
{	int ret = 0;
	int len;

	while( (len = read(d->fd[PIPE_CHILD_READ],buf, size)) >0 )
	{   
		if(!d->failed )
			if ( send(d->local,buf,len,0) < 0 )
				ret = -1;
	}
	if( len <= 0)
	{	
		if( !d->failed )
			if ( send(d->local,".\r\n",3,0) < 0 )
				ret = -1;
		close(d->fd[PIPE_CHILD_READ]);
		kill(d->pid,SIGKILL);
		waitpid(d->pid,NULL,0);
		d->pid=0;
	}
		
	return ret<=0 ? ret:len;
}
/*******************************************************************************/
