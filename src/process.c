/*
 * process -- 
 * $Id: process.c,v 1.2 2002/06/18 15:58:55 juam Exp $
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

#include "pop.h"
#include "string.h"


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


static  void
do_server_init( int local, int remote, char *dataptr,struct global *data,
                 fd_set *r,fd_set *w,fd_set *e)
{
	data->local = local;
	data->remote = remote;
	data->last_cmd = 0;
	data->exec = dataptr;
	data->pid = 0;

	FD_ZERO(r);
	FD_SET(local,r);
	FD_SET(remote,r);

	FD_ZERO(w);
	FD_SET(local,w);
	FD_SET(remote,w);
	
	FD_ZERO(e);
	FD_SET(local,e);
	FD_SET(remote,e);

	return ; 
}

/*
 * async proxy loop (yes, This is ugly.)
 */
int
do_server(int local, int remote, void *dataptr )
{	struct global data;
	char buf[MAX_BUFF];
	string_t lstring,rstring;
	int nRet=0;
	fd_set rfds,rback,  wfds,wback, efds,eback;
	
	do_server_init(local,remote,dataptr,&data,&rfds,&wfds,&efds);
	lstring = NewString();
	rstring = NewString();
	if( lstring == NULL || rstring == NULL )
	{	FreeString(lstring);
		FreeString(rstring);
		return -1;
	}
	rback=rfds, eback=efds, wback=wfds;
	
	while( nRet != -1 && select(FD_SETSIZE,&rback,NULL,&eback,NULL)>0 )
	{	
		/* check the pipe first    */
		if( data.pid &&
		   FD_ISSET( data.fd[PIPE_CHILD_READ],&rback) )
		{
			nRet = pop_child_read(&data,buf,sizeof(buf));
		}

		if( FD_ISSET(local,&eback) || FD_ISSET(remote,&eback) )
		{
			break; /* uggg */
		}

		if( FD_ISSET(local,&rback) )
		{
			nRet = readsock(local,buf,sizeof(buf),lstring,&data,
			             pop_local_read);
		}

		if( FD_ISSET(remote,&rback) )
			 nRet = readsock(remote,buf,sizeof(buf),rstring,&data,
			             pop_remote_read);
		/*
		 * if( FD_ISSET(local,&wback) )
		 * 	 do_write_local(&data);
		 *
		 * if( FD_ISSET(remote,&wback) )
		 *  	 do_write_remote(&data);
		 */


		rback=rfds, eback=efds, wback=wback;
		/* restore flags */
		if( data.pid  )
		{	FD_SET(data.fd[PIPE_CHILD_READ],&rback);
		}

		
	}
	
	FreeString(lstring);
	FreeString(rstring);
	
	return 0;
}

