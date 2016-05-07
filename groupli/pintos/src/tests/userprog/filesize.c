/* Open a file. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  int handle = open ("sample.txt");
  msg("file size is %d", filesize(handle));
  if (filesize(handle) == 239)
  	 msg("filesize is correct");
}
