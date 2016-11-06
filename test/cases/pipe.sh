#!/bin/bash

./clean_sandbox.sh


cat data/unixref | $ENDLINES oldmac 2>/dev/null | $ENDLINES win 2>/dev/null >sandbox/pipetest
PIPETEST=`$MD5<sandbox/pipetest`
WINREF=`$MD5<data/winref`

if [[ "$WINREF" == "$PIPETEST" ]]
then
    echo "OK : using with pipes"
else
    echo "FAILURE : using with pipes yielded non matching output"
    ./case_failed.sh
fi
