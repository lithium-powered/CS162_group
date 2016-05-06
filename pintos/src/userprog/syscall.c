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

#include "filesys/inode.h"
#include "userprog/process.h"


//This is the node for the children linked list that each parent thread stores

struct child{
  struct list_elem elem;
  struct semaphore wait;
  int status;
  tid_t child_id;
  int memory;
  struct lock memory_lock;
};


static void syscall_handler (struct intr_frame *);

void check_args(struct intr_frame *f, int n);
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

struct fd_elem* get_fd_elem_from_fd(int fd);
bool chdir (const char* dir);
bool mkdir (const char* dir);
bool readdir (int fd, char* name);
bool isdir (int fd);
int inumber (int fd); 

struct fd_elem{ //struct for multiple open/closes
    int fd;
    struct file *file;
    struct list_elem elem;

    struct dir *dir;
    bool is_dir;
};


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


static void syscall_handler (struct intr_frame *f UNUSED) 
{
  
  uint32_t* args = ((uint32_t*) f->esp);


  if (args[0] == SYS_EXIT) {
    f->eax = args[1];
    check_args(f,1);
    printf("%s: exit(%d)\n", &thread_current ()->name, args[1]);
    struct thread *t = thread_current();
    //Close all files this thread currently has open
    while (!list_empty (&t->fd_list)){
        struct list_elem *i = list_begin(&t->fd_list);
        close(list_entry(i, struct fd_elem, elem)->fd);
    }

    thread_exit(args[1]);
  }

  //Halt
  if (args[0] == SYS_HALT){
    shutdown_power_off();
  }

  //Practice
  if (args[0] == SYS_PRACTICE){
    check_args(f,1);
    int retval = args[1];
    retval = retval + 1;
    f->eax = retval;
  }

  //Exec
if (args[0] == SYS_EXEC){
    check_args(f,1);
    tid_t pid = process_execute(args[1]);
    if (pid==TID_ERROR){
        f->eax = -1;
    }
    else{
        f->eax = pid;
    }
    }

  //Wait
  if (args[0] == SYS_WAIT){
    check_args(f,1);
    int status = process_wait(args[1]);
    f->eax = status;
  }

  //Filesys - Create
  if (args[0] == SYS_CREATE){
    check_args(f,2);
    args[1] = ptr_check((const void *) args[1]);
    f->eax = create((const char *)args[1], (unsigned) args[2]);
  }

  //Filesys - Remove
  if (args[0] == SYS_REMOVE){
    check_args(f,1);
    args[1] = ptr_check((const void *) args[1]);
    f->eax = remove((const char *)args[1]);
  }

  //Filesys - Open
  if (args[0] == SYS_OPEN){
    check_args(f,1);
    args[1] = ptr_check((const void *) args[1]);
    f->eax = open((const char*) args[1]);
  }

  //Filesys - Filesize
  if (args[0] == SYS_FILESIZE){
    check_args(f,1);
    f->eax = filesize(args[1]);
  }

  //Filesys - Read
  if (args[0] == SYS_READ){
    check_args(f,3);
    args[2] = ptr_check((const void *) args[2]);
    f->eax = read(args[1], (void *) args[2], (unsigned)args[3]);
  }

  //Filesys - Write
  if (args[0] == SYS_WRITE){
    check_args(f,3);
    args[2] = ptr_check((const void *) args[2]);
    f->eax = write(args[1], (const void *) args[2], (unsigned)args[3]);
  }

  //Filesys - Seek
  if (args[0] == SYS_SEEK){
    check_args(f,2);
    seek(args[1], (unsigned) args[2]);
  }

  //Filesys - Tell
  if (args[0] == SYS_TELL){
    check_args(f,1);
    f->eax = tell(args[1]);
  }

  //Filesys - Close
  if (args[0] == SYS_CLOSE){
    check_args(f,1);
    close(args[1]);
  }

  if (args[0] == SYS_CHDIR){
    check_args(f,1);
    args[1] = ptr_check((const void *) args[1]);
    f->eax = chdir((const char*) args[1]);
  }

  if (args[0] == SYS_MKDIR){
    check_args(f,1);
    args[1] = ptr_check((const void *) args[1]);
    f->eax = mkdir((const char*) args[1]);
  }

  if (args[0] == SYS_READDIR){
    check_args(f,2);
    args[2] = ptr_check((const void *) args[2]);
    f->eax = readdir(args[1], (char*) args[2]);
  }

  if (args[0] == SYS_ISDIR){
    check_args(f,1);
    f->eax = isdir(args[1]);
  }

  if (args[0] == SYS_INUMBER){
    check_args(f,2);
    f->eax = inumber(args[1]);
  }
  
}


//Sanity check if arguments are valid
void check_args(struct intr_frame *f, int n){
    int i;
    for (i = 0; i <= n; i++){
        int *ptr = (int *)f->esp +4*i;
        if (!is_user_vaddr(ptr) || ptr > PHYS_BASE){
            exit(-1);
        }
    }
 }

