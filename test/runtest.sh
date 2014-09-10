#!/bin/bash

MD5=md5 
echo
echo "hash command : $MD5"
echo

rm *test 2>/dev/null
FAILURES=""

# Part 1 : try various input/output combinations

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

# Part 2 : large files, testing the buffers' integrity

echo "...building a large file..."
for ((i=1;i<=500;i++));
do
    cat unixref >> bigunixintest
done

echo "...processing the large file..."
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



# Part 3 : details, options...

echo "" | ../endlines dunnothis 2>unknowntest >/dev/null
UNKNOWN=`cat unknowntest`
if [[ $UNKNOWN == *unknown* ]]
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

# Part 4 : multiple files...
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

rm *test

echo
if [[ -z $FAILURES ]]
then
    echo "All tests succeeded"
else
    echo "** FAILURES OCCURED **"
fi

