/*
 * $Id: strdup.c,v 0.0 2001/08/21 14:36:23 juam Exp $
 */
#include <stdlib.h>
#include <errno.h>
#include "debug.h"
#include "strdup.h"
        
char *
strdup(const char *s)
{	char *ptr;
        extern int errno;

	if( s == NULL)
	{	errno = EINVAL;
		return NULL;
	}
	ptr=malloc( strlen(s) + 1 );
	if( ! ptr )
	{	errno = ENOMEM ;
		return NULL;
	}

	return strcpy(ptr,s);
}

