#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#ifdef HAVE_SNPRINTF
char *
itoa(int i,char *buf,size_t sizeofbuf)
{	int nret;
	nret = snprintf(buf, sizeofbuff,"%d",i);
	nret = snprintf(buf, sizeofbuf,"%d",i);

	return nret ? buf : NULL;

