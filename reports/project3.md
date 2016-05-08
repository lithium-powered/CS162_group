# Project 3 Final Report


### The changes you made since your initial design document and why you made them.
					 		

* Global cache lock

	We originally envisioned the synchronization strategy for the cache to include a lock on each cache element (each block in the cache), so that a thread would lock up the block it was trying to access upon a cache_read() or cache_write(). At the design meeting, Andrew suggested a simpler version that is a global cache lock, so that each thread acquires the lock before it accesses anything in the cache or changes any metadata. We decided to use this global lock strategy because it was much simpler and eliminated a lot of the issues surrounding thread races to use/evict individual blocks in the cache. Threads acquire this lock at the very beginning of cache_read() and cache_write() and release the lock immediately before returning from the functions.


* Implementing cache as array instead of linked list

	In our original design doc, we envisioned the cache to be implemented as a linked list of cache elem list nodes. We realized during our design meeting that an array would have the same functionality and be much more efficient to implement (because accesses to an array take constant time whereas linked list accesses require iterating through the whole list). We thus changed our cache to an array of cache elem structs.


* Using a general inode resizing function

	During the design meeting, we discussed the broader process of resizing files as allocating extra space (and assigning to the direct/indirect/doubly indirect pointers) as needed and then separately handling . After carefully looking at the inode_resizing code for Section 12, we realized that having a general inode resizing function was a really clean abstraction that could come in handy in a variety of cases. We adapted this code for our purposes and found that it was able to effectively handle direct and singly indirect pointers. We extended the functionality to handle doubly indirect pointers and made sure that our code was not only able to extend inodes, but also shrink them. We were able to use this function to easily create inodes, extend inodes, delete inodes (by resizing them to size 0), and free allocated blocks if we got an error in the middle of file extension (by recursively resizing back to the original size). Making this design decision made our code more portable and memory-leak free.


* Tracking if an inode is a directory
 
	During our design meeting, we discussed how exactly we could keep track of which inodes were directories and how to in turn navigate through them for the subdirectories task. One suggestion we were given was to implement all the information in the directory class but after many tries of using lists to track all inodes that were directories, we ended up falling back on the original plan because of many difficulties. Our design initially was to have the inodes themselves keep track of whether or not they were directories and create would feed an additional parameter for directories. We would then manage the parents through the inodes themselves. In addition, we kept track of this information in a struct in filesys and managed all the functions accordingly. We also implemented a lock on inodes and accessed this for synchronization. 	1



					
### A reflection on the project – what exactly did each member do? What went well, and what could be improved?



#### Albert
Coded and debugged task 3. Debugging with another person was extremely helpful as it really helped me catch most of my mistakes. Doing it alone initially was a poor decision. Team communication could have been improved as the independent parts had aspects that were seemingly very dependent on each other, and when we merged files many things went wrong. Also more information on debugging for this project would have been very helpful as the errors were always very similar.

#### Nerissa
I worked on task 2 implementation and getting the calculations and pointer arithmetic working for especially indirect and doubly indirect pointers. I helped work out the logic for when we need to cache write data for that and how changing the implementation of the files would affect the other functions. I found that having another person to talk through and double check my work was extremely helpful since the logic could be fixed if flawed by a secondary perspective. I thought that where previous projects working on tasks in isolation wasn't so hard, this one was particularly bad since errors could be attributed to any of the parts, from miscalculating of writes or sector numbers to failing cache implementation and it was hard to discover when we weren't absolutely certain on minute implementation details. Our hesitation to make big changes was also a huge issue since we were trying to change a full file system implementation but tried to assume that abstraction was thorough enough to cover many of the functions.


#### Annie
I worked on task 2 implementation and debugging. I adapted the inode_resize function from the section notes and also helped implement synchronization (particularly, the global cache lock). I struggled a lot with the debugging for task 2. In particular, we had trouble getting the doubly indirect pointers to store the correct sector numbers. This was especially hard to debug because we had no idea what any of the numbers meant or, when reading from the wrong location in memory, where the read data was coming from. I learned about the importance of using critical thinking while debugging - instead of re-looking over the same code over and over again or poking around for a long time, our debugging was much more successful when we examined our errors more carefully and gave consideration to why an error might be printing or how certain aspects of our implementation affect the test cases.


#### Li
I worked on task 1 and implemented the cache. I think the cache implementation was pretty straightforward so there were not many issues. The main issue I had with this part was the stack limit that I forgot about and accidently put the entire cache onto the stack instead of mallocing memory for it. This issue seemed like a pretty consistent issue we had throughout the project. Li also wrote the two student test cases for the testing report, as well as helping significantly with extensive debugging of tasks 2 and 3.



