#!/bin/bash
./clean_sandbox.sh

for ((i=1;i<=500;i++));
do
    cat data/unixref >> sandbox/bigunixintest
done

$ENDLINES win <sandbox/bigunixintest 2>/dev/null | $ENDLINES oldmac 2>/dev/null | $ENDLINES unix >sandbox/bigunixouttest 2>/dev/null


BIGUNIXIN=`$MD5<sandbox/bigunixintest`
BIGUNIXOUT=`$MD5<sandbox/bigunixouttest`

if [[ "$BIGUNIXIN" == "$BIGUNIXOUT" ]]
then
    echo "OK : large file processing"
else
    echo "FAILURE : large file processing ; buffered access mechanism may be damaged"
    ./case_failed.sh
fi




