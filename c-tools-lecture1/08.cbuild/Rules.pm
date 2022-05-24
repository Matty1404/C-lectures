package Rules;

#
# analyse all C source files in a single directory, just like mfbuild did,
# looking for normal modular C, building internal make-style rules showing
# how to compile each .c file, and how to link each build target, and return
# those rules.
# 
# Written by Duncan C. White; April 2020, obviously trying to
# find things to do during the Coronavirus lockdown:-)
#

use strict;
use warnings;
use feature 'say';
use feature qw(signatures);
no warnings qw(experimental::signatures);
use Data::Dumper;

require Exporter;
our @ISA = qw(Exporter);
our @EXPORT = qw(
	read_all_defs
	expand_macros
	find_all_main_programs
	find_all_includes
	make_all_rules
	);


my $debug = 0;

my $home = $ENV{HOME};

my @locations = qw(/etc/cbuilddefs $home/include/cbuilddefs
		   $HOME/c-tools/lib/cbuilddefs ../.build ../.build);


my %defs;	# All definitions read from all files in @locations above
		# (and .build in the current directory as special case, affecting
		# some non-inheritable definitions


# The following definitions are only believed in a leaf (bottom-level) directory
my %leaf_only = map { $_ => 1 } qw(SUBDIR SUBLIB SUBINC EXTRA_CFLAGS EXTRA_LDLIBS);

#
# read_defns( $def, $leaf );
#	Read all definitions from $def, which is a "leaf" defns
#	file only if $leaf is true), into the global %defs
#

sub read_defns( $def, $leaf )
{
	open( my $in, '<', $def ) || return;
	while( <$in> )
	{
		chomp;
		next if /^#/ || /^\s*$/;
		die "read_all_defs: badly formed line '$_' in $def, '=' expected\n" unless /=/;
		my( $var, $value ) = split( /\s*=\s*/, $_, 2 );

		# skip leaf-only variables in non-leaf nodes
		next if !$leaf && $leaf_only{$var};

		$value = expand_macros( $value );
		$defs{$var} = $value;
		say "defn: $var = $value" if $debug;
	}
	close( $in );
}


#
# my %defs = read_all_defs();
#	Load definitions from any library/per-user/per-project/this-dir files.
#	Return a hash of those definitions.
#
sub read_all_defs(  )
{
	%defs = ();
	foreach my $def (@locations)
	{
		read_defns( $def, 0 );
	}
	# read .build in the current directory as a leaf
	read_defns( '.build', 1 );

	$defs{CC} //= 'gcc';
	$defs{CFLAGS} //= '-Wall';

	$defs{CFLAGS} .= " ".$defs{EXTRA_CFLAGS} if defined $defs{EXTRA_CFLAGS};
	$defs{LDLIBS} .= " ".$defs{EXTRA_LDLIBS} if defined $defs{EXTRA_LDLIBS};

	#die Dumper(\%defs);

	return %defs;
}


#
# my $str = expand($varname);
#	Expand $varname via %defs and %ENV.
#
sub expand( $varname )
{
	return $defs{$varname} // $ENV{$varname} // '';
}



#
# my $str = expand_macros( $instr );
#	Expand any $(xxx) macros in $instr to their values (in the global %defs).
#
sub expand_macros( $instr )
{
	my $str = $instr;
	#say "debug: in:$str";
	$str =~ s/\$\((\w+)\)/&expand($1)/eg;
	$str =~ s/^\s+//;
	$str =~ s/\s+$//;
	$str =~ s/\s+/ /g;
	#say "debug: out:$str";
	return $str;
}


#
# my $str = compilecmd( $cfile );
#	Generate a Unix action (command) for a single .c -> .o compilation,
#	return it as a string.
#
sub compilecmd( $cfile )
{
	return expand_macros( "\$(CC) \$(CFLAGS) -c $cfile" );
}


#
# my $action = linkcmd($build, @objs);
#	Generate a link command for executable $build, from objects @objs.
#
sub linkcmd( $build, @objs )
{
	my $ostr = join(' ',@objs);
	return expand_macros( "\$(CC) \$(CFLAGS) $ostr \$(LDFLAGS) -o $build \$(LDLIBS)" );
}

#
# my $action = makelibcmd($lib, @objs);
#	Generate a "make library" command to build $lib from @objs.
#
sub makelibcmd( $lib, @objs )
{
	my $ostr = join(' ',@objs);
	return expand_macros( "ar rcs $lib $ostr" );
}


#my $str = compilecmd( "wibble.c" );
#die $str;


my %cinc;	# basename.c -> list of included .h file basenames
my %h2c;	# does basename.h have a corresponding .c file? 1 for yes, 0 for no.


