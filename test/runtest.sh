#!/bin/bash


if [ -n "`command -v ../endlines`" ]
then
    export ENDLINES=../endlines
else
    echo "No endlines executable found in project root directory. Can't run tests."
    exit
fi


if [ -n "`command -v md5`" ]
then
    export MD5=md5
elif [ -n "`command -v md5sum`" ]
then
    export MD5=md5sum 
elif [ -n "`command -v csum`" ]
then
    export MD5="csum - " 
else
    echo "No MD5 hash command found, can't run tests."
    exit
fi

chmod +r data/*
chmod +w data/*
chmod +rx cases/*
chmod +x case_failed.sh
chmod +x clean_sandbox.sh

./clean_sandbox.sh

echo "">failures


echo


cases/convention_outputs.sh
cases/permissions.sh
cases/pipe.sh
cases/empty_file.sh
cases/utf.sh
cases/necessity_checks.sh
cases/large_file.sh
cases/command_line_options.sh
cases/multiple_files.sh
cases/failure_notifications.sh



./clean_sandbox.sh

echo
if [[ -z "`cat failures`" ]]
then
    echo "All tests succeeded"
else
    echo "** FAILURES OCCURED **"
fi

rm failures
