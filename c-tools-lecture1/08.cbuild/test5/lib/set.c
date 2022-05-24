/*
 * set.c: set (based on hashes) storage for C..
 *
 * (C) Duncan C. White, 1996-2017 although it seems longer:-)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "set.h"


#define	NHASH	32533


typedef struct tree_s *tree;


struct set_s {
	tree *		data;
	setprintfunc	p;
	int		nmembers;
};

struct tree_s {
	setkey		k;			/* key aka set member */
	bool		in;			/* in, i.e. not deleted */
	tree		left;			/* Left... */
	tree		right;			/* ... and Right ptr's */
};


/*
 * operation
 */
typedef enum { Search, Define, Exclude } ops;


/* Private functions */

static void adddelop( setkey k, void * v );
static void exclude_if_notin_cb( setkey k, void * arg );
static void diff_cb( setkey k, void * arg );
static void dump_foreachcb( setkey k, void * arg );
static tree talloc( setkey k );
static int shash( char * str );
static tree symop( set s, setkey k, ops op );
static void foreach_tree( tree t, setforeachcb f, void * arg );
static tree copy_tree( tree t );
static void free_tree( tree t );
static int depth_tree( tree t );


/*
 * Create an empty set
 */
set setCreate( setprintfunc p )
{
	set   s = (set) malloc( sizeof(struct set_s) );
	s->data = (tree *) malloc( NHASH*sizeof(tree) );
	s->p = p;
	s->nmembers = 0;

	int   i;
	for( i = 0; i < NHASH; i++ )
	{
		s->data[i] = NULL;
	}
	return s;
}


/*
 * Empty an existing set - ie. retain only the skeleton..
 */
void setEmpty( set s )
{
	int   i;

	for( i = 0; i < NHASH; i++ )
	{
		free_tree( s->data[i] );
		s->data[i] = NULL;
	}
	s->nmembers = 0;
}


/*
 * Copy an existing set.
 */
set setCopy( set s )
{
	int   i;
	set   result;

	result = (set) malloc( sizeof(struct set_s) );
	result->data = (tree *) malloc( NHASH*sizeof(tree) );
	result->p = s->p;
	result->nmembers = s->nmembers;

	for( i = 0; i < NHASH; i++ )
	{
		result->data[i] = copy_tree( s->data[i] );
	}

	return result;
}


/*
 * Free the given set - clean it up and delete it's skeleton too..
 */
void setFree( set s )
{
	int   i;

	for( i = 0; i < NHASH; i++ )
	{
		free_tree( s->data[i] );
	}
	free( (void *) s->data );
	free( (void *) s );
}


/*
 * Set metrics:
 *  calculate the min, max and average depth of all non-empty trees
 *  sadly can't do this with a setForeach unless the depth is magically
 *  passed into the callback..
 */
void setMetrics( set s, int *min, int *max, double *avg )
{
	int	i;
	int	nonempty = 0;
	int	total    = 0;

	*min =  100000000;
	*max = -100000000;
	for( i = 0; i < NHASH; i++ ) {
		if( s->data[i] != NULL )
		{
			int d = depth_tree( s->data[i] );
			if( d < *min ) *min = d;
			if( d > *max ) *max = d;
			total += d;
			nonempty++;
		}
	}
	*avg = ((double)total)/(double)nonempty;
}


/*
 * Add k to the set s
 */
void setAdd( set s, setkey k )
{
	(void) symop( s, k, Define);
}


/*
 * Remove k from the set s
 */
void setRemove( set s, setkey k )
{
	(void) symop( s, k, Exclude);
}


/*
 * Convenience function:
 *  Given a changes string of the form "[+-]item[+-]item[+-]item..."
 *  modify the given set s, including (+) or excluding (-) items
 * NB: This assumes that key == char *..
 */
