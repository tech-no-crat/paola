# Testing script.
# Compiles every *.c file under ./tests with paola, and looks for testing directives: special
# comments starting with @, that specify whether the code is expected to compile or not, what the
# compiled executable should be returning etc.
# Available directives:
# @COMPILE_STATUS {int8}: The status the compiler is expected to return.
# @COMPILE OK: Alias for @COMPILE_STATUS 0
# @COMPILE_MESSAGE {string}: (TODO) Part of the messages the compiler is expected to print.
# @EXPECT {int8}: The exit status of the compiled executable.

fail=0
pass=0
total=0
comp="./bin/paola"

REDCOL='\033[0;31m'
GREENCOL='\033[0;32m'
NOCOL='\033[0m'

run_test () {
  total=$((total+1))
  expected_binary_status=`cat $test | sed -En 's/.*@EXPECT ([0-9]+)/\1/p'`
  expected_compile_status=`cat $test | sed -En 's/.*@COMPILE_STATUS ([0-9]+)/\1/p'`

  grep -Fq "@COMPILE OK" $test
  compile_ok=$?
  
  if [ $compile_ok == 0 ]; then
    if [ "$expected_compile_status" != "" ]; then
      echo "$REDCOL FAIL$NOCOL $test:\n\t Both @COMPILE OK and @COMPILE_STATUS directives were specified."
      fail=$((fail+1))
      return;
    fi
    expected_compile_status=0
  fi

  if [ "$expected_compile_status" == "" ]; then
    echo "$REDCOL FAIL$NOCOL $test:\n\tNo compile status expectation specified in test."
    fail=$((fail+1))
    return;
  fi

  ($comp $test > /dev/null)
  compile_status=$?
  if [ "$compile_status" -ne "$expected_compile_status" ]; then
    echo "$REDCOL FAIL$NOCOL $test:\n\tCompilation returned exit status $compile_status, \
expected $expected_compile_status."
    fail=$((fail+1))
    return;
  fi

  if [ "$expected_compile_status" -ne 0 ]; then
    # The program was not supposed to compile, the test passed and we're done.
    pass=$((pass+1))
    return;
  fi

  g++ out.s -o out
  assemblestatus=$?
  if [ "$assemblestatus" -ne 0 ]; then
    echo "$REDCOL FAIL$NOCOL $test:\n\tAssembling returned non-zero exit status $assemblestatus."
    fail=$((fail+1))
    return;
  fi

  (./out)
  actual_binary_status=$?
  if [ "$actual_binary_status" -ne "$expected_binary_status" ]; then
    echo "$REDCOL FAIL$NOCOL $test:\n\tExpected $expect, got $actual."
    fail=$((fail+1))
  else
    echo "$GREENCOL PASS$NOCOL $test "
    pass=$((pass+1))
  fi
}

for test in $(find ./tests -name '*.c'); do
  run_test
done

rm -f out.s
rm -f out

echo ""
if [ $pass -eq $total ]; then
  echo "$GREENCOL PASS $pass/$total $NOCOL"
  exit 0
else
  echo "$REDCOL PASS $pass/$total $NOCOL"
  exit 1
fi
