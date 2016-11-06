#!/bin/bash

./clean_sandbox.sh

cp data/unixref sandbox/permstest
chmod 753 sandbox/permstest
$ENDLINES unix sandbox/permstest &>/dev/null
PERMISSIONS=`ls -l sandbox/permstest`
if [[ $PERMISSIONS == *"rwxr-x-wx"* ]]
then
    echo "OK : preserves file permissions"
else
    echo "FAILURE : failed to preserve file permissions"
    ./case_failed.sh
fi
