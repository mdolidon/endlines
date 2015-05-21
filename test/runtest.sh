#!/bin/bash

if [ -n "`command -v md5`" ]
then
    MD5=md5
elif [ -n "`command -v md5sum`" ]
then
    MD5=md5sum
else
    echo "No MD5 hash command found, can't run tests."
    exit
fi

echo
echo "hash command : $MD5"
echo

rm *test 2>/dev/null
FAILURES=""

chmod +rw *


#
# Part 1 : try various input/output combinations
#

echo "Round tripping in files mode unix -> oldmac -> win -> unix"

cp unixref oldmactest
../endlines oldmac oldmactest &>/dev/null

cp oldmactest wintest
../endlines win wintest &>/dev/null

cp wintest unixtest
../endlines unix unixtest &>/dev/null

UNIXREF=`$MD5<unixref`
WINREF=`$MD5<winref`
OLDMACREF=`$MD5<oldmacref`

UNIXTEST=`$MD5<unixtest`
WINTEST=`$MD5<wintest`
OLDMACTEST=`$MD5<oldmactest`


if [[ "$OLDMACREF" == "$OLDMACTEST" ]]
then
    echo "OK : oldmac output"
else
    echo "FAILURE : oldmac output doesn't match"
    FAILURES="yes"
fi

if [[ "$WINREF" == "$WINTEST" ]]
then
    echo "OK : windows output"
else
    echo "FAILURE : windows output doesn't match"
    FAILURES="yes"
fi

if [[ "$UNIXREF" == "$UNIXTEST" ]]
then
    echo "OK : unix output"
else
    echo "FAILURE : unix output doesn't match"
    FAILURES="yes"
fi



#
# Part 2 : large files, testing the buffers' integrity
#

echo "...building a large file..."
for ((i=1;i<=500;i++));
do
    cat unixref >> bigunixintest
done

echo "...processing the large file, three times..."
../endlines win <bigunixintest 2>/dev/null | ../endlines oldmac 2>/dev/null | ../endlines unix >bigunixouttest 2>/dev/null


BIGUNIXIN=`$MD5<bigunixintest`
BIGUNIXOUT=`$MD5<bigunixouttest`

if [[ "$BIGUNIXIN" == "$BIGUNIXOUT" ]]
then
    echo "OK : large file processing"
else
    echo "FAILURE : large file processing ; buffered access mechanism may be damaged"
    FAILURES="yes"
fi




#
# Part 3 : details, options...
#

echo "" | ../endlines dunnothis 2>unknowntest >/dev/null
UNKNOWN=`cat unknowntest`
if [[ $UNKNOWN == *"unknown line end"* ]]
then
      echo "OK : reacts to an unknown convention"
else
    echo "FAILURE : failed to react to an unknown convention"
    FAILURES="yes"
fi

echo "" | ../endlines unix -q 2>quiettest >/dev/null
QUIET=`cat quiettest`
if [[ -z $QUIET ]]
then
    echo "OK : quietens down with -q"
else
    echo "FAILURE : keeps chatting even though -q was used"
    FAILURES="yes"
fi


cp abin abintest
../endlines unix -v abintest 2>bintest >/dev/null
BINARY=`cat bintest`
if [[ $BINARY == *skipped* ]]
then
    echo "OK : skips binaries"
else
    echo "FAILURE : didn't mention skipping a binary file"
    FAILURES="yes"
fi

cp abin bbintest
../endlines unix -v -b bbintest 2>binforcetest >/dev/null
BINARYFORCE=`cat binforcetest`
if [[ $BINARYFORCE != *skipped* ]]
then
    echo "OK : option -b forces conversion of binaries"
else
    echo "FAILURE : skipped a binary in spite of -b"
    FAILURES="yes"
fi

cp unixref timetest
touch -t 200001010000 timetest
ORIGINALTIME=`ls -l timetest`
../endlines unix -k timetest 2>/dev/null >/dev/null
KEEPTIME=`ls -l timetest`
../endlines unix timetest 2>/dev/null >/dev/null
CHNGTIME=`ls -l timetest`
if [[ ($ORIGINALTIME == $KEEPTIME) && ($ORIGINALTIME != $CHNGTIME) ]]
then
    echo "OK : option -k preserves the file's modification time"
else
    echo "FAILURE : a file time was changed when it shouldn't, or kept when it shouldn't"
    FAILURES="yes"
fi




#
# Part 4 : multiple files...
#

mkdir dummydir
cp unixref multi1test
cp unixref multi2test
cp unixref multi3test
../endlines win -q dummydir multi1test multi2test multi3test

MULTIONE=`$MD5<multi1test`
MULTITWO=`$MD5<multi2test`
MULTITHREE=`$MD5<multi3test`
if [[ "$MULTIONE" == "$WINREF" &&  "$MULTITWO" == "$WINREF" && "$MULTITHREE" == "$WINREF" ]]
then
    echo "OK : multiple files"
else
    echo "FAILURE : failure to handle multiple files"
    FAILURES="yes"
fi
rmdir dummydir



#
# Part 5 : dealing with failure
#

../endlines unix doesnotexist 2>notexisttest >/dev/null
DOESNOTEXIST=`cat notexisttest`
if [[ $DOESNOTEXIST == *"can not read"* ]]
then
    echo "OK : notifies the user if a file can't be opened"
else
    echo "FAILURE : didn't mention hitting a non-existent file name"
    FAILURES="yes"
fi


cp unixref noreadtest
chmod -r noreadtest
../endlines unix noreadtest 2>noreadresulttest >/dev/null
NOREAD=`cat noreadresulttest`
if [[ $NOREAD == *"can not read"* ]]
then
    echo "OK : notifies the user if a file can't be read"
else
    echo "FAILURE : didn't mention hitting a an unreadable file"
    FAILURES="yes"
fi
chmod +r noreadtest


cp unixref nowritetest
chmod -w nowritetest
../endlines unix nowritetest 2>nowriteresulttest >/dev/null
NOWRITE=`cat nowriteresulttest`
if [[ $NOWRITE == *"can not write"* ]]
then
    echo "OK : notifies the user if a file is write protected"
else
    echo "FAILURE : didn't mention hitting a write protected file"
    FAILURES="yes"
fi
chmod +w nowritetest


touch .tmp_endlines
chmod -w .tmp_endlines
../endlines unix unixref 2>notemptest >/dev/null
NOTEMP=`cat notemptest`
if [[ $NOTEMP == *"can not create"* ]]
then
    echo "OK : notifies the user if the temporary file can not be created"
else
    echo "FAILURE : didn't mention unability to create temporary file"
    FAILURES="yes"
fi
chmod +w .tmp_endlines
rm .tmp_endlines





rm *test

echo
if [[ -z $FAILURES ]]
then
    echo "All tests succeeded"
else
    echo "** FAILURES OCCURED **"
fi
