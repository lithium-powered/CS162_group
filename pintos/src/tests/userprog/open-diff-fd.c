/* Tries to open the two different files,
   which must succeed and must return a different file descriptor
   in each case. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int h1 = open ("sample.txt");
  int h2 = open ("sample2.txt");  

  CHECK ((h1 = open ("sample.txt")) > 1, "open \"sample.txt\"");
  CHECK ((h2 = open ("sample2.txt")) > 1, "open \"sample2.txt\"");
  if (h1 == h2)
    fail ("open() returned %d both times", h1);
}
