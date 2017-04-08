#!/bin/bash
./clean_sandbox.sh

echo "" | $ENDLINES dunnothis 2>sandbox/unknowntest >/dev/null
UNKNOWN=`cat sandbox/unknowntest`
if [[ $UNKNOWN == *"unknown action"* ]]
then
      echo "OK : reacts to an unknown convention"
else
    echo "FAILURE : failed to react to an unknown convention"
    ./case_failed.sh
fi

echo "" | $ENDLINES unix -q 2>sandbox/quiettest >/dev/null
QUIET=`cat sandbox/quiettest`
if [[ -z $QUIET ]]
then
    echo "OK : quietens down with -q"
else
    echo "FAILURE : keeps chatting even though -q was used"
    ./case_failed.sh
fi


cp data/abin sandbox/abintest
$ENDLINES unix -v sandbox/abintest >sandbox/bintest
BINARY=`cat sandbox/bintest`
if [[ $BINARY == *skipped* ]]
then
    echo "OK : skips binaries"
else
    echo "FAILURE : didn't mention skipping a binary file"
    ./case_failed.sh
fi

cp data/abin sandbox/bbintest
$ENDLINES unix -v -b sandbox/bbintest >sandbox/binforcetest
BINARYFORCE=`cat sandbox/binforcetest`
if [[ $BINARYFORCE != *skipped* ]]
then
    echo "OK : option -b forces conversion of binaries"
else
    echo "FAILURE : skipped a binary in spite of -b"
    ./case_failed.sh
fi

cp data/bin_as_per_extension.exe sandbox/bin_as_per_extension_test.exe
$ENDLINES unix -v sandbox/bin_as_per_extension_test.exe >sandbox/extbintest
EXTBINARY=`cat sandbox/extbintest`
if [[ $EXTBINARY == *skipped* ]]
then
    echo "OK : skips binaries with known extension without reading contents"
else
    echo "FAILURE : didn't mention skipping a binary file with a known extension"
    ./case_failed.sh
fi

cp data/bin_as_per_extension.exe sandbox/bin_as_per_extension_forced_test.exe
$ENDLINES unix -v -b sandbox/bbintest >sandbox/extbinforcetest
EXTBINARYFORCE=`cat sandbox/extbinforcetest`
if [[ $EXTBINARYFORCE != *skipped* ]]
then
    echo "OK : option -b forces conversion of binaries with known extension"
else
    echo "FAILURE : skipped a binary with a known binary extension in spite of -b"
    ./case_failed.sh
fi

cp data/unixref sandbox/timetest
touch -t 200001010000 sandbox/timetest
ORIGINALTIME=`ls -l sandbox/timetest`
$ENDLINES cr -k sandbox/timetest 2>/dev/null >/dev/null
KEEPTIME=`ls -l sandbox/timetest`
$ENDLINES lf sandbox/timetest 2>/dev/null >/dev/null
CHNGTIME=`ls -l sandbox/timetest`
if [[ ($ORIGINALTIME == $KEEPTIME) && ($ORIGINALTIME != $CHNGTIME) ]]
then
    echo "OK : option -k preserves the file's modification time"
else
    echo "FAILURE : a file time was changed when it shouldn't, or kept when it shouldn't"
    ./case_failed.sh
fi




cp data/single_line_no_newline sandbox/noeol
NOEOLREF=`$MD5<data/single_line_no_newline`
UNIXEOLREF=`$MD5<data/single_line_unix_newline`
WINEOLREF=`$MD5<data/single_line_win_newline`

$ENDLINES win sandbox/noeol >/dev/null
NOEOL_OUT=`$MD5<sandbox/noeol`
if [[ $NOEOL_OUT == $NOEOLREF ]]
then
    echo "OK : no new-line gets added without the --final flag"
else
    echo "FAILURE : a file without a final eol was changed without the --final flag"
fi


$ENDLINES win sandbox/noeol --final >/dev/null
NOEOL_FINAL_OUT=`$MD5<sandbox/noeol`
if [[ $NOEOL_FINAL_OUT == $WINEOLREF ]]
then
    echo "OK : a new-line was added by the --final flag to a text without final eol"
else
    echo "FAILURE : a run with the --final flag did not yield the expected output"
fi


$ENDLINES unix sandbox/noeol --final >/dev/null
NOEOL_FINAL_OUT_2=`$MD5<sandbox/noeol`
if [[ $NOEOL_FINAL_OUT_2 == $UNIXEOLREF ]]
then
    echo "OK : no new-line was added by the --final flag to a text that already had a final eol"
else
    echo "FAILURE : a run with the --final flag did not yield the expected output"
fi