#
# find_all_includes();
#	find all includes in all .c files in the current directory,
#	initialising %cinc (c basename -> list of included .h basenames)
#	and %h2c (basename.h -> 0/1: is there a corresponding basename.c?
#
sub find_all_includes()
{
	foreach my $filename (glob("*.c"))
	{
		my @includes;
		open( my $infile, '<', $filename ) || die "cb: can't open $filename\n";
		while( <$infile> )
		{
			chomp;

			if( /^\s*#include\s+"(.+).h"/ )
			{
				# append .h basename to include list
				push @includes, $1;
				#say "debug: $filename, found include $1";
			}
		}
		close( $infile );

		my $basename = $filename;
		$basename =~ s/\.c$//;
		$cinc{$basename} = [ @includes ];
		$h2c{$_} = -f "$_.c" ? 1 : 0     for @includes;
	}
	#die Dumper \%cinc;
	#die Dumper \%h2c;
}


#
# my @b = find_all_main_programs();
#	Look for main() functions in *.c, return a list of basenames.
#
sub find_all_main_programs(  )
{
	my @cfiles = glob( "*.c" );
	return () unless @cfiles;
	my @b;
	foreach my $filename (@cfiles)
	{
		open( my $infile, '<', $filename ) ||
			die "cb: can't open $filename\n";
		my $basename = $filename;
		$basename =~ s/\.c$//;
		while( <$infile> )
		{
			chomp;

			if( /^\s*(void|int)\s+main\s*\(/ )
			{
				push @b, $basename;
				#say "debug: $filename, found main in $basename";
			}
		}
		close( $infile );
	}
	return @b;
}


#
# follow_build( $build, \%rules, \@extracompdeps, \@extralinkdeps );
#	Given $build, the basename of one C executable, transitively follow
#	all the #include X.h -> analyse X.c dependencies (stored in global
#	vars %cinc and %h2c), and build rules in %rules.  Include extra link deps
#	in all link dependencies, and extra compilaton deps in all compilation
#	dependencies.
#
sub follow_build( $build, $rules, $extracompdeps, $extralinkdeps )
{
	my @todo = ( $build );		# what we're looking for..
	my %seen = ( $build => 1 );	# what we've seen this time..

	my @objs;		# object files done, all part of executable

	while( @todo )		# more to do
	{
		my $basename = shift @todo;

		# allow building of a library: build target of the form libintlist.a
		# which triggers reading the LIBOBJS definition for a list of object
		# files that the library depends on
		if( $basename =~ /^lib.*\.a$/ )
		{
			my $lib = $basename;
			my $objs = $defs{LIBOBJS};
			die "cb: no LIBOBJS definition for library\n"
				unless $objs;
			say "debug: lib=$lib, objs=$objs" if $debug;
			my @objs = split( /\s+/, $objs );
			my $action = makelibcmd($lib, @objs);
			my $rule = [ $lib, \@objs, $action ];
			say "debug: lib rule = ", Dumper($rule) if $debug;

			$rules->{$lib} = $rule;
			next;
		}

		my $filename = "$basename.c";

		die "cb: target $basename, but $filename does not exist!\n"
			unless -f $filename;

		my $objfilename = "$basename.o";
		say "debug: analysing $filename" if $debug;

		my @includes = @{$cinc{$basename}};

		# Make the compilation rule, if not already made..
		unless( defined $rules->{$objfilename} )
		{
			my @hfile = map { "$_.h" } @includes;
			my @srcs = ( $filename );
			push @srcs, @hfile;
			push @srcs, @$extracompdeps;
			my $action = compilecmd($filename);
			#say "debug: target=$objfilename, srcs=", join(',',@srcs), ", action=$action" if $debug;

			my $rule = [ $objfilename, \@srcs, $action ];
			#say "debug: compile rule = ", Dumper($rule) if $debug;

			$rules->{$objfilename} = $rule;
		}

		# find unseen includes, add them to todo if the .c files exist
		my @new = grep { $h2c{$_} && ! $seen{$_}++ } @includes;
		#say "debug: adding $_ to todo" for @new if $debug;
		push @todo, @new;

		# append object filename to @objs, for linking in
		push @objs, $objfilename;
	}

	# Make the link rule (if not already made): link all @objs
	unless( defined $rules->{$build} )
	{
		my $action = linkcmd($build, @objs);
		say "debug: target=$build, srcs=", join(',',@objs), ", action=$action" if $debug;

		push @objs, @$extralinkdeps;
		my $rule = [ $build, \@objs, $action ];
		#say "debug: link rule = ", Dumper($rule) if $debug;

		$rules->{$build} = $rule;
	}
}


#
# my %rules = make_all_rules( $extracompdeps, $extralinkdeps, @build );
#	Make all rules for the buildables @build, adding any @$extracompdeps
#	to the compilation rule dependencies, and any @$extralinkdeps to the
#	link rule dependencies.
#	Return a hash of those rules, with the target as the key.
#
sub make_all_rules( $extracompdeps, $extralinkdeps, @build )
{
	my %rules;
	foreach my $build (@build)
	{
		follow_build( $build, \%rules, $extracompdeps, $extralinkdeps );
	}
	return %rules;
}


1;
