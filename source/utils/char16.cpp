/*  string.c -- standard C string-manipulation functions.

Copyright (C) 2008              Segher Boessenkool <segher@kernel.crashing.org>
Copyright (C) 2009              Haxx Enterprises <bushing@gmail.com>

Portions taken from the Public Domain C Library (PDCLib).
https://negix.net/trac/pdclib

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "char16.h"
//#include "gecko.h"

size_t strlen16( const char16 *s )
{
	if( !s )
	{
		return 0;
	}
	size_t len;

	for (len = 0; s[len]; len++)
		;

	return len;
}

size_t strnlen16( const char16 *s, size_t count )
{
	if( !s )
	{
		return 0;
	}
	size_t len;

	for (len = 0; s[len] && len < count; len++)
		;

	return len;
}

int strcmp16( const char16 *s1, const char16 *s2 )
{
	size_t i;

	for (i = 0; s1[i] && s1[i] == s2[i]; i++)
		;

	return s1[i] - s2[i];
}

int strncmp16 (const char16 *s1, const char16 *s2, size_t n )
{
	size_t i;

	for (i = 0; i < n && s1[i] && s1[i] == s2[i]; i++)
		;
	if (i == n) return 0;
	return s1[i] - s2[i];
}

char16* strcpy16( char16 *__restrict__ str, const char16 *__restrict__ strToCopy )
{
	if( !strToCopy )
	{
		return str;
	}
	char16* outPtr = str;
	do
	{
		*outPtr++ = *strToCopy;
	}
	while( *strToCopy++ );
	return str;
}

char16* strcpy16( char16 *str, const char *strToCopy )
{
	if( !strToCopy )
	{
		return str;
	}
	char16* outPtr = str;
	do
	{
		*outPtr++ = *strToCopy;
	}
	while( *strToCopy++ );
	return str;
}

size_t strlcpy16( char16 *dest, const char *src, size_t maxlen )
{
	if( !src )
	{
		return 0;
	}
	size_t len,needed;

	len = needed = strnlen(src, maxlen-1) + 1;
	if (len >= maxlen)
		len = maxlen-1;

	for( size_t i = 0; i < len; i++ )
	{
		dest[ i ] = src[ i ];
	}

	dest[ len ] = '\0';

	return needed - 1;
}

size_t strlcpy16( char16 *__restrict__ dest, const char16 *__restrict__ src, size_t maxlen )
{
	if( !src )
	{
		return 0;
	}
	size_t len,needed;

	len = needed = strnlen16( src, maxlen - 1 ) + 1;
	if (len >= maxlen)
		len = maxlen-1;

	for( size_t i = 0; i < len; i++ )
	{
		dest[ i ] = src[ i ];
	}

	dest[ len ] = '\0';

	return needed - 1;
}

size_t strlcat16( char16 *dest, const char *src, size_t maxlen )
{
	if( !src )
	{
		return 0;
	}
	size_t used;

	used = strnlen16( dest, maxlen - 1 );
	return used + strlcpy16( dest + used, src, maxlen - used );
}

char16 *strdup16( const char16 *str )
{
	if( !str )
	{
		return NULL;
	}
	int len = strlen16( str ) + 1;
	char16 *ret = (char16 *)malloc( len * sizeof( char16 ) );
	if( !ret )
	{
		return ret;
	}
	strcpy16( ret, str );
	return ret;
}

char16 *strdup16( const char *str )
{
	if( !str )
	{
		return NULL;
	}
	int len = strlen( str ) + 1;
	char16 *ret = (char16 *)malloc( len * sizeof( char16 ) );
	if( !ret )
	{
		return ret;
	}
	strcpy16( ret, str );
	return ret;
}

int sprintf16( char16 *str, const char* fmt, ... )
{
	char *charStr = NULL;

	va_list argptr;
	va_start( argptr,fmt );
	int ret = vasprintf( &charStr, fmt, argptr );
	va_end( argptr );

	// some error
	if( ret < 1 || !charStr )
	{
		// the manpage says the that the state of charStr is undefined on error. on BSD it is set to NULL
		// i guess we'll not free it then and let it leak memory if thats what it wants to do.
		// just dont create any errors here and theres nothing to worry about
		return ret;
	}
	strcpy16( str, charStr );
	free( charStr );
	return ret;
}

int snprintf16( char16 *str, int maxLen, const char* fmt, ... )
{
	char *charStr = NULL;

	va_list argptr;
	va_start( argptr,fmt );
	int ret = vasprintf( &charStr, fmt, argptr );
	va_end( argptr );

	// some error
	if( ret < 1 || !charStr )
	{
		// the manpage says the that the state of charStr is undefined on error. on BSD it is set to NULL
		// i guess we'll not free it then and let it leak memory if thats what it wants to do.
		// just dont create any errors here and theres nothing to worry about
		return ret;
	}
	strlcpy16( str, charStr, maxLen );
	free( charStr );
	return ret;
}
