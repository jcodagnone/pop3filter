#ifndef _MY_STRING_H_
#define _MY_STRING_H_

/** Abstract data type */
typedef struct stringCDT *  string_t;

/** creates a new string_t object */
string_t string_new( void );

/** destroys an existing string_t object */
void string_destroy( string_t s );

/** append a NIL terminated array to a  string_t */
int
string_append( string_t s, const char *q);

/** append n bytes from q to  string_t */
int
string_nappend( string_t s, const char *q, size_t n);

/** get the current string. you can't change the returned string, and
 *  it is only valid until the next call
 */
const char *
string_get_as_ansi( string_t s );

/** clears the string_t */
int
string_reset( string_t s );

#endif
