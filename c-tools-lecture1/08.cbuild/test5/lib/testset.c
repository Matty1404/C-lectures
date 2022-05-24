/*
 * testset.c: test program for the hash-based set module.
 *
 * (C) Duncan C. White, 1996-2017 although it seems longer:-)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "set.h"



static set s;


void test( setkey k, bool expected )
{
	char tstname[1024];
	sprintf( tstname, "test(%s)", k );
	if( setIn( s, k ) )
	{
		if( ! expected )
		{
			printf( "T %s: '%s' in when shouldn't be: FAIL\n",
				tstname, k );
		} else
		{
			printf( "T %s: '%s' in: OK\n", tstname, k );
		}
	} else
	{
		if( expected )
		{
			printf( "T %s: '%s' not in when it should be\n",
				tstname, k );
		} else
		{
			printf( "T %s: '%s' not in: OK\n", tstname, k );
		}
	}
}


void lookuptest( int x1, int x2, int x3, int x4, int xaard, int xgrumble )
{
	printf( "lookup tests:\n" );
	test( "eeny", x1 );
	test( "meeny", x2 );
	test( "miny", x3 );
	test( "mo", x4 );
	test( "aardvark", xaard );
	test( "grumble", xgrumble );
}


static void myPrint( FILE *out, setkey k )
{
	fprintf( out, "%s,", k );
}


/* 
 * each_cb: print out an item to the FILE *
 *	    hidden in the arg; used in the main
 *	    program to print the hash ourselves.
 */

static void each_cb( setkey k, void *arg )
{
	FILE *out = (FILE *)arg;
	myPrint( out, k );
}


int main( int argc, char **argv )
{
	s = setCreate( myPrint );

	if( argc > 1 )
	{
		malloc(strlen(argv[1]));
	}

	setAdd( s, "eeny" );
	setAdd( s, "meeny" );
	setAdd( s, "miny" );
	setAdd( s, "mo" );

	printf( "here's the set:\n" );
	setDump( stdout, s );

	lookuptest( 1, 1, 1, 1, 0, 0 );

	printf( "modify set a bit:\n" );
	setRemove( s, "meeny" );
	setAdd( s, "one" );
	setAdd( s, "aardvark" );

	printf( "now here's the set:\n" );
	setDump( stdout, s );

	lookuptest( 1, 0, 1, 1, 1, 0 );

	printf( "\nprint the set out using an explicit foreach loop:\n{" );
	setForeach( s, &each_cb, stdout );
	printf( "}\n" );

	printf( "\nfree the set\n" );
	setFree( s );

	return 0;
}