//Sanity check if pointer points to valid address
int ptr_check(const void *vaddr){
    if (is_user_vaddr(vaddr)){
        void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
        if (ptr){
            return (int) ptr;
        }
    }
    exit(-1);
}

//Get a file struct from the int fd
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

struct fd_elem* get_fd_elem_from_fd(int fd){
    struct thread *t = thread_current();
    struct list_elem *i;
    for (i = list_begin(&t->fd_list); i != list_end(&t->fd_list); i = list_next(i)){
        struct fd_elem *fde = list_entry(i, struct fd_elem, elem);
        if (fd == fde->fd){
            return fde;
        }
    }
    return NULL;
}

//Exit if a pointer or argument check fails
void exit(int status){
    struct thread *t = thread_current();
    while (!list_empty (&t->fd_list)){
        struct list_elem *i = list_begin(&t->fd_list);
        close(list_entry(i, struct fd_elem, elem)->fd);
    }

    printf("%s: exit(%d)\n", (char *)&thread_current ()->name, status);
    thread_exit(status);
}

//Create a new file
bool create (const char *file, unsigned initial_size) {
    
    bool toReturn = filesys_create(file, initial_size, false);
    
    return toReturn;
}


//Remove an existing file
bool remove (const char *file) {
    
    bool toReturn = filesys_remove(file);
    
    return toReturn;
}

int open (const char *file) {
    
    struct file *f = filesys_open(file);
    if (f){

        struct thread *t = thread_current();
        struct fd_elem *fde = (struct fd_elem*)malloc(sizeof(struct fd_elem));
        if (!fde){ //failed
            file_close(f);
            
            return -1;
        }
        fde->fd = t->fd;
        if (!inode_is_dir(file_get_inode(f))){
          fde->file = f;
        }else{
          fde->dir = (struct dir *) f;
        }

        t->fd = t->fd + 1; //open-twice test
        list_push_back(&t->fd_list, &fde->elem);

        return fde->fd;

    }else{
        
        return -1;
    }
}

//Get the size of the file
int filesize (int fd) {
    
    struct file *f = get_file_from_fd(fd);
    if (f){
        int toReturn = file_length(f);
        
        return toReturn;
    }else{
        
        return -1;
    }
}

//Read from the file
int read (int fd, void *buffer, unsigned size) {
    
    if (fd == STDIN_FILENO){ //edge case to STDIN
        uint8_t *buf = (uint8_t *) buffer;
        unsigned i;
        for (i = 0; i < size; i ++){
            buf[i] = input_getc();
        }
        
        return size;
    }
    struct file *f = get_file_from_fd(fd);
    if (f){
        int toReturn = file_read(f,buffer,size);
        
        return toReturn;
    }else{
        
        return -1;
    }
}

//Write to the file
int write (int fd, const void *buffer, unsigned size) {
    
    if (fd == STDOUT_FILENO){ //edge case to STDOUT
        putbuf(buffer, size);
        
        return size;
    }
    struct file *f = get_file_from_fd(fd);
    if (f){
        int toReturn = file_write(f,buffer,size);
        
        return toReturn;
    }else{
        
        return -1;
    }
}


//Seek a position in the file
void seek (int fd, unsigned position) {
    
    struct file *f = get_file_from_fd(fd);
    if (f){
        file_seek(f, position);
    }
    
}

//Return the position of the next file or byte to be written
unsigned tell (int fd) {
    
    struct file *f = get_file_from_fd(fd);
    if (f){
        off_t toReturn = file_tell(f);
        
        return toReturn;
    }else{
        
        return -1;
    }
}

//Close the file referenced by the int fd
void close (int fd) {

    
    struct thread *t = thread_current();

    struct list_elem *i;

    for (i = list_begin(&t->fd_list); i != list_end(&t->fd_list); i = list_next(i)){
        struct fd_elem *fde = list_entry(i, struct fd_elem, elem);
        if (fde && fd == fde->fd){
            if (fde->file){
                file_close(fde->file);
            }
            list_remove(&fde->elem);
            free(fde);
            break;
        }
    }
    
    return;
}

bool chdir (const char* dir)
{
  return filesys_chdir(dir);
}

bool mkdir (const char* dir)
{
  return filesys_create(dir, 0, true);
}

bool readdir (int fd, char* name)
{
  struct fd_elem *f = get_fd_elem_from_fd(fd);
  if (!f)
    {
      return false;
    }
  if (!f->is_dir)
    {
      return false;
    }
  if (!dir_readdir(f->dir, name))
    {
      return false;
    }
  return true;
}

bool isdir (int fd)
{
  struct fd_elem *f = get_fd_elem_from_fd(fd);
  if (!f)
    {
      return -1;
    }
  return f->is_dir;
}

int inumber (int fd)
{
  struct fd_elem *f = get_fd_elem_from_fd(fd);
  if (!f)
    {
      return -1;
    }
  block_sector_t inumber;
  if (f->is_dir)
    {
      inumber = inode_get_inumber(dir_get_inode(f->dir));
    }
  else
    {
      inumber = inode_get_inumber(file_get_inode(f->file));
    }
  return inumber;
}
