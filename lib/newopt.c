/*
 * $Id: newopt.c,v 0.0 2001/09/30 15:52:00 juam Exp $
 *
 * Command line options parser inspired in getopt(3) but unlike that,
 * this one _is_ THREAD SAFE
 *
 */
#include <stdio.h> 	/* for sscanf */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <assert.h>
#include <debug.h>


#include <strdup.h>
#include "newopt.h"

struct global
{	char * const * argv;	/* argv as main */
	const optionT *table;	/* option table */
	int i;			/* index in argv we are */
	const char *pt;		/* acutal position in argv[i] */
	unsigned flags;
	void *res;
	const char *arg;
};

/*
 * Initiliazation dependences 
 */

/*
 * is 'str' alphabetic?
 */
static int
buffIsAlpha( const char *str )
{	
	if( str == NULL )
		return 0;

	for(; *str && isalpha(*str) ;str++)
		;

	return !*str;
}

#define valid_type(f)	( (f)>=0 && (f)<OPT_T_INVALID)
#define valid_flag(t)	( (t)>=0 && (t)<OPT_INVALID)

/*
 * Checks:
 *   name not null and alphabetic
 *   valid type of variable 
 *
 *  Returns:
 *	<0 if there is an error
 *
 *  NOTE we don't check colitions yet
 *  NULL data is fine
 */
static int
check_table( const optionT * opt )
{	int i;

	assert( opt );

	/* TODO: ADD TO CHECK REPETITIONS 
	 */
	for( i=0; opt[i].name && buffIsAlpha(opt[i].name) &&
	     valid_type(opt[i].type) && valid_flag(opt[i].flags) ; i++ )
		;
	return opt[i].name == NULL ? -i:0;
}

/*
 * do sanity checks for GetOptions() and fills 'data'
 */
static int
do_sanety( struct global *data,char *const *argv,  const optionT *opt, 
                  unsigned flags, void *reserved )
{	int nRet=0;

	assert( data && argv && opt );

	if( !argv )
		nRet = -1;
	else if( opt==NULL || !check_table(opt)  )
		nRet = -1;
	else if( flags <0 && flags >= OPT_F_INVALID )
		nRet = -1;
	data->argv = argv;
	data->table = opt;
	data->i = 1;
	data->pt = NULL;
	data->flags = flags;
	data->res = reserved;
	return nRet;
}

/***/

#define is_option(a) (  (a)[0]=='-' &&  \
		    ( (a)[1]=='-' ? (a)[2]!='\0' : (a)[1]!='\0' ) )

#define force_end(str) ( !strcmp(str,"--" ) )

#define is_long_opt(s)  ( (s)[0]=='-' && (s)[1]=='-' && (s)[2]!='\0' )

#define current_arg(s) ( (s)->argv[(s)->i] )



/*
 * finds a short option in the table
 * 
 *  returns the index or -1
 */
static int
findOption( const optionT *table, int c , const char *str)
{	int i;
	int found;
	
	assert(table);

	/*
	 * TODO: extend search for long option and incompleate str
	 */
	for( found=i=0; table[i].name && !found ; i++ )
	{	if( str && !table[i].short_opt && !strcmp(str,table[i].name) )
			found=1;
		if( !str && table[i].short_opt && strchr(table[i].name,c) )
			found = 1;
	}
	
	return found? i-1 : -1;
}

/*
 * TODO: add soport to +- if a number is nest
 */
#define findShortOption(table,c) ( findOption((table),(c),NULL) )
#define findLongOption(table,s)  ( findOption((table),0,s) )

enum
{	RET_UNKNOW=-1,
	RET_ENOUGH=-2,
	RET_END=-3
};

#define has_no_arg(t)  ((t).type==OPT_T_FLAG || (t).type==OPT_T_FUNCT )
#define has_one_arg(t) ((t).type==OPT_T_INT || (t).type==OPT_T_LONG || \
				(t).type==OPT_T_FLOAT || (t).type==OPT_T_GENER )
/*
 * get aditionals parameters of an option if any.
 * <0 error. else the position in the table of the command
 */