## Student Testing Report
					
					 				
#### Test 1 - Coalesce Writes to the Same Sector (cache_coal)
						 					
* Provide a description of the feature your test case is supposed to test.
						
	This case tests whether our cache coalesces writes to the same sector, that is, whether writes to the same sector are first written to the cache using the write back protocol. If we write over the same sector multiple times, we would still have only 1 call to block_write when the sector is removed from the cache.

				 							
* Provide an overview of how the mechanics of your test case work, as well as a qualitative description of the expected output.
						
	We write to the cache 1 byte at a time for a large file. This would mean that multiple writes to the cache will be on the same sector (512 writes in this case). This would mean that the number of block_write calls will be significantly less than calls to cache_write. We then check the number of block writes to make sure it is less than cache_writes. If working correctly, the test would print a message claiming that there were less block_write calls than cache_write calls.


					 							
* Provide the output of your own Pintos kernel when you run the test case.

```
Output:						 					
qemu -hda /tmp/kAVwBG4IWf.dsk -hdb tmp.dsk -m 4 -net none -nographic -monitor null
PiLo hda1
Loading...........
Kernel command line: -q -f extract run cache_coal
Pintos booting with 4,088 kB RAM...
382 pages available in kernel pool.
382 pages available in user pool.
Calibrating timer...  314,163,200 loops/s.
hda: 1,008 sectors (504 kB), model "QM00001", serial "QEMU HARDDISK"
hda1: 185 sectors (92 kB), Pintos OS kernel (20)
hda2: 235 sectors (117 kB), Pintos scratch (22)
hdb: 5,040 sectors (2 MB), model "QM00002", serial "QEMU HARDDISK"
hdb1: 4,096 sectors (2 MB), Pintos file system (21)
filesys: using hdb1
scratch: using hda2
Formatting file system...done.
Boot complete.
Extracting ustar archive from scratch device into file system...
Putting 'cache_coal' into the file system...
Putting 'tar' into the file system...
Erasing ustar archive...
Executing 'cache_coal':
(cache_coal) begin
(cache_coal) create "testfile"
(cache_coal) open "testfile"
(cache_coal) writing "testfile"
(cache_coal) close "testfile"
(cache_coal) open "testfile" for verification
(cache_coal) verified contents of "testfile"
(cache_coal) close "testfile"
(cache_coal) writes were coalesced
(cache_coal) writes were coalesced: FAILED
cache_coal: exit(1)
Execution of 'cache_coal' complete.
Timer: 713 ticks
Thread: 0 idle ticks, 68 kernel ticks, 645 user ticks
hdb1 (filesys): 913 reads, 681 writes
hda2 (scratch): 234 reads, 2 writes
Console: 1308 characters output
Keyboard: 0 keys pressed
Exception: 0 page faults
Powering off...
		

Result:
FAIL
run: writes were coalesced: FAILED
```

* Identify two non-trivial potential kernel bugs, and explain how they would have affected your output of this test case.
						
1) If the kernel did not use an effective/correct cache replacement policy, we might do a poor job of evicting blocks in a clever manner and end up evicting the most popular or still in use blocks, which would increase the number of block_writes. Even worse, a bad policy could cause us to evict all the blocks on the every sweep, meaning that our cache is effectively useless and is not any more efficient than directly reading/writing to/from disk, and we call block_write() every time we call, thus failing the test.	 				
2) If the kernel does not coalesce cache writes to the same sector and instead calls block_write() separately on each cache_write call to the same sector number, then the number of calls to cache_write() will exactly equal the number of calls to block_write() while running our test case. This would happen because the kernel doesn’t combine the writes from the same sector, thus causing inefficiency in the cache implementation.



#### Test 2 - Cache’s Ability to Write Full Blocks to Disk Without Reading Them First (cache_eff)
		
* Provide a description of the feature your test case is supposed to test.
						
Our cache has a write-back policy for handling writes. This means that when a write happens, the cache doesn’t immediately find the correct block on disk and write back to it. Instead, the cache stores the newly-written data in a cache block and marks that block as “dirty” by setting a dirty bit, meaning that eventually that cache block needs to be re-written back to disk. When this block eventually gets evicted, our code should write the data back to disk before removing the block from the cache. This test should check whether the write-back policy is working correctly by making sure we keep newly-written data in the cache by writing full block disks without calling cache_read().		 							

* Provide an overview of how the mechanics of your test case work, as well as a qualitative description of the expected output.
						
