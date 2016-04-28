# Project 3 Design Document


## Task 1:

### Data Structures and Functions:

Add a global cache object 
```
//initialized in inode.c
struct cache_elem cache[64];
```

```
//defined in inode.h
struct cache_elem
  {
    block_sector_t sector;          /* Sector number of disk location */
    bool dirty;                     /* True if dirty block, false o/w */
    bool empty;						/* True if slot does not contain block */
    struct lock cache_elem_lock;	/* Lock on the cache element */
    Char data[BLOCK_SECTOR_SIZE];   /* block data. */
    uint32_t chances				/* Chances used for clock alg */
  };
```

Function to replace block_read, same parameters. It will check the cache and try to pull data from there before calling actual block_read function.
```
//defined in inode.c
buffered_write (sector_idx)
```

Function to replace block_write.
```
//defined in inode.c
buffered_write (sector_idx);
```

Finds a space in the cache and (also evicts a block if have to) stores a block into it.
```
Int cache_block();
```

### Algorithms:
We first malloc memory for an array which holds 64 cache_elem. We then use this list as the cache. We replace block_read and block_write with our own functions that will check the cache for blocks before going into disk to get them. We should have a lock for each individual cached block so that other processes can?t access the block when it is being written to. We also implement write back by only writing to disk when we evict a block from the cache.

### Synchronization:
We obviously need synchronization for the cache. We will need locks on the blocks to make sure 2 threads aren't writing to the block at the same time.

Put a lock on the entire cache (will think about using more efficient implementation later).

### Rationale:
Can't have a lock on the whole cache or else there will be performance issues. Next best thing is to lock on cache elements.


## Task 2:

Data structure and functions:
Change inode_disk struct.
```
struct inode_disk
  {
    block_sector_t start;               /* First data sector. */
    off_t length;                       /* File size in bytes. */
    unsigned magic;                     /* Magic number. */
    block_sector_t direct[123];			/* direct block pointers */
    block_sector_t indirect;			/* indirect block pointer */
    block_sector_t double;				/* double indirect block pointer */
  };
```

### Algorithms:
We use the unused space in inode_disk to store additional block_sector_t to directly and indirectly point to blocks.

### Synchronization:
We have to worry about concurrent writes to inode, so we will need a lock for this situation when two threads try to write to it.

### Rationale:
Following the structure given in lecture seems to be the easiest way to implement task 2. If we configure reads and writes correctly, we can hide how the filesystem is actually doing these calls.


## Task 3: Subdirectories

### Data Structures and Functions:

In inode.c (inode_disk and inode structs):
```
  bool is_dir/file;  // if inode is a directory or a file
  block_sector_t parent;  //  Parent inode sector number

  (for just inode struct)
  struct lock dir_lock; // Lock for each inode for synchronization
```
In thread.c:
```
  struct dir* curr_dir; // The thread's current working directory (src/filesys/directory.c) to set it based of our algorithm.
```

### Algorithms:
A path is broken up into tokens, with each separated by a '/'. If the first
character of the path is '/', then the root is the current tracked directory. 
Else, if the first token of the path is '.', then the current tracked
directory is the current thread's working directory. Else. if the first token is '..',
then the current tracked directory is its parent. 

If none of the above then it is a normal name and it is looked up.
If the token exists and is a directory, then this becomes the current tracked directory.
If the name exists and is a file, it must be the last token in the path.
Otherwise, the path is invalid unless we are doing create, in which we make that the name
of the new file. 

### Synchronization: 
We make sure to lock operations per directory, so that adding or removing from a directory requires the inode lock for that directory.  Each directory will have its own inode lock.

### Rationale:
We need to parse the file name to get the right directory. It makes sense that only one thread can access a directory at a time to prevent conflicts. However, multiple threads should be allowed to access multiple directories. We also need to account for local paths to file system calls, which we do by using the current directory as the start of our search.

## Additional Questions

### write-behind:

- Use a cache.
Iterating through the cache, if an entry is dirty, it is written back to
disk and dirty is switched to false. Done every constant number of cycles (we would choose this number based on how often we want to iterate through and write-back, tradeoff is that doing this more often saves more data in the event of a crash but the operation is too costly to do too often). So if we are 
writing a small number of bytes to one sector in a file, then they are first put in cache
and written to disk all at once in a certain period of time.

### read-ahead:

- Use a cache.
Spawn an additional thread when a block was retrieved from the
cache to get the next data block sector number from the
inode and put it into the cache. Thus the process doesn't need to wait on disk for every
time it needs a new sector.


###To do:
1. Remove global lock