/*
 * process -- 
 * $Id: process.c,v 1.9 2003/01/19 19:56:51 juam Exp $
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

# include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <signal.h>
#include <unistd.h>

#include "trace.h"
#include "mstring.h"
#include "pop.h"
#include "access.h"


#define MAX_BUFF		4096

/*******************************/


/*
 * return
 *	<0	error or end of file
 *	 0	no new line. buff concatenated
 *	 1 	was a new line. in buff is stored the rest
 *	 2	same as 1 but there are more newlines
 */
int
readsock( int socket, char *buff, size_t size, string_t string,struct global *d,
int (*p)(struct global  *, const char *) )
{	int len;
	int nRet=0;
	char *s;
	
	len = recv(socket,buff,size-1,0);
	if( len <= 0 )
		return -1;
	else
	{	buff[len]='\0';
		while( !nRet && (s=strchr( buff,'\n')) )
		{	StringNCat( string,buff, s - buff +1);
			buff = s+1;
			nRet = (*p)(d,GetAnsiString(string));
			StringClean(string);
		}
		StringCat( string, buff );
	}

	return nRet;
}


static int
proxy_init( int local, int remote, const struct opt *opt,struct global *data,
                 fd_set *r,fd_set *w)
{
	data->local = local;
	data->remote = remote;
	data->last_cmd = 0;
	data->pid = 0;
	data->opt = opt;
	data->username[0] = '\0';

	FD_ZERO(r);
	FD_SET(local,r);
	FD_SET(remote,r);

	FD_ZERO(w);

	data->queue_fifo   = queue_new();
	data->queue_remote = queue_new();
	data->queue_local  = queue_new();
	
	if( ! queue_is_valid(data->queue_fifo  ) ||
	    ! queue_is_valid(data->queue_remote) ||
	    ! queue_is_valid(data->queue_local ) )
	{	queue_delete(data->queue_fifo);
		queue_delete(data->queue_remote);
		queue_delete(data->queue_local );
		
		return -1;
	}

	data->fd[0] = data->fd[1] = data->fd[2] = data->fd[3] = -1;

	return 0; 
}

static void
proxy_delete( struct global *data )
{
	queue_delete(data->queue_fifo);
	queue_delete(data->queue_remote);
	queue_delete(data->queue_local );
}

/*
 * async proxy loop (yes, This is ugly.)
 */
int
proxy_request ( int local, int remote, struct opt *opt )
{	struct global data;
	char buf[MAX_BUFF];
	string_t lstring,rstring;
	int nRet=0;
	fd_set rfds,rback,  wfds,wback;
	char *s;
	size_t len;
	
	if( !client_access( local, opt))
		return -1;

	if( proxy_init(local,remote,opt,&data,&rfds,&wfds) == -1 )
		return -1;

	lstring = NewString();
	rstring = NewString();
	if( lstring == NULL || rstring == NULL )
	{	FreeString(lstring);
		FreeString(rstring);
		proxy_delete(&data);

		return -1;
	}

	rback=rfds;
	wback=wfds;
	while( nRet != -1 && select(FD_SETSIZE,&rback,&wback,NULL,NULL)>0 )
	{	
		/* read from the filter process */
		if( data.fd[PIPE_CHILD_READ] != -1 && 
		    FD_ISSET(data.fd[PIPE_CHILD_READ],&rback) )
			nRet = pop_child_read(&data,buf,sizeof(buf));
			
		/* write to the filter process */
		if( data.fd[PIPE_PAREN_WRITE] != -1 &&
		    FD_ISSET( data.fd[PIPE_PAREN_WRITE],&wback) )
		{	
			s = queue_dequeue(data.queue_fifo,&len);
			if( s )
			{	write(data.fd[PIPE_PAREN_WRITE],s,len);
				free(s);
			}
		}

		/* read commands from local socket */
		if( FD_ISSET(local,&rback) )
			nRet = readsock(local,buf,sizeof(buf),lstring,&data,
			             pop_local_read);

		/* read responces from pop3 server */
		if( FD_ISSET(remote,&rback) )
			nRet = readsock(remote,buf,sizeof(buf),rstring,&data,
			             pop_remote_read);

		/* write to sockets */
		if( FD_ISSET(local,&wback) )
		{	s = queue_dequeue(data.queue_local,&len);
			send(local,s,len,0);
			free(s);
		}

		if( FD_ISSET(remote,&wback) )
		{	s = queue_dequeue(data.queue_remote,&len);
			send(remote,s,len,0);
			free(s);
		}

		/* restore flags */
		rback=rfds;
		wback=wback;
		if( data.fd[PIPE_CHILD_READ] != -1 )
			FD_SET(data.fd[PIPE_CHILD_READ],&rback);

		if( data.fd[PIPE_PAREN_WRITE] != -1 )
		{	if( !queue_is_empty(data.queue_fifo ) )
				FD_SET(data.fd[PIPE_PAREN_WRITE],&wback);
			else
				FD_CLR(data.fd[PIPE_PAREN_WRITE],&wback);
		}
		else
			 FD_CLR(7,&wback);

		if( !queue_is_empty(data.queue_local) )
			FD_SET(data.local,&wback);
		else
			FD_CLR(data.local,&wback);
			
		if( !queue_is_empty(data.queue_remote) )
			FD_SET(data.remote,&wback);
		else
			FD_CLR(data.remote,&wback);

		if( data.last_cmd == CMD_RETR &&
		    data.retr     == RT_END   &&
		    data.fd[PIPE_PAREN_WRITE] != -1 &&
		    queue_is_empty(data.queue_fifo))
		{
			FD_CLR(data.fd[PIPE_PAREN_WRITE],&wback);
			close (data.fd[PIPE_PAREN_WRITE]);
			data.fd[PIPE_PAREN_WRITE] = -1;
		}
	}

	/* if something failed, try to clean buffers */
	nRet = 1;
	while(nRet>0 && (s=queue_dequeue(data.queue_fifo,&len)))
	{ 	nRet = write(data.fd[PIPE_PAREN_WRITE],s,len);
		free(s);
	}
	nRet = 1;
	while(nRet>0 && (s=queue_dequeue(data.queue_remote,&len)))
	{ 	send(data.remote,s,len,0);
		free(s);
	}
	nRet = 1;
	while(nRet>0 && (s=queue_dequeue(data.queue_local,&len)))
	{ 	send(data.local,s,len,0);
		free(s);
	}
		
	proxy_delete(&data);
	FreeString(lstring);
	FreeString(rstring);
	
	return 0;
}

