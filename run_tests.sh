fail=0
pass=0
total=0
comp="./bin/paola"

run_test () {
  expect=`cat $test | sed -En 's/.*@EXPECT ([0-9]+)/\1/p'`

  ($comp $test > /dev/null)

  total=$((total+1))
  if [ $? -ne 0 ]; then
    echo "FAIL $test: Compilation returned non-zero exit status $?."
    fail=$((fail+1))
    return;
  fi

  g++ out.s -o out
  (./out)
  actual=$?
  if [ $actual -ne $expect ]; then
    echo "FAIL $test: Expected $expect, got $actual"
    fail=$((fail+1))
  else
    echo "PASS $test"
    pass=$((pass+1))
  fi
}

for test in $(find ./tests -name '*.c'); do
  run_test
done

rm -f out.s
rm -f out

echo ""
echo "PASS $pass/$total"