void setModify( set s, setkey changes )
{
	char *str = strdup( changes );	/* so we can modify it! */
	char *p = str;

	char cmd = *p;
	while( cmd != '\0' )		/* while not finished */
	{
		assert( cmd == '+' || cmd == '-' );
		p++;

		/* got a string of the form... [+-]itemstring[+-\0]... */
		/* cmd = the + or - command                            */
		/* and p points at the first char  ^p                  */

		/* find the next +- command,                  ^q       */
		char *q = p;
		for( ; *q != '\0' && *q != '+' && *q != '-'; q++ );

		/* terminate itemstring here, remembering the next cmd */
		char nextcmd = *q;
		*q = '\0';

		/* now actually include/exclude the item from the set  */
		if( cmd == '+' )
		{
			setAdd( s, p );
		} else
		{
			setRemove( s, p );
		}

		/* set up for next time                                */
		cmd = nextcmd;			   /* the next command */
		p = q;				   /* the next item    */
	}
	free( (void *)str );
}


/*
 * Look for something in the set s
 */
bool setIn( set s, setkey k )
{
	tree x = symop(s, k, Search);

	return x != NULL && x->in;
}


/*
 * perform a foreach operation over a given set
 * call a given callback for each item pair.
 */
void setForeach( set s, setforeachcb cb, void * arg )
{
	int	i;

	for( i = 0; i < NHASH; i++ ) {
		foreach_tree( s->data[i], cb, arg );
	}
}


typedef enum { Uncond, IfIn, IfNotIn } cond;

typedef struct
{
	cond      c;
	set	  result;
	set	  other;
	int       add;
} setop;


static void adddelop( setkey k, void *v )
{
	setop *d = (setop *)v;
	if( d->c == Uncond
	|| (d->c == IfIn && setIn(d->other,k))
	|| (d->c == IfNotIn && ! setIn(d->other,k))
	) {
		if( d->add )
		{
			setAdd( d->result, k );
		} else
		{
			setRemove( d->result, k );
		}
	}
}


/*
 * Set union, a += b
 */
void setUnion( set a, set b )
{
	setop       data;
	data.result = a;
	data.c      = Uncond;
	data.add    = 1;

	setForeach( b, &adddelop, (void *)&data );
}


/*
 * Set intersection, a = a&b
 *   exclude each member of a FROM a UNLESS in b too
 *   here we need to pass both sets to the callback,
 *   via this "pair of sets" structure:
 */
typedef struct { set a, b; } setpair;
static void exclude_if_notin_cb( setkey k, void *arg )
{
	setpair *d = (setpair *)arg;
	if( ! setIn(d->b, k) )
	{
		setRemove( d->a, k );
	}
}
void setIntersection( set a, set b )
{
	setpair data; data.a = a; data.b = b;
	setForeach( a, &exclude_if_notin_cb, (void *)&data );
}



/*
 * Set difference, simultaneous a -= b and b -= a
 *  exclude each item of both sets from both sets, LEAVING
 *  - a containing elements ONLY in a, and
 *  - b containing elements ONLY in b.
 */
static void diff_cb( setkey k, void *arg )
{
	setpair *d = (setpair *)arg;
	if( setIn(d->a, k) )
	{
		setRemove( d->a, k );
		setRemove( d->b, k );
	}
}


void setDiff( set a, set b )
{
	setpair data; data.a = a; data.b = b;
	setForeach( b, &diff_cb, (void *)&data );
}


/*
 * SetNMembers: how many members in the set?
 */
//static void count_cb( setkey k, void *arg )
//{
//	int *n = (int *)arg;
//	(*n)++;
//}
int setNMembers( set s )
{
	//int n = 0;
	//setForeach( s, &count_cb, (void *)&n );
	//return n;
	return s->nmembers;
}

/*
 * Set is empty? Is the set empty?
 */
bool setIsEmpty( set s )
{
	//return setNMembers( s ) == 0;
	return s->nmembers == 0;
}

/*
 * Set subtraction, a -= b
 */
