/* 
 * $Id: debug.c,v 0.0 2001/06/13 14:54:59 juam Exp $
 *
 * Funciones para debugear el programa
 *
 */
#include <stdio.h>
#include <stdarg.h>
#include "debug.h"


#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 20
#endif

void
debug(unsigned level,const char *fmt,...)
{  va_list ap;

   if( level <= DEBUG_LEVEL )
   {   va_start(ap,fmt);
       vfprintf(stderr,fmt,ap);
       va_end(ap);  
   }

}
