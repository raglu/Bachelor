#! /bin/csh
#  Copyright (c) 1989 Lars Fredriksen, Bryan So, Barton Miller
#  All rights reserved
#   
#  This software is furnished under the condition that it may not
#  be provided or otherwise made available to, or used by, any
#  other person.  No title to or ownership of the software is
#  hereby transferred.
# 
#  Any use of this software must include the above copyright notice.

#  Fuzz tests.
#
#
# Testing utilites that require special file name extensions
#
# Example
#           run.cp t.c cc -g
#
# will use cp to copy the test cases to a file named t.c, then execute
# cc -g t.c
#
# a - (minus) can be given as the first argument to suppress appending
# the file name to the command
#

set NOTES=notes
set TIMEOUT=20
set N_TESTS=0
set N_CRASHES=0
set N_HANGS=0

set suppress=0
if ( $1 == - ) then
     set suppress=1
     shift
endif

set TARGET=$1
shift
if ( ! `executable $1` ) then
     echo $*\: Command not found.
     echo $*\: Command not found. >> $NOTES
     exit 1
endif

echo Testing $* ... >> $NOTES

#
#  Erase some old core images
#
if ( -f core ) /bin/rm core

while (1)
	@ N_TESTS ++
	
	set FUZZ="fuzz"
	set SEED=`openssl rand 4 | od -DAn`
	set LENGTH=`shuf -i 1000-100000 -n 1`

	if ( ( `openssl rand 1 | od -DAn` % 2 ) == 0) set FUZZ="$FUZZ -p"
	if ( ( `openssl rand 1 | od -DAn` % 2 ) == 0) set FUZZ="$FUZZ -0"
	set FUZZ="$FUZZ -s $SEED $LENGTH"

	eval "$FUZZ" > testfile
	printf "%-20s %-40s %-15s %-15s\n" "Test nr. $N_TESTS :" "$FUZZ" "Crashes: $N_CRASHES" "Hangs: $N_HANGS"

     /bin/cp testfile $TARGET
     if ( $suppress ) then
	  timeout $TIMEOUT $* >& /dev/null
       if ( $status == 124 ) then
          goto Timeout
       endif
     else
       timeout $TIMEOUT $* $TARGET >& /dev/null
       if ( $status == 124 ) then
          goto Timeout
       endif
     endif
     goto CheckCore

Timeout:
     @ N_HANGS ++
     echo timeout
     echo timeout with $FUZZ >> $NOTES

CheckCore:
     if ( -f core ) then
	  @ N_CRASHES ++
	  echo core dumped
	  echo core dumped with $FUZZ >> $NOTES
	  /bin/rm core
     endif
end
if ( -f $TARGET ) then
     echo rm $TARGET
     /bin/rm $TARGET
endif
