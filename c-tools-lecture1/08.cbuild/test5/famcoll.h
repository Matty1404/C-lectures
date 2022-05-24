/*
 * famcoll.h:
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


/* a family collection is a pointer to an opaque type: struct famcoll_s
 * is defined only in famcoll.c
 */
typedef struct famcoll_s *famcoll;

/* a famcoll foreach callback is called by famcollForeach() once per family */
typedef void (*famcollforeachcb)( char *parent, set kids, void *extra );

extern famcoll famcollCreate( void );
extern void famcollFree( famcoll f );
extern void famcollAddChild( famcoll f, char * parent, char * child );
extern bool famcollIsChild( famcoll f, char * parent, char * child );
extern void famcollDump( FILE * out, famcoll f );
extern set famcollChildren( famcoll f, char * parent );
extern int famcollNFamilies( famcoll f );
extern void famcollForeach( famcoll f, famcollforeachcb cb, void * extra );
