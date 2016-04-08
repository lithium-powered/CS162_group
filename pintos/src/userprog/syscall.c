#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"

#include "threads/thread.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "threads/malloc.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);

//Task3
struct lock filesys_globlock;
//void check_args(struct intr_frame *f, int n);
int ptr_check(const void *vaddr);
struct file* get_file_from_fd(int fd);
void exit(int status);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

struct fd_elem{ //struct for multiple open/closes
	int fd;
	struct file *file;
	struct list_elem elem;
};


void
syscall_init (void) 
{
  lock_init(&filesys_globlock); //Initialize Global Lock
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


static void syscall_handler (struct intr_frame *f UNUSED) 
{
  
  uint32_t* args = ((uint32_t*) f->esp);
  //printf("System call number: %d\n", args[0]);
  //printf ("%d\n", args[1]);

  if (args[0] == SYS_EXIT) {
    f->eax = args[1];
    check_args(f,1);
    printf("%s: exit(%d)\n", &thread_current ()->name, args[1]);
    thread_exit();
  }

  //Halt
  if (args[0] == SYS_HALT){
  	shutdown_power_off();
  }

  //Practice
  if (args[0] == SYS_PRACTICE){
  	int retval = args[1];
  	retval = retval + 1;
  	f->eax = retval;
  }


  //COMMENT ME OUT TO SPEED UP TESTING
  //Exec
  // if (args[0] == SYS_EXEC){
  // 	tid_t pid = process_execute(&args[1]);
  // 	if (pid==TID_ERROR){
  // 		f->eax = -1;
  // 	}
  // 	else{
  // 		f->eax = pid;
  // 	}
  // }
  //COMMENT ME OUT TO SPEED UP TESTING


  //Wait
  if (args[0] == SYS_WAIT){

  }


  //Task3
  if (args[0] == SYS_CREATE){
  	args[1] = ptr_check((const void *) args[1]);
  	f->eax = create((const char *)args[1], (unsigned) args[2]);
  }
  if (args[0] == SYS_REMOVE){
  	args[1] = ptr_check((const void *) args[1]);
  	f->eax = remove((const char *)args[1]);
  }
  if (args[0] == SYS_OPEN){
  	args[1] = ptr_check((const void *) args[1]);
  	f->eax = open((const char*) args[1]);
  }
  if (args[0] == SYS_FILESIZE){
  	f->eax = filesize(args[1]);
  }
  if (args[0] == SYS_READ){
  	args[2] = ptr_check((const void *) args[2]);
  	f->eax = read(args[1], (void *) args[2], (unsigned)args[3]);
  }
  if (args[0] == SYS_WRITE){
  	args[2] = ptr_check((const void *) args[2]);
  	f->eax = write(args[1], (const void *) args[2], (unsigned)args[3]);
  }
  if (args[0] == SYS_SEEK){
  	seek(args[1], (unsigned) args[2]);
  }
  if (args[0] == SYS_TELL){
  	f->eax = tell(args[1]);
  }
  if (args[0] == SYS_CLOSE){
  	close(args[1]);
  }
  
}

void check_args(struct intr_frame *f, int n){
 	int i;
   	for (i = 0; i <= n; i++){
   		int *ptr = (int *)f->esp +4*i;
   		//printf("%d\n",ptr);
   		//printf("%d\n", PHYS_BASE);
 	  	if (!is_user_vaddr(ptr) || ptr > PHYS_BASE){
 	  		exit(-1);
 	  	}
   	}
 }

int ptr_check(const void *vaddr){ //check for valid address
	if (is_user_vaddr(vaddr)){
		void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
		if (ptr){
			return (int) ptr;
		}
	}
	exit(-1);
}

struct file* get_file_from_fd(int fd){
	struct thread *t = thread_current();
	struct list_elem *i;
	for (i = list_begin(&t->fd_list); i != list_end(&t->fd_list); i = list_next(i)){
		struct fd_elem *fde = list_entry(i, struct fd_elem, elem);
		if (fd == fde->fd){
			return fde->file;
		}
	}
	return NULL;
}

void exit(int status){
	struct thread *t = thread_current();
	while (!list_empty (&t->fd_list)){
		struct list_elem *i = list_begin(&t->fd_list);
		close(list_entry(i, struct fd_elem, elem)->fd);
	}

    printf("%s: exit(%d)\n", (char *)&thread_current ()->name, status); //warning fix
    thread_exit();
}

bool create (const char *file, unsigned initial_size) {
	lock_acquire(&filesys_globlock);
	bool toReturn = filesys_create(file, initial_size);
	lock_release(&filesys_globlock);
	return toReturn;
}

bool remove (const char *file) {
	lock_acquire(&filesys_globlock);
	bool toReturn = filesys_remove(file);
	lock_release(&filesys_globlock);
	return toReturn;
}

int open (const char *file) {
	lock_acquire(&filesys_globlock);
	struct file *f = filesys_open(file);
	if (f){

		struct thread *t = thread_current();
		struct fd_elem *fde = (struct fd_elem*)malloc(sizeof(struct fd_elem));
		if (!fde){ //failed
			file_close(f);
			lock_release(&filesys_globlock);
			return -1;
		}
		fde->fd = t->fd;
		fde->file = f;
		t->fd = t->fd + 1; //open-twice test
		list_push_back(&t->fd_list, &fde->elem);

		lock_release(&filesys_globlock);
		return fde->fd;

	}else{
		lock_release(&filesys_globlock);
		return -1;
	}
}

int filesize (int fd) {
	lock_acquire(&filesys_globlock);
	struct file *f = get_file_from_fd(fd);
	if (f){
		int toReturn = file_length(f);
		lock_release(&filesys_globlock);
		return toReturn;
	}else{
		lock_release(&filesys_globlock);
		return -1;
	}
}

int read (int fd, void *buffer, unsigned size) {
	lock_acquire(&filesys_globlock);
	if (fd == STDIN_FILENO){ //edge case of STDIN
		uint8_t *buf = (uint8_t *) buffer;
		unsigned i;
		for (i = 0; i < size; i ++){
			buf[i] = input_getc();
		}
		lock_release(&filesys_globlock);
		return size;
	}
	struct file *f = get_file_from_fd(fd);
	if (f){
		int toReturn = file_read(f,buffer,size);
		lock_release(&filesys_globlock);
		return toReturn;
	}else{
		lock_release(&filesys_globlock);
		return -1;
	}
}

int write (int fd, const void *buffer, unsigned size) {
	lock_acquire(&filesys_globlock);
	if (fd == STDOUT_FILENO){ //edge case to STDOUT
		putbuf(buffer, size);
		lock_release(&filesys_globlock);
		return size;
	}
	struct file *f = get_file_from_fd(fd);
	if (f){
		int toReturn = file_write(f,buffer,size);
		lock_release(&filesys_globlock);
		return toReturn;
	}else{
		lock_release(&filesys_globlock);
		return -1;
	}
}

void seek (int fd, unsigned position) {
	lock_acquire(&filesys_globlock);
	struct file *f = get_file_from_fd(fd);
	if (f){
		file_seek(f, position);
	}
	lock_release(&filesys_globlock);
}

unsigned tell (int fd) {
	lock_acquire(&filesys_globlock);
	struct file *f = get_file_from_fd(fd);
	if (f){
		off_t toReturn = file_tell(f);
		lock_release(&filesys_globlock);
		return toReturn;
	}else{
		lock_release(&filesys_globlock);
		return -1;
	}
}

void close (int fd) {

	lock_acquire(&filesys_globlock);
	struct thread *t = thread_current();

	struct list_elem *i;

	for (i = list_begin(&t->fd_list); i != list_end(&t->fd_list); i = list_next(i)){
		struct fd_elem *fde = list_entry(i, struct fd_elem, elem);
		if (fd == fde->fd || fd == -1){
			file_close(fde->file);
			list_remove(&fde->elem);
			free(fde);
			if (fd != -1){
				lock_release(&filesys_globlock);
				return;
			}
		}
	}
	lock_release(&filesys_globlock);
	return;
}

