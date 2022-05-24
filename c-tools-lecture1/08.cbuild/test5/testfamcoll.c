/*
 * testfamcoll.c: test program for the famcoll module.
 *
 * (C) Duncan C. White, May 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <set.h>
#include <hash.h>
#include <testutils.h>

#include "famcoll.h"


//#define TESTSET


static famcoll f;


/*
 * testcontains( f, parent, csvchildren );
 *	test that famcoll f contains an entry for parent, containing
 *	precisely the comma-separated csvchildren
 *	(and no other children).  assumes that child names contain
 *	no commas themselves:-)
 */
void testcontains( famcoll f, char *parent, char *csvchildren )
{
	set s = famcollChildren( f, parent );

	// we need to modify the string, so make a mutable copy..
	csvchildren = strdup( csvchildren );

	char msg[1024];
	sprintf( msg, "f(%s) contains %s", parent, csvchildren );

	// foreach csv value in csvchildren..
	int nfound = 0;
	int nincsv = 0;
	char *element = csvchildren;
	for(;;)
	{
		char *comma=strchr( element, ',' );
		if( comma != NULL )
		{
			// found the first comma after element
			*comma = '\0';
		}
		//printf( "debug: csvForeach: found value %s\n", element );
		nincsv++;

		// check whether element is in the set (as it should be!)
		if( setIn( s, element ) )
		{
			// good: expected element to be in, and it is.
			printf( "T %s: %s is in set: OK\n", msg, element );
			nfound++;
		} else
		{
			printf( "T %s: %s should be in set but isn't: FAIL\n",
				msg, element );
		}

	if( comma == NULL ) break;

		// move start to one beyond where comma was..
		element = comma+1;
	}
	// don't forget to free the copy..
	free( csvchildren );

	sprintf( msg, "f(%s) has %d children(s)", parent, nincsv );
	testint( nfound, nincsv, msg );
}

int main( int argc, char **argv )
{
	if( argc > 1 )
	{
		malloc(strlen(argv[1]));	/* test memory leaks */
	}

	#ifdef TESTSET
	set s = setCreate( NULL );
	setAdd( s, "one" );
	printf( "set s = " );
	setDump( stdout, s );
	printf( "\n" );
	testcond( setIn( s, "one" ), "one in s" );
	testint( setNMembers(s), 1, "s has 1 entry" );
	#endif

	f = famcollCreate();
	famcollAddChild( f, "one", "a" );
	printf( "added <a> to <one> in f\n" );
	testint( famcollNFamilies(f), 1, "f has 1 family" );

	set s2 = famcollChildren( f, "one" );
	testint( setNMembers(s2), 1, "f[one] has 1 child" );

	testcond( setIn(s2, "a" ), "a in f[one]" );
	testcond( !setIn(s2, "b" ), "b in f[one]" );

	printf( "initial families:\n" );
	famcollDump( stdout, f );

	testcontains( f, "one", "a" );	// test that "one" contains only "a"

	famcollAddChild( f, "one", "b" );
	printf( "added <b> to <one> in f\n" );
	testcontains( f, "one", "a,b" );	// should contain a and b

	famcollAddChild( f, "one", "c" );
	printf( "added <c> to <one> in f\n" );
	testcontains( f, "one", "a,b,c" );	// should contain a,b and c

	famcollAddChild( f, "two", "a" );
	printf( "added <a> to <two> in f\n" );
	testcontains( f, "one", "a,b,c" );
	testcontains( f, "two", "a" );

	famcollAddChild( f, "three", "z" );
	printf( "added <z> to <three> in f\n" );
	testcontains( f, "one", "a,b,c" );
	testcontains( f, "two", "a" );
	testcontains( f, "three", "z" );

	famcollAddChild( f, "two", "d" );
	printf( "added <d> to <two> in f\n" );
	testcontains( f, "one", "a,b,c" );
	testcontains( f, "two", "d,a" );	// order irrelevent

	printf( "final families:\n" );
	famcollDump( stdout, f );

	famcollFree( f );

	return 0;
}
