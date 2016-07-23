// @COMPILE OK
// @EXPECT 42
{

  if (2==1);
  else {
    if (2==1) {}
    else {
      return 42;
    }
  }

  return 0;
}
