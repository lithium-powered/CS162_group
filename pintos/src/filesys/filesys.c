#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "threads/thread.h"

/* Partition that contains the file system. */
struct block *fs_device;
struct lock globalCacheLock;

static void do_format (void);

struct dir* containing_dir(const char* path);
char* get_name(const char* path);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format) 
{
  fs_device = block_get_role (BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC ("No file system device found, can't initialize file system.");

  lock_init(&globalCacheLock);
  cache_init((struct cache_elem **) &cache);
  clock_hand = CACHE_SIZE - 1;

  inode_init ();
  free_map_init ();

  if (format) 
    do_format ();

  free_map_open ();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void) 
{
  int i;
  for(i = 0; i < CACHE_SIZE; i++){
    if(!cache[i]->empty && (cache[i]->dirty)){
      //do we need to do a lock check? in case of write?
      block_write (fs_device, cache[i]->sector, cache[i]->data);
      free(cache[i]);
    }
  } 
  free_map_close ();
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name, off_t initial_size, bool is_dir) 
{
  block_sector_t inode_sector = 0;
  struct dir *dir = containing_dir(name);
  char* file_name = get_name(name);
  //printf(name);
  bool success = false;
  if (strcmp(file_name, ".") != 0 && strcmp(file_name, "..") != 0){
      success = (dir != NULL
                  && free_map_allocate (1, &inode_sector)
                  && inode_create (inode_sector, initial_size, is_dir)
                  && dir_add (dir, file_name, inode_sector));
  }
  if (!success && inode_sector != 0) 
    free_map_release (inode_sector, 1);
  dir_close (dir);
  free(file_name);
  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *name)
{
  if (strlen(name) == 0){
    return NULL;
  }
  struct dir *dir = containing_dir(name);
  char* file_name = get_name(name);
  struct inode *inode = NULL;

  if (dir != NULL){
      if (strcmp(file_name, "..") == 0){
        if (!dir_parent(dir, &inode))
          {
            free(file_name);
            return NULL;
          }
      }else if ((dir_is_root(dir) && strlen(file_name) == 0) || strcmp(file_name, ".") == 0){
        
        free(file_name);
        return (struct file *) dir;

      }else{
        dir_lookup (dir, file_name, &inode);
      }
  }

  dir_close (dir);
  free(file_name);

  if (!inode)
    {
      return NULL;
    }

  if (inode_is_dir(inode))
    {
      return (struct file *) dir_open(inode);
    }
  return file_open (inode);
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *name) 
{
  struct dir *dir = containing_dir(name);
  char* file_name = get_name(name);
  bool success = dir != NULL && dir_remove (dir, file_name);
  dir_close (dir); 
  free(file_name);

  return success;
}

/* Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}

struct dir* containing_dir (const char* path)
{ 
  char s[strlen(path) + 1];
  memcpy(s, path, strlen(path) + 1);
  struct dir* dir;
  if (path[0] == 47 || !thread_current()->cur_dir){
      dir = dir_open_root();
    }else{
      dir = dir_reopen(thread_current()->cur_dir);
    }

  char *save_ptr, *next_token = NULL, *token = strtok_r(s, "/", &save_ptr);
  if (token){
      next_token = strtok_r(NULL, "/", &save_ptr);
  }
  while (next_token != NULL){
    if (strcmp(token, ".") == 0){
      continue;
    }else{
      struct inode *inode;
      if (strcmp(token, "..") == 0){
        if (!dir_parent(dir, &inode)){
          return NULL;
        }
      }else{
        if (!dir_lookup(dir, token, &inode)){
          return NULL;
        }
      }

      //open next directory
      if (inode_is_dir(inode)){
        dir_close(dir);
        dir = dir_open(inode);
      }else{
        inode_close(inode);
      }
    }


    token = next_token;
    next_token = strtok_r(NULL, "/", &save_ptr);
  }
  return dir;
}

bool filesys_chdir (const char* name)
{
  struct dir* dir = containing_dir(name);
  char* file_name = get_name(name);
  struct inode *inode = NULL;

  if (dir != NULL){
      if (strcmp(file_name, "..") == 0){
        //check parent
        if (!dir_parent(dir, &inode)){
            free(file_name);
            return false;
          }
      }
      else if ((strlen(file_name) == 0 && (dir_is_root(dir))) || (strcmp(file_name, ".") == 0)){
        thread_current()->cur_dir = dir;
        free(file_name);
        return true;
      }else{
        dir_lookup (dir, file_name, &inode);
      }
  }

  dir_close (dir);
  free(file_name);

  dir = dir_open(inode);
  if (dir)
    {
      dir_close(thread_current()->cur_dir);
      thread_current()->cur_dir = dir;
      return true;
    }
  return false;
}

char* get_name (const char* path)
{
  char s[strlen(path) + 1];
  memcpy(s, path, strlen(path) + 1);

  char *token, *save_ptr, *prev_token = "";
  for (token = strtok_r(s, "/", &save_ptr); token != NULL;
       token = strtok_r (NULL, "/", &save_ptr))
    {

      prev_token = token;
    }
  char *file_name = malloc(strlen(prev_token) + 1);
  memcpy(file_name, prev_token, strlen(prev_token) + 1);
  return file_name;
}
