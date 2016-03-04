Design Document for Project 1: Threads
======================================

## Group Members

* Albert Weng <albertweng@berkeley.edu>
* Annie Lin <annie_lin@berkeley.edu>
* Liquin Yu <liyu@berkeley.edu>
* Nerissa Lin <lin.nerissa@berkeley.edu>

# Proposed Design

## Task 1: Timer Sleep

### Progress and To-Do

Progress:
*	Added static struct list elem to init.c at 120
* 	Made note on Thread.H line 31 is potential place for static word
* 	Tell li his var in thread should probably be a list and not a list elem
* 	Added to do note in thread.c threadtick  on line 141
* 	line 95 in timer.c
	//Instead of this while loop, change so that thread's sleep count is initialized
  	//Call yield or potentially other function -> Checks that timer has a number and puts it into the list
  	//Change ticks so that it checks for new threads to throw on ready

* Once enough ticks have passed, call list_push_back (&ready_list, &t->elem); thread.c 242

* 	when timer_sleep is called, add the thread to the sleep thread
* 	added sleep_list to thread.c 27 where ready thread is declared
* 	commented out while look in timer_sleep to replace with the comments in that section
* 	removed init in init.c and added it to thread_init where ready threads are init too (93 thread.c) 
* Look at thread_start and how it is initialized with a semaphore, do that for every thread started somehow
* 	add sleep_time int in the thread_create? or init_thread (probably init_thread but not going to make any changes to it)
* 	In timer.c added sleep_time to sleep value right after asserting interrupts are on
* 	yielded thread so -> yield MUST add it to the sleep list 
* 	In thread.c yield line 314 adding funciton where check for thread sleep time = if not up yet, add to sleep_list else add to ready list
* CHECK INTERRUPTS AFTER THIS NOTHING MIGHT WORK OOOOPS LOOOL T_T
* 	Insert ordered takes a FUNCTION and AUX that i havent filled in on 324 of thread.c
* FIND MISSING NOT_READY CALL FOR A THREAD STATUS I FUCKED UP.

* 	Returns true if A is less than B, or false if A is greater than or equal to B. 
* Adding comparator inside list.c at 526 called compare_sleeptime_priority ##MIGHT ERROR BECAUSE thread_elem_A->thread->sleep_time. might need to be .sleeptime
* 	Fixed the earlier insert ordered call
* 	init sleeptime to 0 in 108 of thread.c
* 	NOTE: interrupt off on works like enum intr_level old_level = intr_disable (); then intr_set_level (old_level);
* 	Checking for awake threads will occur at the time of Schedule () aka the moving of asleep threads to ready threads will appear in code in schedule
* Concept: To make it short, check for any awake threads and if there are, move them to ready within interrupts?
* 	Added to thread.c588 schedule function call to sleep_to_ready() which moves all ready to awake threads to ready_list
* 	Sleep_to_ready added at 522 thread.c under next_thread_to_run and is completed outside of interrupts
* MAKE A CHECK OF THE PROCESS AS IN THE STEPS
* MAKE A LIST OF PLACES IT NEEDS TO NOT BE INTERRUPTED AND INCLUDE HERE

* 	added semaphore to thread.c at line 45 aking static struct called sema
*	added sleep_time declaration at 108
*	moved list_func to thread.c so that it could be called






### 1) Data Structures and Functions

Global Variable 
```
struct list sleep_list;

#Make sleep list using list_init
list_init(&sleep_list);
```

Create a list that holds threads that are asleep.

```
struct thread_list_elem
{
	struct thread *thread; //Thread value stored in this node
	struct list_elem elem; //Points to prev and next elem in list
}

```
Need a linked-list of sleeping threads (using provided linked list library)


Add to thread struct:
```
struct semaphore *sema_sleep; // initialize with value 0 

int64_t sleep_time;

```
* Add these two fields into the thread struct. 
* sema_sleep should be created when the thread calls timer_sleep() and then the thread should be added to sleep_list as a linked list object.

New function:
```
void sleep_list_crawler(){
	
}
```
Function that looks through sleep_list and ups semaphores of threads that should be woken up.


### 2) Algorithms

- Each thread has value 0 initialized for its semaphore
- Next, timer_sleep is called 
	 * Sema_down (ie p() function) called on semaphore which now "waits for positive value then decrements the semophore"
	 * Behind the scenes this disables interrupts, releases the lock, adds the thread to sleep queue with the sleep time being sleep time + current time, and puts thread to sleep
	 * When added to the sleep queue, it will be added in sorted order using insertion sort 
	 **	This must occur in an atomic process so that two threads can't be added at the same time and the queue cant be corrupted
- Another thread should be notified to start running
- For every timer tick call, we compare the value of timer ticks (modulo number since timer cant go to infinity) to the sleep_time of each thread 
	* Since list is in order, means we should only have to go to the first item 
	* Other threads that meet the wake time criteria will be woken when this one completes running 
	* Notification done with some signal that should be called in sema_up()
- Original sleep thread must reacquire lock when awoken - Not sure if included in code already must check



