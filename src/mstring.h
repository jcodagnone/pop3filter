#ifndef _MY_STRING_H_
#define _MY_STRING_H_

typedef struct stringCDT *  string_t;

string_t string_new( void );

void string_destroy( string_t s );

int
string_cat( string_t s, const char *q);

int
string_ncat( string_t s, const char *q, size_t n);


const char *
string_get_as_ansi( string_t s );

int
string_reset( string_t s );

#endif
