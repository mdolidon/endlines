#!/bin/bash

MD5=md5 
echo
echo "hash command : $MD5"
echo

rm *test 2>/dev/null


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
fi

if [[ "$WINREF" == "$WINTEST" ]]
then
    echo "OK : windows output"
else
    echo "FAILURE : windows output doesn't match"
fi

if [[ "$UNIXREF" == "$UNIXTEST" ]]
then
    echo "OK : unix output"
else
    echo "FAILURE : unix output doesn't match"
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
fi



# Part 3 : details, options...

echo "" | ../endlines dunnothis 2>unknowntest >/dev/null
UNKNOWN=`cat unknowntest`
if [[ $UNKNOWN == *unknown* ]]
then
      echo "OK : reacts to an unknown convention"
else
    echo "FAILURE : failed to react to an unknown convention"
fi

echo "" | ../endlines unix -q 2>quiettest >/dev/null
QUIET=`cat quiettest`
if [[ -z $QUIET ]]
then
    echo "OK : quietens down with -q"
else
    echo "FAILURE : keeps chatting even though -q was used"
fi

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
fi
rmdir dummydir

rm *test
