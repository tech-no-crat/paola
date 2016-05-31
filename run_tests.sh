fail=0
pass=0
total=0
comp="./bin/paola"

REDCOL='\033[0;31m'
GREENCOL='\033[0;32m'
NOCOL='\033[0m'

run_test () {
  expect=`cat $test | sed -En 's/.*@EXPECT ([0-9]+)/\1/p'`
  total=$((total+1))

  ($comp $test > /dev/null)
  compstatus=$?
  if [ $compstatus -ne 0 ]; then
    echo "$REDCOL FAIL$NOCOL $test:\n\tCompilation returned non-zero exit status $compstatus."
    fail=$((fail+1))
    return;
  fi

  g++ out.s -o out
  (./out)
  actual=$?
  if [ $actual -ne $expect ]; then
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
