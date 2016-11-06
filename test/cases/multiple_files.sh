#!/bin/bash
./clean_sandbox.sh


mkdir dummydir
cp data/unixref sandbox/multi1test
cp data/unixref sandbox/multi2test
cp data/unixref sandbox/multi3test
$ENDLINES win -q dummydir nonexistent sandbox/multi1test sandbox/multi2test sandbox/multi3test 2>/dev/null

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
rmdir dummydir
