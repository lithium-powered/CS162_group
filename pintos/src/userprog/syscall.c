#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  uint32_t* args = ((uint32_t*) f->esp);
  //printf("System call number: %d\n", args[0]);
  if (args[0] == SYS_EXIT) {
    f->eax = args[1];
    printf("%s: exit(%d)\n", &thread_current ()->name, args[1]);
    thread_exit();
  }

  //Write (needed to pass tests)
  else if (args[0] == SYS_WRITE){
  	int fd = args[1];
  	void *buf = args[2];
  	unsigned size = args[3];
  	if (fd == STDOUT_FILENO){
  		putbuf(args[2],args[3]);
  	}
  }

  //Halt
  else if (args[0] == SYS_HALT){
  	shutdown_power_off();
  }

  //Practice
  else if (args[0] == SYS_PRACTICE){
  	int retval = args[1];
  	retval = retval + 1;
  	f->eax = retval;
  }

  //Exec
  else if (args[0] == SYS_EXEC){
  	tid_t pid = process_execute(&args[1]);
  	if (pid==TID_ERROR){
  		f->eax = -1;
  	}
  	else{
  		f->eax = pid;
  	}
  }

  //Wait
  else if (args[0] == SYS_WAIT){

  }
  
}
