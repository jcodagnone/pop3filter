/*
 * string.c  -- String object implemetation
 *
 * Copyright (C) 2002 by Juan F. Codagnone <juam@users.sourceforge.net>
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

#include <stdlib.h>
#include <string.h>

#include "mstring.h"

enum {
	INIT_CHUNK   = 512,
	CHUNK_FACTOR = 2
};

struct stringCDT
{
	char *s;	/* string */
	size_t len;	/* string length */
	size_t size;	/* total allocated size */
};

/**
 * resize an string_t
 * 
 * @param 	s	string to resize
 * @param	min	minimun value to resize
 * 
 * \returns NULL on error, else a pointer to the new address
 */
static char *
_string_resize( string_t s, size_t min )
{	size_t new = CHUNK_FACTOR * s->size;
	char *q;
	
	if( min < s->size )
		return s->s;
	else if( new > min ) 
		s->size = new;
	else
		s->size = min + INIT_CHUNK; /* we are generous to avoid calls */

	q = realloc( s->s, s->size );
	if( q != NULL )
		s->s = q;
		
	return q;
}

string_t
string_new( void )
{ 	string_t s;

	s = malloc ( sizeof( *s) );
	if( s == NULL )
		return NULL;

	s->len = 0;
	s->size = INIT_CHUNK;
	s->s = malloc( s->size );
	if( s->s == NULL )
		free( s );
	else
		s->s[0]=0;;
	
	return s;
}


void
string_destroy( string_t s )
{
	if( s != NULL )
	{
		free( s->s );
		free( s );
	}

}

int
string_n_append( string_t s, const char *q, size_t len)
{	char *nRet=(char *)1;
	
	if( s == NULL || q == NULL )
		return -1;

	if( len + s->len + 1 >= s->size )
		nRet = _string_resize( s, len + s->len + 1 );
	
	if( nRet )
	{	s->len += len;
		strncat( s->s, q, len);
		s->s[ s->len ] = '\0';
	}
	
	return nRet ? 0 : -1;
}

int
string_append( string_t s, const char *q)
{
	if( s == NULL || q == NULL )
		return -1;
		
	return string_n_append( s, q, strlen(q) );
}

const char *
string_get_as_ansi( string_t s )
{
	if( s == NULL )
		return NULL;
		
	return s->s;
}

int
string_reset( string_t s )
{
	if( s == NULL )
		return -1;

	s->s[0] = '\0';
	s->len  = 0;

	return -1;
}

#ifdef _STRING_TESTDRIVER_
int
main(void)
{	string_t s;

	s =  string_new();

	string_append(s,"pepe");
	string_append(s," hola mundo");
	/* string_reset(s); */
	printf("|%s|\n", string_get_as_ansi(s));
	
	string_destroy(s);

	return 0;

}

#endif	/* _STRING_TESTDRIVER_ */