### 3) Synchronization

List of all potential concurrent accesses to shared resources:

 - List of sleeping semaphores - this list actually can only be accessed by one thread at a time because only one thread can run at a time. We modify or read the list only on timer interrupts (when timer_sleep is called), all other interrupts are disabled, so no other thread can access the list or interrupt the calling thread during this time.
 - No other resources- because all threads are just going to “sleep” and should not access any resources during their sleep duration.

### 4) Rationale

We’ve considered using a linked list holding numerical values and decrementing each one during every tick, but this seems to waste a lot of needless computing power. Our current implementation also seems less complex than most because we’re just using already built in semaphore objects and just having to iterate through a linked list. It would be pretty simple to debug this code because there’s not many parts we would have to check.



## Task 2: Priority Scheduler

### 1) Data Structures and Functions

Add to thread struct:
```
struct thread *donee;
struct thread_list_elem donor_list;
int64_t effective_priority;
```
* add a donor linked list `donor_list`, each node in the linked list stores a `struct thread *donor`. This linked list stores all donated priority values of the thread. The effective priority of the thread is stored in `effective_priority`.
* add a variable struct `thread *donee`, which points to the thread that the current thread has currently donated to (points to null if thread does not have donee).
* `effective_priority` holds the priority that should be used for this thread (max of original and donated priority). Use `get_effective_priority` to set this variable.


```
int64_t get_effective_priority(struct thread *thread){
}
```
Function which should sort return the effective priority of a thread by looking at the thread's original and all donated priority and returning the highest value.


```
void donate(lock *lock, int64_t priority){
}
```
The thread that calls donate is added to the donor_list of the thread which holds lock and the envoking thread will donate its priority to the thread which holds the lock. The `struct thread *donee` variable of the envoking thread is set to point to the thread which holds the lock.


```
void sortDonations(struct thread *donee){
}
```
We sort the stack of the donor_list for each thread recursively based on their priority values, starting from thread which held the lock in the donate function.
(Is is possible to use a general compare function that sorts based on efficient_priority? Can be used for both sorting donations and ready_queue.)

To obtain the holder of a current lock, use `struct thread *holder`
in struct lock


### 2) Algorithms

* Choosing the next thread to run - The ready queue should be a linked list implementation of a priority queue, where the linked list of threads is sorted by priority (in descending order). The next thread to run should always be the thread with the highest priority.

* Acquiring a lock - When a thread tries to acquire a lock, if the lock is held by another thread, we call the donate function on that lock. It would then add the priority of the calling thread to the thread that holds this lock. We would then go through `donor_list` of each thread recursively to get the efficient priority. Once the thread which wants to acquire the lock returns, we go to our donee and remove ourselves from their donor_list, reset our donee variable to `NULL`, and recursively resort the affected donor_list in threads.

* Releasing a lock - Keeping the same functionality, would send interrupt to threads that are waiting for the released lock.

* Computing the effective priority - Each thread has a list of threads that have donated to the thread that keeps track of the history of priority values that the thread has. Calling `get_effective_priority()` would return the effective priority of a thread by looking at the thread's original and all donated priority and returning the highest value.

* Priority scheduling for semaphores, locks, condition variables - All priority scheduling will be based on popping off the first element of the ordered ready_list. All threads waiting for semaphores, locks, or condition variables will be popped back onto the ready_list and have the ready_list reordered when schedule() chooses a new thread to run.

* Changing thread’s priority - to change a thread’s priority, set that thread’s priority value to its new priority. Then, for the donee of that thread, recursively call `get_effective_priority()` to get the priority of the donee and all of the donee’s descendants (where a descendant is defined as a donee of a donee of a donee of a...etc of that thread).
			


### 3) Synchronization

List of all potential concurrent accesses to shared resources:

* Pushing a new donor onto the stack of a thread - This is thread-safe because lock_acquire is thread-safe and we only add to a list on lock-acquire.


* Re-sorting all descendants of a thread whose priority has been modified - Reordering of donor_list and ready_list should be done with interrupt off so there will not be times when sorting is finished but a value changes. If we enforce this invariant, then re-sorting lists is also thread-safe.

### 4) Rationale

- Having each thread hold a linked list of its donors helps to keep track of all priority values that have been donated to it. 

- Having pointers to donors instead of just saving int64_t values simplifies the case if a thread’s donor priority value changes. The reordering of the priority value will be handled when the linked list is resorted.

-By putting the code for the thread selection based on priority inside or close to the scheduler as possible, we avoid having to duplicate code for different sync methods.


## Task 3: Multi-level Feedback Queue Scheduler

### 1) Data Structures and Functions

Written so far: See mlfqs_recalculate, compare_priority, and thread_tick
Setting up background stuff
Getting/setting relevant values
Sorting

To Do:
The actual MLFQS formula
Interrupt handling


