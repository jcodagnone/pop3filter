
#include <stdlib.h>
#include <string.h>

#include "string.h"

#define INIT_CHUNK	80
#define CHUNK_FACTOR	2

struct stringCDT
{
	char *s;	/* string */
	size_t len;	/* string length */
	size_t size;	/* total allocated size */
};

/* resize_string
 * 	resize an string
 * Parameters
 *	s	string to resize
 *	min	minimun value to resize
 * Returns
 *	NULL	on error
 *	else	a pointer to the new address
 */
static char *
resize_string( string_t s, size_t min )
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
NewString( void )
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
FreeString( string_t s )
{
	if( s != NULL )
	{
		free( s->s );
		free( s );
	}

	return;
}

int
StringNCat( string_t s, char *q, size_t len)
{	char *nRet=(char *)1;
	
	if( s == NULL || q == NULL )
		return -1;

	if( len + s->len + 1 >= s->size )
		nRet = resize_string( s, len + s->len + 1 );
	
	if( nRet )
	{	s->len += len;
		strncat( s->s, q, len);
		s->s[ s->len ] = '\0';
	}
	
	return nRet ? 0 : -1;
}

int
StringCat( string_t s, char *q)
{
	if( s == NULL || q == NULL )
		return -1;
		
	return StringNCat( s, q, strlen(q) );
}

const char *
GetAnsiString( string_t s )
{
	if( s == NULL )
		return NULL;
		
	return s->s;
}

int
StringClean( string_t s )
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

	s =  NewString();

	StringCat(s,"pepe");
	StringCat(s," hola mundo");
	/* StringClean(s); */
	printf("|%s|\n",GetAnsiString(s));
	
	FreeString(s);

	return 0;

}

#endif