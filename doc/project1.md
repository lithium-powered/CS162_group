Design Document for Project 1: Threads
======================================

## Group Members

* Albert Weng <albertweng@berkeley.edu>
* Annie Lin <annie_lin@berkeley.edu>
* Liquin Yu <liyu@berkeley.edu>
* Nerissa Lin <lin.nerissa@berkeley.edu>

## Task 1: Timer Sleep

### Data Structures and Functions

```
struct list sleep_list;
list_init(&sleep_list);

struct sema_sleep_time{
	struct semaphore *sema;
	int64_t sleep time;
}

```
Creates a struct that holds a semaphore and how long the thread that created the semaphore should sleep.