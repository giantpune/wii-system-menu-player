#ifndef CHAR16_H
#define CHAR16_H

#include <stdlib.h>
#include <gctypes.h>

// functions to mess with strings with be16 characters

typedef short char16;

// return the number of characters
size_t strlen16( const char16 *s );
size_t strnlen16( const char16 *s, size_t count );


int strcmp16( const char16 *s1, const char16 *s2 );
int strncmp16 (const char16 *s1, const char16 *s2, size_t n );

char16* strcpy16( char16 *__restrict__ str, const char16 *__restrict__ strToCopy );
char16* strcpy16( char16 *str, const char *strToCopy );

size_t strlcpy16( char16 *dest, const char *src, size_t maxlen );
size_t strlcpy16( char16 *__restrict__ dest, const char16 *__restrict__ src, size_t maxlen );
size_t strlcat16( char16 *dest, const char *src, size_t maxlen );


// use malloc to allocate and copy a char or char16 string
char16 *strdup16( const char16 *str );
char16 *strdup16( const char *str );

// uses sprintf internally to create a formatted char* string and then copies it to the char16* string specified
int sprintf16( char16 *str, const char* fmt, ... )  __attribute__(( format( printf, 2, 3 ) ));
int snprintf16( char16 *str, int maxLen, const char* fmt, ... )  __attribute__(( format( printf, 3, 4 ) ));

#endif // CHAR16_H
