/*
 *  $Id: debug.h,v 0.0 2001/06/17 23:33:13 juam Exp $
 */
#ifndef _DEBUG_H_
#define _DEBUG_H_

enum {  DEBUG_NONE,
	DEBUG_NORMAL,
	DEBUG_VERBOSE,
	DEBUG_VERY_VERBOSE,
	DEBUG_ALL
     };

void
debug(unsigned level,const char *fmt,...);


#endif 
