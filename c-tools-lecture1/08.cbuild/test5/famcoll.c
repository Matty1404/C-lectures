/*
 * famcoll.c:
 *
 *	Consider a collection of family information about parents
 *	and their children.
 *
 *	Specifically, we want to associate a non-empty set of children S
 *	with a named parent P.  We will be told (parent P, child C) pairs,
 *	and want to gather together, for each parent, all their children
 *	together.
 *
 *	We store this as a parent -> set(child) collection;
 *      actually, a hash from a string (the parent name) to a set of
 *	other strings (the names of the children of that parent).
 *
 * (C) Duncan C. White, May 2017
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include <hash.h>
#include <set.h>

#include "famcoll.h"


struct famcoll_s	/* a family collection is simply a hash of sets */
			/* with some meta-data */
{
	int nfamilies;
	hash f;
};


static void printV( FILE *out, hashkey parent, hashvalue v )
{
	set s = (set)v;
	fprintf( out, "%s: ", parent );
	bool empty = setIsEmpty( s );
	if( empty )
	{
		fprintf( out, "empty" );
	} else
	{
		//fprintf( out, "children: " );
		setDump( out, s );
	}
	fprintf( out, "\n" );
}


static hashvalue copyV( hashvalue v )
{
	return (hashvalue) setCopy( (set)v );
}


static void freeV( hashvalue v )
{
	setFree( (set)v );
}


/*
 * famcoll f = famcollCreate();
 *	Create a family collection: a hash of sets.
 */
famcoll famcollCreate( void )
{
	famcoll new = (famcoll) malloc( sizeof(struct famcoll_s));
	assert( new != NULL );
	new->f = hashCreate( &printV, &freeV, &copyV );
	new->nfamilies = 0;
	return new;
}


/*
 * famcollFree( f );
 *	Free a famcoll
 */
void famcollFree( famcoll f )
{
	hashFree( f->f );
	free( (void *)f );
}


/*
 * famcollAddChild( f, parent, child );
 *	Add child to parent.
 */
void famcollAddChild( famcoll f, char *parent, char *child )
{
	set s = (set)hashFind( f->f, parent );
	if( s==NULL )	/* parent not present in f yet */
	{
		s  = setCreate( NULL );
		hashSet( f->f, parent, (hashvalue)s );
		f->nfamilies++;
	}
	/* add child to the set */
	setAdd( s, child );
}


/*
 * bool ischild = famcollIsChild( f, parent, child );
 *	is child currently a child of parent?
 */
bool famcollIsChild( famcoll f, char *parent, char *child )
{
	set s = (set)hashFind( f->f, parent );
	return s!=NULL && setIn( s, child );
}


/*
 * famcollDump( out, f );
 *	display family collection f to out.
 */
void famcollDump( FILE *out, famcoll f )
{
	hashDump( out, f->f );
	fprintf( out, "There are %d families\n", f->nfamilies );
}


/*
 * set s = famcollChildren( f, parent );
 *	Retrieve parent's set of children, nb: they are not cloned
 *	Precondition: parent exists in f
 */
set famcollChildren( famcoll f, char *parent )
{
	set s = (set)hashFind( f->f, parent );
	assert( s!=NULL );	/* enforce precondition */
	return s;
}


/*
 * int n = famcollNFamilies( f );
 *	how many families (parents with kids) does family collection f contain?
 */
int famcollNFamilies( famcoll f )
{
	return f->nfamilies;
}


/*
 * famcollForeach( f, cb, extra );
 *	foreach (P,set of kids) entry, call the given callback cb
 *	with parent P, the set of kids, and the given extra value.
 */
void famcollForeach( famcoll f, famcollforeachcb cb, void *extra )
{
	// the func ptr type cast is safe, the only difference is that
	// famcollforeachcb's 2nd arg is a set, whereas hashforeachcb's
	// 2nd argument is a void *.  I'm sure it'll be great:-)
	hashForeach( f->f, (hashforeachcb)cb, extra );
}
