#!/bin/bash
./clean_sandbox.sh

$ENDLINES unix doesnotexist 2>sandbox/notexisttest >/dev/null
DOESNOTEXIST=`cat sandbox/notexisttest`
if [[ $DOESNOTEXIST == *"an not read"* ]]
then
    echo "OK : notifies the user if a file can't be opened"
else
    echo "FAILURE : didn't mention hitting a non-existent file name"
    ./case_failed.sh
fi


cp data/unixref sandbox/noreadtest
chmod -r sandbox/noreadtest
$ENDLINES unix sandbox/noreadtest 2>sandbox/noreadresulttest >/dev/null
NOREAD=`cat sandbox/noreadresulttest`
if [[ $NOREAD == *"can not read"* ]]
then
    echo "OK : notifies the user if a file can't be read"
else
    echo "FAILURE : didn't mention hitting a an unreadable file"
    ./case_failed.sh
fi
chmod +r sandbox/noreadtest


cp data/unixref sandbox/nowritetest
chmod -w sandbox/nowritetest
$ENDLINES unix sandbox/nowritetest 2>sandbox/nowriteresulttest >/dev/null
NOWRITE=`cat sandbox/nowriteresulttest`
if [[ $NOWRITE == *"can not write"* ]]
then
    echo "OK : notifies the user if a file is write protected"
else
    echo "FAILURE : didn't mention hitting a write protected file"
    ./case_failed.sh
fi
chmod +w sandbox/nowritetest


touch sandbox/.tmp_endlines
chmod -w sandbox/.tmp_endlines
cp data/unixref sandbox/dummy
$ENDLINES win sandbox/dummy 2>sandbox/notemptest >/dev/null
NOTEMP=`cat sandbox/notemptest`
if [[ $NOTEMP == *"can not create"* ]]
then
    echo "OK : notifies the user if the temporary file can not be created"
else
    echo "FAILURE : didn't mention unability to create temporary file"
    ./case_failed.sh
fi
chmod +w sandbox/.tmp_endlines
rm sandbox/.tmp_endlines
