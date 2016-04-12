/* Close all files in thread using close(-1), trying to close after that
	should result in silent exit or exit(-1) */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int handle = open ("sample.txt");
  int handle2 = open ("sample2.txt"); 
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  CHECK ((handle2 = open ("sample2.txt")) > 1, "open \"sample2.txt\"");
  close(-1);

}
