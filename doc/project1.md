Design Document for Project 1: Threads
======================================

## Group Members

* Albert Weng <albertweng@berkeley.edu>
* Annie Lin <annie_lin@berkeley.edu>
* Liquin Yu <liyu@berkeley.edu>
* Nerissa Lin <lin.nerissa@berkeley.edu>

# Proposed Design

## Task 1: Timer Sleep

### 1) Data Structures and Functions

```
struct list sleep_list;
list_init(&sleep_list);

struct sema_sleep_time{
	struct semaphore *sema;
	int64_t sleep time;
}

```
Creates a struct that holds a semaphore and how long the thread that created the semaphore should sleep.

```
struct sema_list_elem
{
	struct semaphore *sema; //Semaphore value stored in this node
	struct list_elem elem; //Points to prev/next elem in list
}

```
Need a linked-list of semaphores (using provided linked list library)

### 2) Algorithms

When timer_sleep is called, the thread creates a sema_sleep_time object with a semaphore valued at 0 and int64_t value of how long the thread should sleep + current tick time. It then waits for to down the newly created semaphore. The sema_sleep_time object is added to a global linked list, sleep_list. Every time timer_tick is called, we go through the semaphores in the linked list and then up them if enough tick time has passed (If the int64_t value is equal to the current timer_ticks()). 

### 3) Synchronization

List of all potential concurrent accesses to shared resources:

 - List of sleeping semaphores - this list actually can only be accessed by one thread at a time because only one thread can run at a time. We modify or read the list only on timer interrupts (when timer_sleep is called), all other interrupts are disabled, so no other thread can access the list or interrupt the calling thread during this time.
 - No other resources- because all threads are just going to “sleep” and should not access any resources during their sleep duration.

### 4) Rationale

We’ve considered using a linked list holding numerical values and decrementing each one during every tick, but this seems to waste a lot of needless computing power. Our current implementation also seems less complex than most because we’re just using already built in semaphore objects and just having to iterate through a linked list. It would be pretty simple to debug this code because there’s not many parts we would have to check.



## Task 2: Priority Scheduler

### 1) Data Structures and Functions

To the thread struct, add a donor `linked list (donor_list)` that functions as a stack (always pop/push at the head). Each node in the linked list stores a `priority_donation struct`. This linked list stores all donated/original priority values of the thread. The element at the head is the current priority:

```
struct priority_donation_list_elem
{
	struct thread *donor; // The thread that donated the value
	struct list_elem elem;
}
```

To the thread struct, add an attribute struct `thread *donee`, which points to the thread that the current thread has currently donated to (points to null if thread does not have donee);

```
void donate(lock *lock, int64_t priority){
}
```

The thread that calls donate is added to the donor_list of the thread which holds lock. 

```
void sortDonations(struct thread *donee){
}
```

We sort the stack of the donor_list for each thread recursively based on their priority values, starting from thread which held the lock in the donate function.

To obtain the holder of a current lock, use `struct thread *holder`
in struct lock


### 2) Algorithms

* Choosing the next thread to run - The ready queue should be a linked list implementation of a priority queue, where the linked list of threads is sorted by priority (in descending order). The next thread to run should always be the thread with the highest priority.

* Acquiring a lock - When a thread tries to acquire a lock, if a the lock is held by another thread, we call the donate function on that lock. It would then add the priority of the calling thread to the thread that holds this lock. We would then sort the donor_list of each thread recursively to make sure the highest priority is at the top of the stack. Once the thread which wants to acquire the lock returns, we go to our donee and remove ourselves from their donor_list, reset our donee variable to 0, and recursively resort the affected donor_list in threads.

* Releasing a lock - Keeping the same functionality, would send interrupt to threads that are waiting for the released lock.

* Computing the effective priority - Each thread has a list (that functions as a stack) of priority_donation structs that keeps track of the history of priority values (original and donated) that the thread has. The last element of the list always contains the original priority value of the thread. Calling the thread_get_priority() function should yield the priority value of the top-most element in that thread’s priority linked list.

* Priority scheduling for semaphores, locks, condition variables - All priority scheduling will be based on popping off the first element of the ordered ready_list. All threads waiting for semaphores, locks, or condition variables will be popped back onto the ready_list and have the ready_list reordered when schedule() chooses a new thread to run.

* Changing thread’s priority - to change a thread’s priority, set that thread’s priority value to its new priority. Then, for the donee of that thread, recursively re-sort the priority linked list of the donee and all of the donee’s descendants (where a descendant is defined as a donee of a donee of a donee of a...etc of that thread).
			


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

Make a list (array) of 63 priority queues, where each priority queue is a linked list holding elements of that priority value, where each node contains as its value a struct mlfqs_list_elem. This list element contains a thread. The class shares the global class variables float load_avg and linked list ready_threads, and load_avg is re-calculated every second (100 ticks). It has the instance variables float recent_cpu and int nice (unique to each thread) which are also re-calculated when necessary:

```
struct mlfqs_list_elem
{
	struct thread *thread;
	float priority;
}
```

Every 4 ticks, the list of queues updates itself. For each queue stored in the array, it iterates through that queue?s threads and updates the priority value of each thread. If a thread?s priority value has changed, the queue moves that thread to a different linked list.

```
void update(array){
//iterate over threads in the array and update each with the given MLFQS formulas
}
```

```
void move(thread, queue){
//removes thread from its old queue and moves it to the new queue
}
```


### 2) Algorithms

The ready queue is essentially implemented as an array (hash) of queues (where the queues are implemented as linked lists) where each queue stores asdf and tasks? priority values are updated dynamically using the MLFQS formulas. Every 4 ticks, the priority values for all threads are re-calculated. To choose the next thread to run, go down the array of priority queues starting with the highest priority and choose the first queue that is not empty. Then update the ready_threads with the threads in that priority queue and compute the new load_avg. The first thread to be run will be the first element of the queue. Ties are broken by always taking whichever is first in the queue (at the head).



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
 4          |4     |   4  |    4 |    62|    60|    58|A
 8          |8     |   8  |    8 |    61|    59|    57|A
12          | 12   |   12 |   12 |    60|    58|    56|A
16          | 16   |   16 |   16 |    59|    57|    55|A
20          | 20   |   20 |   20 |    58|    56|    54|A
24          | 24   |   24 |   24 |    57|    55|    53|A
28          | 28   |   28 |   28 |    56|    54|    52|A
32          | 32   |   32 |   32 |    55|    53|    51|A
36          | 36   |   36 |   36 |    54|    52|    50|A

### Question 3
No