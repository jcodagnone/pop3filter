/*
 * $Id: strdup.h,v 0.0 2001/08/21 14:36:23 juam Exp $
 *
 * STRDUP(3)	    Linux Programmer's Manual		STRDUP(3)
 *
 *
 *
 * NAME
 *      strdup - duplicate a string
 *
 * SYNOPSIS
 *        #include <string.h>
 *
 *       char *strdup(const char *s);
 *
 * DESCRIPTION
 *        The  strdup()  function	returns a pointer to a new string
 *        which is a duplicate of the string s.  Memory for the  new
 *        string  is  obtained with malloc(3), and can be freed with
 *        free(3).
 *
 * RETURN VALUE
 *       The strdup() function returns a pointer to the  duplicated
 *       string, or NULL if insufficient memory was available.
 *
 * ERRORS
 *       ENOMEM Insufficient memory available to allocate duplicate
 *	      string.
 *
 * CONFORMING TO
 *       SVID 3, BSD 4.3
 *
 * SEE ALSO
 *      calloc(3), malloc(3), realloc(3), free(3)
 *
 *
 *
 * GNU			    1993-04-12			STRDUP(3)
 */

#ifndef _STRDUP_H_
#define _STRDUP_H_

 #ifdef HAVE_STRDUP
   #include <string.h>
 #else
   #define strdup  strdup__
 #endif

char *
strdup__( const char *s ); 

#endif