void setSubtraction( set a, set b )
{
	setop       data;
	data.result = a;
	data.other  = a;
	data.c      = IfIn;
	data.add    = 0;

	setForeach( b, &adddelop, (void *)&data );
}


/*
 * Display a given set - print each item
 * by calling the array's printfunc
 */
typedef struct {
	FILE *out;
	setprintfunc p;
} dumpdata;


void setDump( FILE *out, set s )
{
	dumpdata arg;

	arg.p = s->p;
	arg.out = out;

	//fputc('\n',out);
	setForeach( s, &dump_foreachcb, (void *)&arg );
	//fputc('\n',out);
}


/*
 * dump foreach callback:
 *	print a single item to (dumpdata *)arg
 */
static void dump_foreachcb( setkey k, void * arg )
{
	dumpdata *dd = (dumpdata *)arg;

	if( dd->p != NULL )
	{
		(*(dd->p))( dd->out, k );
	} else
	{
		//fprintf( dd->out, "%20s\n", k );
		fprintf( dd->out, "%s,", k );
	}
}


/*
 * Allocate a new node in the tree
 */
static tree talloc( setkey k )
{
	tree   p = (tree) malloc(sizeof(struct tree_s));

	if( p == NULL )
	{
		fprintf( stderr, "talloc: No space left\n" );
		exit(1);
	}
	p->left = p->right = NULL;
	p->k    = strdup(k);		/* Save setkey */
	p->in   = true;			/* Include it */
	return p;
}


/*
 * Calculate hash on a string
 */
static int shash( char *str )
{
	unsigned char	ch;
	unsigned int	hh;

	for (hh = 0; (ch = *str++) != '\0'; hh = hh * 65599 + ch )
	{
	}
	return hh % NHASH;
}


/*
 * Operate on the symbol table
 * Search, Define, Exclude.
 */
static tree symop( set s, setkey k, ops op )
{
	tree	ptr;
	tree *	aptr = s->data + shash(k);

	while( (ptr = *aptr) != NULL )
	{
		int rc = strcmp(ptr->k, k);
		if( rc == 0 )
		{
			if( op == Define )
			{
				if( ! ptr->in )
				{
					s->nmembers++;
					ptr->in = true;
				}
			} else if( op == Exclude )
			{
				if( ptr->in )
				{
					ptr->in = false;
					s->nmembers--;
				}
			} else if( ! ptr->in )
			{
				return NULL;
			}
			return ptr;
		}
		if (rc < 0)
		{
			/* less - left */
			aptr = &(ptr->left);
		} else
		{
			/* more - right */
			aptr = &(ptr->right);
		}
	}

	if (op == Define )
	{
		ptr = *aptr = talloc(k);	/* Alloc new node */
		s->nmembers++;
		return ptr;
	}

	return NULL;				/* not found */
}


/*
 * foreach one tree
 */
static void foreach_tree( tree t, setforeachcb f, void * arg )
{
	assert( f != NULL );
	if( t )
	{
		foreach_tree( t->left, f, arg );
		if( t->in )
		{
			(*f)( t->k, arg );
		}
		foreach_tree( t->right, f, arg );
	}
}


/*
 * Copy one tree
 */
static tree copy_tree( tree t )
{
	tree result = NULL;
	if( t )
	{
		result = talloc( t->k );
		result->in    = t->in;
		result->left  = copy_tree( t->left );
		result->right = copy_tree( t->right );
	}
	return result;
}


/*
 * Free one tree
 */
static void free_tree( tree t )
{
	if( t )
	{
		free_tree( t->left );
		free_tree( t->right );
		free( (void *) t->k );
		free( (void *) t );
	}
}


/*
 * Compute the depth of a given tree
 */
#define max(a,b)  ((a)>(b)?(a):(b))
static int depth_tree( tree t )
{
	if( t )
	{
		int d2 = depth_tree( t->left );
		int d3 = depth_tree( t->right );
		return 1 + max(d2,d3);
	}
	return 0;
}


