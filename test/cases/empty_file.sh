#!/bin/bash
./clean_sandbox.sh

touch sandbox/empty
EMPTYREF=`$MD5<sandbox/empty`
$ENDLINES unix sandbox/empty >/dev/null
EMPTYTEST=`$MD5<sandbox/empty`
if [[ "$EMPTYREF" == "$EMPTYTEST" ]]
then
    echo "OK : empty file passes"
else
    echo "FAILURE : the empty file was not processed correctly"
    ./case_failed.sh
fi


