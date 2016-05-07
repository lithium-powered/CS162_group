#ifndef FILESYS_FILESYS_H
#define FILESYS_FILESYS_H

#include <stdbool.h>
#include "filesys/off_t.h"
#include "threads/thread.h"
#include "threads/malloc.h"

/* Sectors of system file inodes. */
#define FREE_MAP_SECTOR 0       /* Free map file inode sector. */
#define ROOT_DIR_SECTOR 1       /* Root directory file inode sector. */

/* Block device that contains the file system. */
struct block *fs_device;
struct lock globalCacheLock;

void filesys_init (bool);
void filesys_done (void);
bool filesys_create (const char *, off_t, bool);
struct file *filesys_open (const char *);
bool filesys_remove (const char *);
bool filesys_chdir (const char*);

#endif /* filesys/filesys.h */
