// @COMPILE OK
// @EXPECT 100
{
  if (5 + 2) {
    if ( 5 - (2 * 2 + 1)) {
      return 1;
    } else {
      if (0) return 2;
      return 100;
    }
  } else {
    return 3;
  }
}
