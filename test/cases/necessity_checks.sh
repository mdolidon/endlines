#!/bin/bash
./clean_sandbox.sh

cp data/unixref sandbox/notouchtest

touch -t 200001010000 sandbox/notouchtest
NOTOUCH=`ls -l sandbox/notouchtest`
$ENDLINES unix sandbox/notouchtest 2>/dev/null >/dev/null
NOTOUCH_CHECK=`ls -l sandbox/notouchtest`
if [[ ($NOTOUCH == $NOTOUCH_CHECK)  ]]
then
    echo "OK : does not rewrite a file that is already in the target convention"
else
    echo "FAILURE : a file was rewritten although it did not need to be converted"
    ./case_failed.sh
fi


cp data/lf_then_crlf_ref sandbox/lf_then_crlf_test

touch -t 200001010000 sandbox/lf_then_crlf_test
LFUNTOUCHED=`ls -l sandbox/lf_then_crlf_test`
$ENDLINES unix sandbox/lf_then_crlf_test 2>/dev/null >/dev/null
LFTOUCHED=`ls -l sandbox/lf_then_crlf_test`
if [[ ($LFUNTOUCHED != $LFTOUCHED)  ]]
then
    echo "OK : identifies mixed conventions as needing conversion"
else
    echo "FAILURE : a file with mixed conventions was misidentified as already in target convention"
    ./case_failed.sh
fi