static int
do_short_opt( struct global *data )
{	int nRet;
	int n;

	assert( data );
	nRet = *(data->pt++);
	if( !*data->pt )
	{	data->pt = NULL;
		data->i++;
	}
	
	n = findShortOption( data->table,nRet);

	if( n < 0 )
		n = RET_UNKNOW;
	else if( has_one_arg( data->table[n] ) )
	{	if( data->pt )
		{	data->arg = data->pt;
			data->pt = NULL;
			data->i++;
		}
		else if ( !current_arg(data) || force_end(current_arg(data)) ||
			  is_option( current_arg(data) ) )
				n = RET_ENOUGH;
		else
		{	data->arg = current_arg(data);
			data->i++;
		}
	}

	return n;
}


/*
 * handle the long options
 * returs <0 on error.  
 */
static int
do_long_option ( struct global *data )
{	char *q,*arg ;
	int i;
	int nRet;

	assert( data );
	arg =  current_arg(data)+2;
	data->i++;
	
/* TODO: can modify argv?. Change  this */
	if( (q=strchr( arg , '='  )) )
	{	 q++;
		*(q-1)=0;
		if(!*q)
			q=NULL;
	}
/* TODO: end of change */
	i = findLongOption ( data->table , arg);
	
	if( i<0 )
		nRet =  RET_UNKNOW;
	else if( has_one_arg( data->table[i] ) )
	{	 if( q )
			data->arg = q;
		else if( !current_arg(data) || force_end( current_arg(data) ) ||
			 is_option( current_arg(data) ) )
			nRet = RET_ENOUGH;
		else
		{	data->arg = current_arg(data);
			data->i++;
		}
	}
	else if( data->table[i].flags!=0)
		assert( 0 );

	return i;
}

/*
 * gets the next option. returns the index on data->table of the option
 *
 * Modifies data->pt and data->i. puts in data->option the new option
 */
static int
getNextOption(struct global *data)
{	int nRet = 1;

	assert( data );
	data->arg = NULL;

	if( data->pt )
		nRet = do_short_opt( data );
	else if( !current_arg(data) )
		nRet = RET_END;
	else if( force_end( current_arg(data) ))
	{	data->i++;
		nRet = RET_END;
	}
	else if( !is_option( current_arg(data) ))
	{	nRet = RET_END;
	}
	else if( is_long_opt( current_arg(data)  ))
		nRet = do_long_option(data);
	else
	{	data->pt = current_arg(data) + 1;
		nRet = do_short_opt( data );
	}

	return nRet;
}

/*
 * process the option at 'argv[i]'
 * '*table' is the element to treat
 * returns the aditional arguments used
 */
static int
process( optionT *table, const char *arg )
{	int nRet;
	static const char * fmt[]=
	{	NULL,"%d","%ld","%f"  };
	typedef void (*callbackT)(void);

	assert( table );
	if( !table->data )
		return -1;

	switch( table->type )
	{	case OPT_T_FLAG:
			*((int *)table->data) = 1;
			break;
		case OPT_T_INT:
		case OPT_T_LONG:
		case OPT_T_FLOAT:
			/*
			 * FIX: "123 1323" token is not valid 
			 */
			nRet = sscanf(arg,fmt[table->type],table->data);
			if( nRet <= 0 )
				return -1;
			break;
		case OPT_T_FUNCT:
			(*(callbackT)table->data)();
		case OPT_T_GENER:
			table->data = (char *)arg;
			break;
		default:
			assert( 0 );
	}
	
	return 0;	
}


int
GetOptions( char *const *argv, optionT *table, unsigned flags, void *reserved )
{	struct global data;
	char single[2];
	int i;

	if( do_sanety(&data,argv,table,flags,reserved) <0 )
		return -1;

	while( (i=getNextOption(&data)) != RET_END )
	{	if( i ==  RET_UNKNOW )
			puts("unknow option!\n");
		else if ( i == RET_ENOUGH )
			puts("NOT ENOUGH PARAMS");
		else
		{	printf("# procesando = %d\n",i);
			if( process(table+i,data.arg) <0 )
				puts("FORMAT ERROR");
		}
	}

	return data.i;
}

