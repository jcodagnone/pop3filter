#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <config.h>

#ifdef HAVE_SNPRINTF
char *
itoa(int i,char *buf,size_t sizeofbuf)
{	int nret;

	nret = snprintf(buf, sizeofbuf,"%d",i);

	return nret ? buf : NULL;
}
#else

char *
itoa( int i, char *buf, size_t sizeofbuf)
{	char bigbigbigbuffer[4060]; /* :^) */
	int nRet;

	nRet = sprintf(bigbigbigbuffer,"%d",i);
	if( nRet +1 > sizeofbuf )
		return 0;
	strncpy(buf,bigbigbigbuffer,nRet+1);

	return buf;

}

#endif

