#ifndef _MY_STRING_H_
#define _MY_STRING_H_

typedef struct stringCDT *  string_t;

string_t
NewString( void );

int
StringCat( string_t s, char *q);

int
StringNCat( string_t s, char *q, size_t n);

void
FreeString( string_t s );

const char *
GetAnsiString( string_t s );

int
StringClean( string_t s );

#endif
