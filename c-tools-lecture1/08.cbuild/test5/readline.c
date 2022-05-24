/*
 *   readline.c: utility function readline..
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "readline.h"


bool readline( FILE *in, char *buf, int maxlen )
{
	if( fgets( buf, maxlen, in ) == 0 )
	{
		return false;
	}
	int l = strlen(buf);
	if( l>0 && buf[l-1]=='\n' )
	{
		buf[l-1] = '\0';
	}
	return true;
}
