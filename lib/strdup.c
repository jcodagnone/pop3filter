/*
 * $Id: strdup.c,v 0.1 2003/01/17 17:42:58 juam Exp $
 */
#include <stdlib.h>
#include <string.h>
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

