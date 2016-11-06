#!/bin/bash

./clean_sandbox.sh

cp data/unixref sandbox/oldmactest
$ENDLINES oldmac sandbox/oldmactest &>/dev/null

cp sandbox/oldmactest sandbox/wintest
$ENDLINES win sandbox/wintest &>/dev/null

cp sandbox/wintest sandbox/unixtest
$ENDLINES unix sandbox/unixtest &>/dev/null

UNIXREF=`$MD5<data/unixref`
WINREF=`$MD5<data/winref`
OLDMACREF=`$MD5<data/oldmacref`

UNIXTEST=`$MD5<sandbox/unixtest`
WINTEST=`$MD5<sandbox/wintest`
OLDMACTEST=`$MD5<sandbox/oldmactest`


if [[ "$OLDMACREF" == "$OLDMACTEST" ]]
then
    echo "OK : oldmac output"
else
    echo "FAILURE : oldmac output doesn't match"
    ./case_failed.sh
fi

if [[ "$WINREF" == "$WINTEST" ]]
then
    echo "OK : windows output"
else
    echo "FAILURE : windows output doesn't match"
    ./case_failed.sh
fi

if [[ "$UNIXREF" == "$UNIXTEST" ]]
then
    echo "OK : unix output"
else
    echo "FAILURE : unix output doesn't match"
    ./case_failed.sh
fi
