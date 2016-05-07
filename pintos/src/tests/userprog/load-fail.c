/* Tries to execute a process with too many arguments.
   Load must fail and 
   the exec system call must return -1. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  	msg ("exec(\"child-args\"): %d", exec ("child-args a b c d e f g h i j k l m n o p q r s t \
		u v w x y z a b c d e f g h"));
}
