/*
 *   transform: read parent: child lines from input, build a hash-of-sets
 *   	        and then print out collected families: each parent and all
 *		their children (currently not in sorted order).
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include <set.h>
#include <hash.h>

#include "readline.h"
#include "famcoll.h"



static famcoll f;

//#define DEBUG

#define STRSIZE 1024

int main( int argc, char **argv )
{
	char line[STRSIZE];

	f = famcollCreate();

	while( readline(stdin, line, STRSIZE ) != 0 )
	{
		//printf( "// debug: read line '%s'\n", line );
		char *parent = strtok( line, ": " );
		assert( parent != NULL );
		char *child = strtok( NULL, ": " );
		assert( child != NULL );
		//printf( "// debug parent='%s', child='%s'\n", parent, child );
		famcollAddChild( f, parent, child );
		#ifdef DEBUG
		printf( "debug: added <%s> to <%s>\n", child, parent );
		#endif
	}

	famcollDump( stdout, f );

	famcollFree( f );

	return 0;
}
