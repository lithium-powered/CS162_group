Final Report for Project 2: User Programs
=========================================

##The changes you made since your initial design document and why you made them (feel free to re-iterate what you discussed with your TA in the design review)

We realized we needed a way for parent threads to access the exit status of each child thread (and consequently, a way to track which child belongs to which parent thread and vice versa). At the design review, we talked with Andrew about keeping a list of child threads in each parent thread and storing relevant information like exit status. We implemented this by adding a linked list of child structs to each thread. Each elem in this linked list stores a child id and the child?s relevant information, like status (an integer that represents the child?s exit status at time of death). We originally had an implementation with a pointer from the child to the parent and iterated through the parents children but realized this would fail if the parent died first. So we modified it such that children have pointers to their own nodes within their parents? linked lists, so that they can update their statuses when they exit. To ensure safe synchronization, we use a lock so that only one thread can write to or read the status at a time. Each node has its own lock called memory_lock that the child and parent use to access the node in a threadsafe manner.

We realized we needed to replace the temporary semaphore in process.c with a better method of synchronization. Essentially, when a new thread is being created, the calling process, process_execute(), cannot return a valid tid until it has ensured that the child thread has successfully loaded the executable. We solved this by using a semaphore that is shared between parent and child. The parent calls sema_down() on the semaphore when it gets created and waits for the child to call sema_up() after it is done loading. We also had to work with this semaphore when a load fails so that if the load didnt succeed, the the semaphore had to be called when the thread_exit(-1). Then, the parent can check the child?s status to see if the child loaded correctly and respond accordingly. Without proper synchronization, the thread might exit after the child loads. 

We realized that not all the syscalls can be simply calling the appropriate function for each, as some of the syscalls required some management for each thread to keep track of what each thread has successfully completed, because some tasks need to be completed in a certain order (for example, a parent thread should not continue until ensuring a child has loaded properly or that a necessary file has been opened correctly). In addition, in order to ensure read and write rules for the executable files of child processes we had to properly deny and allow write permissions when an executable was being run. We had to also ensure we allowed permissions on file_close calls and replace calls the wrongly overwrite our permissions.The same approach applied to all file accesses (for example, we used a global lock for the file system to ensure that file opens/reads/writes/etc were all threadsafe)

## A reflection on the project ? what exactly did each member do? What went well, and what could be improved?

###Albert
- Completed task 3.
- For some of the functions it was really straight forward as we just had to call the right function and use the global lock to ensure thread safety. Checking valid pointers and addresses was a bit difficult to figure out at first. 
- I had trouble with properly exiting as well as setting up a list to track all the fd a thread has. In addition it was confusing how to figure out how to implement ROX as the skeleton code for load initially had aspects that affected my implementation. 

###Nerissa
- Completed task 2
- Debugged some edge cases for entire project
- Debugging multi-oom
- Generating the tests/test idea and helped write it up
- Coding in pairs was particularly helpful and found that things that I couldn?t make a lot of progress on when coding alone were fairly quick and easy when coding in pairs, since we could talk through the errors and misconceptions
- It would be better to detail a log of the changes we make so that we know what the changes that lead to successfully passing tests/fixing our errors are instead of trying a combination of things and then finding one of them works. This was a huge mistake since when we accidentally lost code due to an improper commit, it was next to impossible to track down the changed line that caused it

###Annie
- Completed task 2
- Debugged some edge cases for entire project
- Debugging multi-oom
- Nothing really went well but I finally found all the bugs! Yay! I really struggled with understanding task 2. In particular, I struggled with replacing the temporary semaphore because I had a hard time understanding how the temporary semaphore was necessary for ensuring thread safety in loading executables and, similarly, I was confused about the difference between the temporary semaphore and wait synchronization. This led to a lot of theoretical misunderstandings while I was debugging.

###Li
- Completed task 1.
- Wrote up most of the test suite
- The implementation and logic of this task was straightforward which made code writing pretty simple.
- However, I had trouble with knowing the assertions we could make for the argument input. I understand that we could have just made a reasonable guess at capping the number of arguments but it made no mention at the size of the arguments. I tried to cap it at a number of arguments and characters that would fit into a `PGSIZE` only to realize that the system already caps the argument strlen to ~128 bytes.

###Overall
- As a group we need to more thoroughly discuss the project when we are designing it. We did a lot of things along the way since we were developing a fuller understanding of what was required of us. This left us taking too broad a view during our design review and off the bat, gave us a hard time starting on some of the functions like wait and exit. 
- The really good part was our initiative in completing the project, we worked extremely early on to get the basic parts out of the way and left enough time to work together on the tough parts that required a lot of discussing and talking through. Since we could review each other?s segments thoroughly we were also able to ensure that our misunderstood segments were cleared up either using each other?s understanding or by going to Office Hours
- We need to be careful about functions added and how we path these functions - sometimes adding certain code can bypass previous code that served a similar purpose. This is especially dangerous if either segment is modified to do more. This also applies to locks, if anyone adds a seemingly harmless but wrong sema_up or sema_down call it could unsynchronise the whole system and these changes are critical and must be communicated

#Student Testing Report									
											
##Provide a description of the feature your test case is supposed to test.

