Final Report for Project 1: Threads
===================================

#162 Project 1 Final Report
  
##Design Changes
 
###Part 1:
+ Originally had planned to implement everything using semaphores including the sema up waking and sema down sleeping function. During the design, had many of the variables set inside a semaphore struct that was custom made for the purpose of the timer_sleep function
+ Decided instead to change the implementation so that things would run more smoothly
+ Used Timer Sleep to initiate a sleep time
+ Assuming that every thread will have the option to sleep should it want to
+ Set the sleep time value to 0 until it needed to sleep where it would be set to some point in the future
+ Thread_Yield now handles the placing of thread on either ready or sleep list
+ Used a regularly scheduled function within scheduler to prune the sleep list and move ready to wake threads to the ready thread and let the normal scheduling functions schedule

###Part 2:
+ Added a donate method - we added this method to encapsulate all the updates that need to be done when donating priority. 
+ Added an undonate method - In addition to the donate method we had designed to handle donating to threads, we decided to make an undonate function to reverse the changes and clean up the donor list when a thread has passed lock(s) on to the donor and no longer needs to adopt the donor’s priority value. Using this function kept our code modular and abstracted, so we could call undonate from other functions whenever we needed to clear off an outdated donation. This was probably the hardest method to implement because there were many unexpected things from from the semaphore->waiters. We had to add many checks to make sure that nested donations and chaining were working correctly.
+ We had to change the implementation of the less_than_func functions that we had to write because we were dereferencing the wrong values and getting gibberish values for our threats. Took us a while to catch this.
+ Our first implementation of undonate had the undonate function inside the semaphore up function instead of lock release. This created a lot of problems because we were catching semaphores that were not locks. We finally moved the undonate function to lock release and added in a couple of sanity checks in-order to finish the implementation.
 

###Part 3:
+ Changed data structure for ready threads - We originally believed that the spec required us to use an array of 64 priority queues (where each queue holds all threads of a specific priority value and threads move to different queues when their priority values are updated). Upon speaking to Andrew, we came to understand that we only needed to mimic the behavior of an array of 64 priority queues and could use some more efficient data structures as long as we maintained the correct priority values for all threads. Because it made less sense to maintain a bunch of priority queues when we could just use a single sorted list, we opted to reuse the existing ready_threads list as a linked list of threads that are sorted based on their priority values, which we compute according to the mlfqs rules.
+ Changed storage method for ready threads list - We originally intended for the list of ready threads to be a list of mlfqs_list_elem structs that each held a pointer to a thread and a priority value for that thread. We realized it would be much easier implementation-wise to simply store pointers to threads in the list and have the priority value be an instance variable inside the thread class. This was easier to maintain and prevented confusion because we no longer needed a mlfqs_list_elem struct to act as the connector between priority values and their respective threads.
 
                                 	                                                             	
## Reflection
 
Albert: 
+ Co-coded part 3
+ Involved in debugging part 3 and 1
+ Implemented Part 3 for the first design

Annie:
+ Contributed code to part 3
+ Involved in debugging part 3
+ Implemented Part 3 for the design

Nerissa: 
+ Implemented and tested Part 1 to completion
+ Aided in using GDB to debug heavily and assist in debugging part 2 and some of part 3 
+ Made pizza for the fam
 
Li:
+ Structured the implementation of Part 1 and 2.
+ Wrote majority of the code for part 2.
+ Ate hot pockets
+ Cried while debugging
 
 
What went well:
+ Debugging together, usually caught the harder bugs when we debugged together especially when it was with someone unfamiliar with the code just developed
+  Distribution of work was extremely even and everyone worked well together to get things done
+ Helped each other understand the project off the bat and continued to build a strong group understanding

What could be improved:
+ The group should meet up earlier and more frequently to code together.
+ Better documentation of the decisions made that weren’t in line with the original design
+ Having other group members do sanity checks is helpful and we should have done it more often.
+ Everyone needs to develop a deeper understanding of each part of the project instead of planning and implementing their own parts
+ Need to start earlier
+ Need to maintain better merging and communication about changes in functions that affect multiple parts of the project (We had a bug that stemmed from repeated lines from a bad merge)
+ Need to become better at using GDB
+ Make sure that implementations are theoretically correct when debugging - be careful of passing tests by coincidence? This can be really misleading

