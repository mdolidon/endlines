#!/bin/bash
./clean_sandbox.sh

cp data/utf8unixref sandbox/utf8test
$ENDLINES win sandbox/utf8test >/dev/null


UTF8EXPECTED=`$MD5<data/utf8winref`
UTF8OUT=`$MD5<sandbox/utf8test`

if [[ "$UTF8EXPECTED" == "$UTF8OUT" ]]
then
    echo "OK : UTF-8"
else
    echo "FAILURE : UTF-8"
    ./case_failed.sh
fi




cp data/utf16le_unix_ref sandbox/utf16letest
$ENDLINES win sandbox/utf16letest >/dev/null


UTF16LEEXPECTED=`$MD5<data/utf16le_win_ref`
UTF16LEOUT=`$MD5<sandbox/utf16letest`

if [[ "$UTF16LEEXPECTED" == "$UTF16LEOUT" ]]
then
    echo "OK : UTF-16 Little Endian"
else
    echo "FAILURE : UTF-16 Little Endian"
    ./case_failed.sh
fi




cp data/utf16be_unix_ref sandbox/utf16betest
$ENDLINES win sandbox/utf16betest >/dev/null


UTF16BEEXPECTED=`$MD5<data/utf16be_win_ref`
UTF16BEOUT=`$MD5<sandbox/utf16betest`

if [[ "$UTF16BEEXPECTED" == "$UTF16BEOUT" ]]
then
    echo "OK : UTF-16 Big Endian"
else
    echo "FAILURE : UTF-16 Big Endian"
    ./case_failed.sh
fi

