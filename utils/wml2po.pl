#! /usr/bin/perl -w

# WARNING:
#  DO NOT commit po-files convverted with this script yet.  There is
#  still some work to do !

# USAGE:
#  ./utils/wml2po.pl data/translations/XXXXXXX.cfg po/XX.po

# TODO:
# - how to permanently get rid of this supurious c-format keyword ?

use strict;

## process wml files

require "utils/wmltrans.pm";

# get some id->english from english.cfg
our ($wmltransfile, $pofile) = @ARGV;
our %english = readwml ('data/translations/english.cfg');
our %revenglish;

sub set($$) {
  my ($id, $str) = @_;
  if (defined $english{$id}) {
    print STDERR "WARNING: duplicate def for $id: $str\n";
    return;
  }
  $english{$id}=$str;
#  print STDERR "$id=$str\n";
}

# get ids from unit files
our @wmlfiles = glob ("data/units/*.cfg");
foreach my $wmlfile (@wmlfiles) {
  open (WML, $wmlfile) or die "cannot open $wmlfile";
  my ($id, $str);
  while (<WML>) {
    if (m/id\s*=\s*(.*)/) {
      $id = $str = $1;
      $id =~ s/\s+//g;
      set($id,$str);
#     } elsif (m,\[/.*\],) {
#       $id = undef;
    } elsif (m/unit_description\s*=\s*(?:_\s*)\"(.*)\"\s*$/) {
      # single-line
      if (defined $id) {
	set($id . '_description',$1);
      } else {
	print STDERR "No id for unit_description $1\n";
      }
    }
  }
  close WML;
}

our %suffix = (
	       'difficulty_descriptions' => '_difficulties',
	       'cannot_use_message' => '_cannot_use_message',
	       'objectives' => '_objectives',
	      );

# get ids from other wml files
@wmlfiles = (qw(data/game.cfg data/help.cfg data/items.cfg data/multiplayer.cfg
		data/schedules.cfg data/tutorial.cfg data/tutorial2.cfg),
	     glob ("data/scenarios/*/*.cfg"));
foreach my $wmlfile (@wmlfiles) {
  open (WML, $wmlfile) or die "cannot open $wmlfile";
  print STDERR " Processing $wmlfile\n";
  my $id = 'no_id';
  my ($str, $key, $value,$strtype);
  while (<WML>) {
#    print STDERR $_;
    if (m/id\s*=\s*(.*)/) {
      $id = $1;
#      print STDERR "--> $id\n";
#     } elsif (m,\[/.*\],) {
#       $id = undef;
    } elsif (m/(difficulty\_descriptions|cannot\_use\_message|objectives)\s*=\s*(?:_\s*)?\"(.*)\"\s*$/) {
      # single-line
      die "nested key" if defined $key;

      if (defined $id) {
	set($id . $suffix{$1},$2);
      } else {
	print STDERR "No id for $1 $2\n";
      }
    } elsif (m/(difficulty\_descriptions|cannot\_use\_message|objectives)\s*=\s*(?:_\s*)?\"(.*)\s*$/) {
      # start of multi-line
      die "nested key" if defined $key;

      $strtype=1;
      $key = $id . $suffix{$1};
      $value = $2 . "\n";
    } elsif (m/(?:title|message|name|story)\s*=\s*(?:_\s*)\"(.*)\"\s*$/) {
      # single-line
      set($id,$1);
    } elsif (m/(title|message|name|story)\s*=\s*(?:_\s*)\"(.*)\s*$/) {
      # start of multi-line
      die "nested key" if defined $key;

      $strtype = 2;
      $key = $1;
      $value = $2 . "\n";
    } elsif (m/(\S+)\s*=\s*(?:_\s*)?\"(.*)\"\s*$/) {
      # ignored single-line
    } elsif (m/(\S+)\s*=\s*(?:_\s*)?\s*\"(.*)/) {
      # start of ignored multi-line
      $strtype = 0;
      $value = $2 . "\n";
    } elsif (m/(.*)\"\s*$/ and $strtype == 1) {
      # end of multi-line
      die "end of string without a key" unless defined $key;

      $value .= $1;
      set($key,$value);
      $key = undef;
    } elsif (m/(.*)\"\s*$/ and $strtype == 2) {
      # end of multi-line
      die "end of string without a key" unless defined $key;

      $value .= $1;
      set($id,$value);
      $key = undef;
    } elsif (m/(.*)\"\s*$/ and $strtype == 0) {
      # end of ignored multi-line
    } elsif (defined $key) {
      # part of multi-line
      $value .= $_;
    }
  }
  close WML;
}

#exit; # this is for debug only :)

# reverse the hash to use the strings to find the id
foreach my $key (keys %english) {
  $revenglish{$english{$key}} = $key;
}

# get translations from wml file
our %lang = readwml ($wmltransfile);

## process pofile, filling holes when we can

our $migrated = 0;

open (POFILE, $pofile) or die "cannot open $pofile";

my ($curid, $curmsg);
while (<POFILE>) {
  if (m/^msgid (".*")$/) {
    $curid = "$1\n"; $curmsg = undef;
  } elsif (m/^msgstr (".*")$/) {
    $curmsg = "$1\n";
  } elsif (m/^(".*")$/) {
    if (!defined $curmsg) {
      $curid .= "$_";
    } else {
      $curmsg .= "$_";
    }
  } elsif (m/^$/) {
    if (defined $curid and defined $curmsg) {
      processentry ($curid, $curmsg);
      print "\n";
    } else {
      print;
    }
  } else {
    print;
  }
}

# last entry does not have an empty line afterwards
if (defined $curid and defined $curmsg) {
  processentry ($curid, $curmsg);
}

## keep a list of previously translated entries not migrated
my $n = keys %lang;
if ($n == 0) {
  print STDERR "\nFINAL WORD: ALL $migrated TRANSLATIONS MIGRATED !\n";
} else {
  my $total = $migrated+$n;
  print STDERR "\nFINAL WARNING: the following $n/$total translations were NOT migrated:\n";
  foreach my $key (keys %lang) {
    print STDERR $key, ' = "', $lang{$key}, "\"\n";
  }
}

## Below is utility functions only - end of processing

sub processentry {
  my ($curid, $curmsg) = @_;
  my $output;
  my $touched = 0;

  # lookup

  if ($curmsg eq "\"\"\n") {
    my $id = $revenglish{po2rawstring($curid)};
    if (defined $id) {
      my $trans = $lang{$id};
      if (defined $trans) {
	$output = raw2postring($trans);
	$touched = 1;
	delete $lang{$id};
	$migrated++;
      } else {
	# print STDERR "WARNING: no translation found for $id - setting to empty\n";
	$output = raw2postring("");
      }
    } else {
      # this usually denotes limitations in this script
      print STDERR "WARNING: no id found (setting translation to empty) for $curid\n";
      $output = raw2postring("");
    }
  } else {
    $output = $curmsg;
  }

  print "#, fuzzy\n" if $touched;
  print "msgid $curid";
  print "msgstr $output";

  $curid = undef; $curmsg = undef;
}
