/*
 * set.h: set (based on hash) storage for C..
 *
 * (C) Duncan C. White, 1996-2007 although it seems longer:-)
 */

/*
 * a set is a hash table of key trees..
 */

typedef struct set_s *set;
typedef char *setkey;

typedef void (*setprintfunc)( FILE *, setkey );
typedef void (*setforeachcb)( setkey, void * );

extern set setCreate( setprintfunc p );
extern void setEmpty( set s );
extern set setCopy( set s );
extern void setFree( set s );
extern void setMetrics( set s, int * min, int * max, double * avg );
extern void setAdd( set s, setkey k );
extern void setRemove( set s, setkey k );
extern void setModify( set s, setkey changes );
extern bool setIn( set s, setkey k );
extern void setForeach( set s, setforeachcb cb, void * arg );
extern void setUnion( set a, set b );
extern void setIntersection( set a, set b );
extern void setDiff( set a, set b );
extern int setNMembers( set s );
extern bool setIsEmpty( set s );
extern void setSubtraction( set a, set b );
extern void setDump( FILE * out, set s );
