/* Grows a file from 0 bytes to 200*512 bytes, 1 bytes at a time,
   and checks to make sure the writes to the device is less than the reads. */

#include <syscall.h>
#include "tests/filesys/seq-test.h"
#include "tests/lib.h"
#include "tests/main.h"

static char buf[200*512];

static size_t
return_block_size (void) 
{
  return 1;
}

static void
check_file_size (int fd, long ofs) 
{
  long size = filesize (fd);
  if (size != ofs)
    fail ("filesize not updated properly: should be %ld, actually %ld",
          ofs, size);
}

void
test_main (void) 
{
  unsigned long long cur_write_cnt = block_write_cnt_call();
  seq_test ("testfile",
            buf, sizeof buf, 0,
            return_block_size, check_file_size);
  CHECK (block_write_cnt_call() - cur_write_cnt < 200*512, "writes were coalesced");
}
