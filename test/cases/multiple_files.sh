#!/bin/bash
./clean_sandbox.sh


mkdir sandbox/dummydir
cp data/unixref sandbox/dummydir/donttouch
cp data/unixref sandbox/multi1test
cp data/unixref sandbox/multi2test
cp data/unixref sandbox/multi3test
$ENDLINES win -q sandbox/dummydir nonexistent sandbox/multi1test sandbox/multi2test sandbox/multi3test 2>/dev/null

WINREF=`$MD5<data/winref`
MULTIONE=`$MD5<sandbox/multi1test`
MULTITWO=`$MD5<sandbox/multi2test`
MULTITHREE=`$MD5<sandbox/multi3test`
if [[ "$MULTIONE" == "$WINREF" &&  "$MULTITWO" == "$WINREF" && "$MULTITHREE" == "$WINREF" ]]
then
    echo "OK : multiple files directly on command line"
else
    echo "FAILURE : failure to handle multiple files directly on command line"
    ./case_failed.sh
fi



DONTTOUCH=`$MD5<sandbox/dummydir/donttouch`

if [[ "$DONTTOUCH" != "$WINREF" ]]
then
    echo "OK : did not go into subdirectories when -r wasn't given"
else
    echo "FAILURE : went into a subdirectory although -r wasn not given"
    ./case_failed.sh
fi

rm -r sandbox/dummydir




mkdir sandbox/subdir1
mkdir sandbox/subdir1/subdir11
mkdir sandbox/subdir1/subdir12
mkdir sandbox/subdir1/.hidden_subdir

cp data/unixref sandbox/subdir1/file1a
cp data/unixref sandbox/subdir1/file1b
cp data/unixref sandbox/subdir1/subdir11/file111a
cp data/unixref sandbox/subdir1/subdir11/file111b
cp data/unixref sandbox/subdir1/subdir12/file112a
cp data/unixref sandbox/subdir1/subdir12/file112b
cp data/unixref sandbox/subdir1/.hidden_subdir/file_inside_hidden
cp data/unixref sandbox/subdir1/.hidden_file


$ENDLINES win -r sandbox/subdir1 &>/dev/null

UNIXREF=`$MD5<data/unixref`
FILE1A=`$MD5<sandbox/subdir1/file1a`
FILE1B=`$MD5<sandbox/subdir1/file1b`
FILE111A=`$MD5<sandbox/subdir1/subdir11/file111a`
FILE111B=`$MD5<sandbox/subdir1/subdir11/file111b`
FILE112A=`$MD5<sandbox/subdir1/subdir12/file112a`
FILE112B=`$MD5<sandbox/subdir1/subdir12/file112b`


if [[ 
    "$WINREF" == "$FILE1A" &&
    "$WINREF" == "$FILE1B" &&
    "$WINREF" == "$FILE111A" &&
    "$WINREF" == "$FILE111B" &&
    "$WINREF" == "$FILE112A" &&
    "$WINREF" == "$FILE112B" 
]]
then 
    echo "OK : finding regular files in subdirectories with -r"
else
    echo "FAILURE : regular files in subdirectories were not found when using -r"
    ./case_failed.sh
fi

FILE_INSIDE_HIDDEN=`$MD5<sandbox/subdir1/.hidden_subdir/file_inside_hidden`
HIDDEN_FILE=`$MD5<sandbox/subdir1/.hidden_file`


if [[ 
    "$UNIXREF" == "$FILE_INSIDE_HIDDEN" &&
    "$UNIXREF" == "$HIDDEN_FILE"
]]
then
    echo "OK : hidden files and directories were not affected by -r"
else
    echo "FAILURE : hidden files and directories changed by -r even without -h"
    ./case_failed.sh
fi



$ENDLINES win -r -h sandbox/subdir1 &>/dev/null

FILE_INSIDE_HIDDEN=`$MD5<sandbox/subdir1/.hidden_subdir/file_inside_hidden`
HIDDEN_FILE=`$MD5<sandbox/subdir1/.hidden_file`

if [[ 
    "$WINREF" == "$FILE_INSIDE_HIDDEN" &&
    "$WINREF" == "$HIDDEN_FILE"
]]
then
    echo "OK : hidden files and directories processed with -r -h"
else
    echo "FAILURE : hidden files and directories not changed by -r even with -h"
    ./case_failed.sh
fi


rm -rf sandbox/subdir1