The test creates a new file called “testfile” and grows it from 0 bytes to 102400 bytes by 512 bytes at a time (by writing to the file). The test stores the original number of cache reads and writes (before creating the file), and then compares the original reads and original writes to the number of reads and writes that happen while the file is being grown. If the cache is working correctly (if it uses a write back policy like it should), there should be more writes than reads. At the end of the test, we expect block_write_cnt_call() - original number of writes to be greater than block_read_cnt_call() - original number of reads. Otherwise, the test fails.


					 							
* Provide the output of your own Pintos kernel when you run the test case.
```
Output:						
qemu -hda /tmp/l0m5QEdyIu.dsk -hdb tmp.dsk -m 4 -net none -nographic -monitor null
PiLo hda1
Loading...........
Kernel command line: -q -f extract run cache_eff
Pintos booting with 4,088 kB RAM...
382 pages available in kernel pool.
382 pages available in user pool.
Calibrating timer...  287,948,800 loops/s.
hda: 1,008 sectors (504 kB), model "QM00001", serial "QEMU HARDDISK"
hda1: 185 sectors (92 kB), Pintos OS kernel (20)
hda2: 235 sectors (117 kB), Pintos scratch (22)
hdb: 5,040 sectors (2 MB), model "QM00002", serial "QEMU HARDDISK"
hdb1: 4,096 sectors (2 MB), Pintos file system (21)
filesys: using hdb1
scratch: using hda2
Formatting file system...done.
Boot complete.
Extracting ustar archive from scratch device into file system...
Putting 'cache_eff' into the file system...
Putting 'tar' into the file system...
Erasing ustar archive...
Executing 'cache_eff':
(cache_eff) begin
(cache_eff) create "testfile"
(cache_eff) open "testfile"
(cache_eff) writing "testfile"
(cache_eff) close "testfile"
(cache_eff) open "testfile" for verification
(cache_eff) verified contents of "testfile"
(cache_eff) close "testfile"
(cache_eff) less reads than writes
(cache_eff) less reads than writes: FAILED
cache_eff: exit(1)
Execution of 'cache_eff' complete.
Timer: 77 ticks
Thread: 0 idle ticks, 66 kernel ticks, 11 user ticks
hdb1 (filesys): 913 reads, 681 writes
hda2 (scratch): 234 reads, 2 writes
Console: 1293 characters output
Keyboard: 0 keys pressed
Exception: 0 page faults
Powering off...


result:						 						
FAIL
run: less reads than writes: FAILED
```


* Identify two non-trivial potential kernel bugs, and explain how they would have affected your output of this test case.

1) If our dirty bits were not being set correctly, then we would not correctly write back data on every eviction. For example, we might miss blocks if we do not set their dirty bits when the writes first occur. This means when we finally evict a block, we do not write the new data back to disk first, so the disk never gets updated with all of the calls to cache_write().

2) When we close the filesystem, we need to evict all the blocks in the cache and write back any dirty blocks. Because the filesystem is going to close, the cache is going to be freed, so we need to make sure we iterate through the cache blocks and write back any dirty blocks prior to freeing data and closing the filesystem. If we don’t do this correctly, the last calls to cache_write() will never be written back to disk and the filesystem will close, losing the changes forever, so disk will never get correctly updated.
		 	 	 		
			
					
### Tell us about your experience writing tests for Pintos. What can be improved about the Pintos testing system? (There’s a lot of room for improvement.) What did you learn from writing test cases?						
					 				
	
We found it particularly helpful to look at some of the functions that the other test cases used, for example, seq_test(). Using these functions allowed us to re-use the same basic code as the other test cases for repeated tasks like setting up files or writing/reading to/from disk, which made writing our test cases easier and helped us effortlessly set-up files for tests. This abstraction helped our tests integrate more smoothly with the existing testing system and is just a better code-writing practice in general.

The Pintos testing system could be more helpful if it offered more output on memory-based errors. For example, if it writes to a disk location and then reads from that disk location, but finds an error (for example, output doesn’t match), it may help to print out additional information beyond what the tests currently print, like a stack backtrace or thread numbers or disk sector numbers. Our team usually handled this in GDB, but it it would have been easier to see more detailed error statements from the tests themselves in some cases. A more detailed output suite that includes key functions for things like printing output, examining memory locations, getting stack traces, and examining threads would really expand the testing capabilities and give test-writers more flexibility to add more output to test cases.

We learned that for some features, like caches, testing code efficiency is important in addition to testing code correctness. For example, a cache can function correctly by doing correct reads and writes, but be highly inefficient if it has a bad eviction policy or calls block_read() and block_write() an unnecessary number of times, thus defeating the purpose of even using the cache in the first place! Thus, some great test cases are cases that make sure implementations are efficient. This is particularly important for code that is going to handle a lot of memory or threads (like PintOS), since inefficiency can really bog down a system and slow down its speed.