###Filesize:
Checks to make sure the filesize syscall is giving the correct values.

###Fail-load: 
Checks to make sure that having more than 32 arguments for a process results in a load error.

##Provide an overview of how the mechanics of your test case work, as well as a qualitative description of the expected output.

###Filesize:
Opens sample.txt.
Calls filesize() on the fd of the sample.txt file and compares it to the byte size of sample.txt.
Prints out that the filesize is correct if the actual value is the same as the expected value.

###Fail-load:
Used exec-arg as a template to create a child process with more than 32 arguments.
This causes load to fail and print a message that there were too many arguments.
The child process then returns exit(-1) and the parent exits with exit(0).


##Provide the output of your own Pintos kernel when you run the test case.

###Filesize:
```
Copying tests/userprog/filesize to scratch partition...
Copying ../../tests/userprog/sample.txt to scratch partition...
qemu -hda /tmp/cVa5ZQtFYf.dsk -m 4 -net none -nographic -monitor null
PiLo hda1
Loading..........
Kernel command line: -q -f extract run filesize
Pintos booting with 4,088 kB RAM...
382 pages available in kernel pool.
382 pages available in user pool.
Calibrating timer...  105,267,200 loops/s.
hda: 5,040 sectors (2 MB), model "QM00001", serial "QEMU HARDDISK"
hda1: 175 sectors (87 kB), Pintos OS kernel (20)
hda2: 4,096 sectors (2 MB), Pintos file system (21)
hda3: 103 sectors (51 kB), Pintos scratch (22)
filesys: using hda2
scratch: using hda3
Formatting file system...done.
Boot complete.
Extracting ustar archive from scratch device into file system...
Putting 'filesize' into the file system...
Putting 'sample.txt' into the file system...
Erasing ustar archive...
Executing 'filesize':
(filesize) begin
(filesize) file size is 239
(filesize) filesize is correct
(filesize) end
filesize: exit(0)
Execution of 'filesize' complete.
Timer: 82 ticks
Thread: 0 idle ticks, 79 kernel ticks, 3 user ticks
hda2 (filesys): 94 reads, 212 writes
hda3 (scratch): 102 reads, 2 writes
Console: 982 characters output
Keyboard: 0 keys pressed
Exception: 0 page faults
Powering off?
Result:
PASS
```

###Fail-load:
```
Output:
Copying tests/userprog/load-fail to scratch partition?
Copying tests/userprog/child-args to scratch partition...
qemu -hda /tmp/CqVQxTLKaz.dsk -m 4 -net none -nographic -monitor null
PiLo hda1
Loading..........
Kernel command line: -q -f extract run load-fail
Pintos booting with 4,088 kB RAM...
382 pages available in kernel pool.
382 pages available in user pool.
Calibrating timer...  87,859,200 loops/s.
hda: 5,040 sectors (2 MB), model "QM00001", serial "QEMU HARDDISK"
hda1: 175 sectors (87 kB), Pintos OS kernel (20)
hda2: 4,096 sectors (2 MB), Pintos file system (21)
hda3: 199 sectors (99 kB), Pintos scratch (22)
filesys: using hda2
scratch: using hda3
Formatting file system...done.
Boot complete.
Extracting ustar archive from scratch device into file system...
Putting 'load-fail' into the file system...
Putting 'child-args' into the file system...
Erasing ustar archive...
Executing 'load-fail':
(load-fail) begin
load failed, too many arguments
(load-fail) exec("child-args"): -1
(load-fail) end
load-fail: exit(0)
Execution of 'load-fail' complete.
Timer: 77 ticks
Thread: 0 idle ticks, 74 kernel ticks, 3 user ticks
hda2 (filesys): 94 reads, 404 writes
hda3 (scratch): 198 reads, 2 writes
Console: 996 characters output
Keyboard: 0 keys pressed
Exception: 0 page faults
Powering off?
	Result:
		PASS
```

##Identify two non-trivial potential kernel bugs, and explain how they would have affected your output of this test case.

###Filesize:
1. If the kernel function file_length returned the wrong value, our filesize syscall would not work properly and the file size would be wrong.
2. If the kernel function file_length returned a pointer instead of an int value, our filesize function will probably return a huge number that does that make sense and would not match our expected values.

###Fail-load:
1. The argument input is capped at ~128 characters. However if the kernel did not have this cap, we could input a file_name with the second argument being extremely long. This could pass the check for the number of arguments but could crash the program by being larger than the stack. This would cause errors in the test.
2. If the kernel had a bug where it would read the arguments incorrectly, our load would function correctly, but the output of the test will error.
	

##Tell us about your experience writing tests for Pintos. What can be improved about the Pintos testing system? (There?s a lot of room for improvement.) What did you learn from writing test cases?

Adding the tests to be able to run using the Make.test file was pretty simple. However, trying to copy some tests proved to be a hassle. There were so subtle things that were hard to discover and could have been explained better so that we could write better tests. It was pretty limiting to have to use other tests as a template and really hard to write tests if we couldn?t find a template that was close enough to match. For example, trying to copy the args tests, we could not find a way to add more tests that would run with arg.c, even though we added in the necessary variable changes and add ons into Make.test.
