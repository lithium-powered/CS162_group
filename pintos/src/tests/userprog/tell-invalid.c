/* Opens a file and then tries to close it twice.  The second
   close must either fail silently or terminate with exit code
   -1. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int handle, handle2;
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  CHECK ((handle2 = open ("sample.txt")) > 1, "open \"sample2.txt\"");
  close(-1);
  msg ("close \"sample.txt\" and \"sample2.txt\"");
  msg("Try closing both files individually");
  close (handle);
  msg ("close \"sample.txt\" again");
  close (handle2);
}
