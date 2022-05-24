/*
 * testhash.c: test program for the hash module.
 *
 *	       specifically, let's have a hash : string->string
 *
 * (C) Duncan C. White, 1996-2017 although it seems longer:-)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "testutils.h"
#include "hash.h"



void myPrint( FILE *out, hashkey k, hashvalue v )
{
	fprintf( out, "%s: %s\n", k, (char *)v );
}

static void myFree( hashvalue v )
{
	free( v );
}


static hashvalue myCopyValue( hashvalue v )
{
	return strdup(v);
}


static hash h1;


void test( hashkey k, char *expected )
{
	char tstname[1024];
	sprintf( tstname, "test(%s)", k );
	hashvalue got = hashFind( h1, k );
	if( got != NULL )
	{
		if( expected == NULL )
		{
			printf( "T %s: '%s' found when shouldn't be: FAIL\n",
				tstname, k );
		} else if( strcmp( got, expected ) == 0 )
		{
			printf( "T %s: '%s' found, correct value '%s': OK\n",
				tstname, k, (char *)got );
		} else
		{
			teststring( got, expected, tstname );
		}
		//myPrint( stdout, k, got );
	} else
	{
		if( expected == NULL )
		{
			printf( "T %s: '%s' not found: OK\n",
				tstname, k );
		} else
		{
			printf( "T %s: '%s' not found when should be: FAIL\n",
				tstname, k );
		}
	}
}


void lookuptest( char * x1, char * x2, char * x3, char * x4, char * xaard, char * xgrumble )
{
	printf( "lookup tests:\n" );
	test( "one", x1 );
	test( "two", x2 );
	test( "three", x3 );
	test( "four", x4 );
	test( "aardvark", xaard );
	test( "grumble", xgrumble );
}


void set( hash h, hashkey k, char *v )
{
	hashSet( h, k, myCopyValue(v) );
}


/* 
 * each_cb: print out a (k,v) pair to the FILE *
 *	    hidden in the arg; used in the main
 *	    program to print the hash ourselves.
 */

static void each_cb( hashkey k, hashvalue v, hashvalue arg )
{
	FILE *out = (FILE *)arg;

	myPrint( out, k, v );
}


int main( int argc, char **argv )
{
	h1 = hashCreate( myPrint, myFree, myCopyValue );

	if( argc > 1 )
	{
		malloc(strlen(argv[1]));
	}

	set( h1, "one", "eeny" );
	set( h1, "two", "meeny" );
	set( h1, "three", "miny" );
	set( h1, "four", "mo" );

	printf( "here's the hash:\n" );
	hashDump( stdout, h1 );

	lookuptest( "eeny", "meeny", "miny", "mo", NULL, NULL );

	printf( "modify hash a bit:\n" );
	set( h1, "one", "solitary posh git" );
	set( h1, "aardvark", "not a herring" );
	set( h1, "three", "quite a few" );

	printf( "now here's the hash:\n" );
	hashDump( stdout, h1 );

	lookuptest( "solitary posh git", "meeny", "quite a few", "mo",
		    "not a herring", NULL );

	printf( "\nprint the hash out using an explicit foreach loop:\n{\n" );
	hashForeach( h1, &each_cb, stdout );
	printf( "}\n" );

	printf( "\ncopy the hash\n" );
	hash h2 = hashCopy( h1 );

	printf( "modify both hashes a bit:\n" );
	set( h1, "one", "single solitaire" );
	set( h2, "one", "on me ownses, savvy?" );

	printf( "print h1:\n" );
	hashDump( stdout, h1 );

	printf( "print h2:\n" );
	hashDump( stdout, h2 );

	printf( "free the original hash\n" );
	hashFree( h1 );

	printf( "print the copy again:\n" );
	hashDump( stdout, h2 );

	printf( "free the copy\n" );
	hashFree( h2 );

	return 0;
}