Update: We are currently using the class variable "ready_list" as the priority queue of threads and the instance variable "priority" as the mlfqs priority value. Each thread should be sorted by "priority". The class shares the global class variables float(fixed point real numbers) load_avg and linked list ready_threads, and load_avg is re-calculated every second (100 ticks). It has the instance variables float recent_cpu and int nice (unique to each thread) which are also re-calculated when necessary:

```
struct mlfqs_list_elem
{
	struct thread *thread;
	float priority;
}
```

Every 4 ticks, each thread updates its priority value. At a set time, the priority queue blocks interrupts and re-sorts itself. Always take the next task from the top of the priority queue (the one with the highest priority value).

```
void update(thread){
//iterate over threads in the array and update each with the given MLFQS formulas
}
```

```
void move(thread, queue){
//removes thread from its old queue and moves it to the new queue
}
```


### 2) Algorithms
IGNORE THIS NOW. CORRECT DOCUMENTATION IS ABOVE ^
The ready queue is essentially implemented as an array (hash) of queues (where the queues are implemented as linked lists) where each queue stores tasks? priority values are updated dynamically using the MLFQS formulas. Every 4 ticks, the priority values for all threads are re-calculated. To choose the next thread to run, go down the array of priority queues starting with the highest priority and choose the first queue that is not empty. Then update the ready_threads with the threads in that priority queue and compute the new load_avg. The first thread to be run will be the first element of the queue. Ties are broken by always taking whichever is first in the queue (at the head).



### 3) Synchronization

List of all potential concurrent accesses to shared resources:
-The list of priority queues - The list of priority queues is shared by all threads. For that reason, the update function is purposefully implemented so that the list itself iterates through all queues and all threads within queues and updates each thread (instead of threads updating themselves.) Forcing the list to update threads one at a time prevents threads from racing and modifying the same queues, therefore ensuring data integrity.

-The class variables - The class variables (the ready thread list and the load average) are shared by all threads because all threads add to the same ready thread list and use the same load average to calculate their priority values. For that reason, those variables should be updated only before or after the priority queue list is updating itself. Enforcing this invariant along with the invariant that threads do not update themselves or class variables ensures integrity of data.

### 4) Rationale

- Using an array of queues allows us to hash the queues for easy retrieval (each queue's hash value is its priority value). Elements in the hash table can be accessed in O(1) time and in this case, the hash table never has to be resized, as it will never exceed 64 entries in length.

- Storing key values like the load_avg as class variables makes it easy to access them and ensures that there is only one true value used for calculations (that is, there aren't multiple load_avgs floating around and when we update the load average, we only need to update the one global variable load_avg that all the threads rely on).

- In the update function, making the list of queues iterate through all threads and update each in turn prevents race conditions by ensuring one central authority is doing all updates and preventing multiple threads from updating at the same time. This makes sure there are no race conditions that affect the queues (since linked lists are not thread-safe) and ensure only one authority (the linked list of queues) is accessing the list at one time, so the integrity of the queues is preserved.


# Final Questions

### Question 1

Consider the following scenario: Thread A has priority 3, Thread B has priority 1, and Thread C has priority 2. C has not been placed on the ready queue yet. B currently has Lock 1, which A needs. A little while later, C joins the ready queue.

Expected behavior: A donates its priority value to B, so B now has a priority of 3.  Now, C has joined the ready queue with a priority value of 2. B has the higher priority (an effective priority of 3), so B runs and then frees Lock 1. A has the highest priority value now, so A takes Lock 1 and runs. Then, finally, C runs.

Behavior with bug: A donates its priority value to B, so B now has a priority of 3. Now, C has joined the ready queue with a priority value of 2. However, since there is a bug that causes the implementation to only look at base priority and not the effective priority, the scheduler thinks A has a priority of 3, B has a priority of 1, and C has a priority of 2, so the scheduler will choose to run C, then A, and then finally B.

Test code: Let thread B have an atomic function where it releases the lock and prints out that the lock has been released. Let thread C have a print statement anywhere in its run time. If there was no bug, B would always release its lock and print out a notice, then C would print something. However, if the bug did exist, after A blocks, we would start running C and C’s would print before B does.

### Question 2

timer ticks | R(A) | R(B) | R(C) | P(A) | P(B) | P(C) | thread to run
------------|------|------|------|------|------|------|--------------
 0          |0     |   0  |    0 |    63|    61|    59|A
 4          |4     |   0  |    0 |    62|    61|    59|A
 8          |8     |   0  |    0 |    61|    61|    59|A
12          | 12   |   0 |   0 |    60|    61|    59|A
16          | 12   |   4 |   0 |    60|    60|    59|B
20          | 12   |   8 |   0 |    60|    59|    59|B
24          | 16   |   8 |   0 |    59|    59|    59|A
28          | 20   |   8 |   0 |    58|    59|    59|A
32          | 20   |   12 |   0 |    58|    58|    59|B
36          | 20   |   12 |   4 |    58|    58|    58|C


### Question 3
Yes, we broke ties by the following rule: If there is a tie between multiple threads, give preference to the thread that is already running. If none of the tied threads are currently running, give preference to the ties by alphabetical order.
