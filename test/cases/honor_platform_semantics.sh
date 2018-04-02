#!/bin/sh

# respect platform delimeter semantics
# delimiter=crlf:separator,lf:terminator

print_bare_sum=`$MD5<data/print_bare`
print_lf_sum=`$MD5<data/print_lf`
print_lf_lf_sum=`$MD5<data/print_lf_lf`
print_crlf_sum=`$MD5<data/print_crlf`
print_lf_print_sum=`$MD5<data/print_lf_print`
print_lf_print_lf_sum=`$MD5<data/print_lf_print_lf`
print_crlf_print_sum=`$MD5<data/print_crlf_print`

test_desc="(1) no delimiter changes by the --semantic flag to win line separators for win, 1 line"
cp data/print_bare sandbox/semantic-01
$ENDLINES win sandbox/semantic-01 --semantic >/dev/null
out_sum=`$MD5<sandbox/semantic-01`
if [ "$out_sum" = "$print_bare_sum" ]
then
    echo "OK : $test_desc"
else
    echo "FAILURE : $test_desc"
fi


test_desc="(2) no delimiter changes by the --semantic flag to win line separators for win, 2 line"
cp data/print_crlf sandbox/semantic-02
$ENDLINES win sandbox/semantic-02 --semantic >/dev/null
out_sum=`$MD5<sandbox/semantic-02`
if [ "$out_sum" = "$print_crlf_sum" ]
then
    echo "OK : $test_desc"
else
    echo "FAILURE : $test_desc"
fi


test_desc="(3) delimiter addition by the --semantic flag to win line separators for unix, 1 line"
cp data/print_bare sandbox/semantic-03
$ENDLINES unix sandbox/semantic-03 --semantic >/dev/null
out_sum=`$MD5<sandbox/semantic-03`
if [ "$out_sum" = "$print_lf_sum" ]
then
    echo "OK : $test_desc"
else
    echo "FAILURE : $test_desc"
fi


test_desc="(4) delimiter addition by the --semantic flag to win line separators for unix, 2 line"
cp data/print_crlf sandbox/semantic-04
$ENDLINES unix sandbox/semantic-04 --semantic >/dev/null
out_sum=`$MD5<sandbox/semantic-04`
if [ "$out_sum" = "$print_lf_lf_sum" ]
then
    echo "OK : $test_desc"
else
    echo "FAILURE : $test_desc"
fi


test_desc="(5) delimiter removal by the --semantic flag to unix line terminators for win, 1 line"
cp data/print_lf sandbox/semantic-05
$ENDLINES win sandbox/semantic-05 --semantic >/dev/null
out_sum=`$MD5<sandbox/semantic-05`
if [ "$out_sum" = "$print_bare_sum" ]
then
    echo "OK : $test_desc"
else
    echo "FAILURE : $test_desc"
fi


test_desc="(6) delimiter removal by the --semantic flag to unix line terminators for win, 2 line"
cp data/print_lf_lf sandbox/semantic-06
$ENDLINES win sandbox/semantic-06 --semantic >/dev/null
out_sum=`$MD5<sandbox/semantic-06`
if [ "$out_sum" = "$print_crlf_sum" ]
then
    echo "OK : $test_desc"
else
    echo "FAILURE : $test_desc"
fi


test_desc="(7) no delimiter changes by the --semantic flag to unix line terminators for unix, 1 line"
cp data/print_lf sandbox/semantic-07
$ENDLINES unix sandbox/semantic-07 --semantic >/dev/null
out_sum=`$MD5<sandbox/semantic-07`
if [ "$out_sum" = "$print_lf_sum" ]
then
    echo "OK : $test_desc"
else
    echo "FAILURE : $test_desc"
fi


test_desc="(8) no delimiter changes by the --semantic flag to unix line terminators for unix, 2 line"
cp data/print_lf_lf sandbox/semantic-08
$ENDLINES unix sandbox/semantic-08 --semantic >/dev/null
out_sum=`$MD5<sandbox/semantic-08`
if [ "$out_sum" = "$print_lf_lf_sum" ]
then
    echo "OK : $test_desc"
else
    echo "FAILURE : $test_desc"
fi


test_desc="(9) delimiter addition by the --semantic --final flags to unterminated unix line"
cp data/print_lf_print sandbox/semantic-09
$ENDLINES unix sandbox/semantic-09 --semantic --final >/dev/null
out_sum=`$MD5<sandbox/semantic-09`
if [ "$out_sum" = "$print_lf_print_lf_sum" ]
then
    echo "OK : $test_desc"
else
    echo "FAILURE : $test_desc"
    hexdump -C sandbox/semantic-09
fi


test_desc="(10) no delimiter changes by the --semantic --final flags to win line separators for win"
cp data/print_crlf_print sandbox/semantic-10
$ENDLINES win sandbox/semantic-10 --semantic --final >/dev/null
out_sum=`$MD5<sandbox/semantic-10`
if [ "$out_sum" = "$print_crlf_print_sum" ]
then
    echo "OK : $test_desc"
else
    echo "FAILURE : $test_desc"
fi
