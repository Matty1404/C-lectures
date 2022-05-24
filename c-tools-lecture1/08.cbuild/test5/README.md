This is a much bigger example [taken from a later C-Programming-Tools lecture]
done here with cb to show that it scales up ok.

The example implements a named collection of sets of strings: a hash-of-sets-of-strings,
and the lib directory contains implementatins of sets-of-strings and
hashes-from-strings-to-generic-pointers.


# Description of the example is below..

Suppose we are presented with an input file with any number of lines
of the form:

```
one: a               (meaning that "one is the parent of a")
two: c               (meaning that "two is the parent of c")
one: b               (meaning that "one is the parent of b")
one: c               (meaning that "one is the parent of c")
```

ie. a named parent (eg "one") may have any number of named children (eg "b"),
there may be any number of parents, any number of children per parent,
and the parent,child information may come in any order.

and our goal is to collect all the children belonging to each parent,
producing output showing, for each parent, the complete set of
their children (ideally sorted):

```
one: a,c,b          (one is the parent of a, c and b)
two: c              (two is the parent of c only)
```

Clearly, the core data type here is a hash from parent names (strings) to
sets of strings (the set of children).

We store this as a hash-of-sets-of-strings,
implemented by the hashofsets.[ch] module and unit-tested by "testhos.c".
The main program that transforms the input to the output is "transform.c".


# Building this...

1. build this project via:

```
cb
```

4. run it's tests:

```
cb test
```

5. run transform on the pc-input file and check for memory leaks:

```
valgrind ./transform < pc-input
```



6. Note that the summarisetests utility here is worth installing into your
TOOLDIR/bin - type cb install to do that.



# Doing transform as a Perl one-liner

Depressingly, after writing nearly 500 lines of C code (not including the
hash and set modules!), it turns out that we could write transform as
a Perl one-liner:

```
perl -MData::Dumper -lane '($p,$c) = split( /:\s*/ ); $h{$p}->{$c}++; END {print Dumper \%h }' < pc-input
```

which produces the output:

```
$VAR1 = {
          'one' => {
                     'c' => 1,
                     'a' => 1,
                     'b' => 1
                   },
          'two' => {
                     'a' => 1,
                     'd' => 1
                   },
          'three' => {
                       'z' => 1
                     }
        };
```

Data::Dumper is a Perl module that prints out Perl data structures in a
rather JSON like format, and the data structure %h is a hash of hashes,
and the inner hashes are "sethashes", i.e. a hash whose values are all 1,
representing a set - the keys of that hash are the members of the set.

so $h{P}->{C} is a boolean expression that represents "is C a child of parent P"

If we want to format the output in the actual desired form, that'd be:

```
perl -lane '($p,$c) = split( /:\s*/ ); $h{$p}->{$c}++; END {foreach $p (sort keys %h) { $c = $h{$p}; $k = join( ",", sort keys %$c ); print "$p: $k" } }' < pc-input
```

Perl is the most amazing text manipulation language I know.  As the Pragmatic
Programmers say:

	Tip 28: Learn a Text Manipulation Language

they recommend Perl, and they're absolutely right:-)
