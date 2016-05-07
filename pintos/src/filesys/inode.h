#ifndef FILESYS_INODE_H
#define FILESYS_INODE_H

#include <stdbool.h>
#include "filesys/off_t.h"
#include "devices/block.h"
#include "threads/thread.h"

struct bitmap;

/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk
  {
    block_sector_t start;               /* First data sector. */
    off_t length;                       /* File size in bytes. */
    unsigned magic;                     /* Magic number. */
    //uint32_t unused[125];               /* Not used. */
    block_sector_t direct[121];         
    block_sector_t indirect;
    block_sector_t doubleind;
    bool is_dir;
    block_sector_t parent;
  };

/* In-memory inode. */
struct inode 
  {
    struct list_elem elem;              /* Element in inode list. */
    block_sector_t sector;              /* Sector number of disk location. */
    int open_cnt;                       /* Number of openers. */
    bool removed;                       /* True if deleted, false otherwise. */
    int deny_write_cnt;                 /* 0: writes ok, >0: deny writes. */
    bool is_dir;
    block_sector_t parent;
    struct lock inode_lock;
  };


void inode_init (void);
bool inode_create (block_sector_t, off_t, bool);
struct inode *inode_open (block_sector_t);
struct inode *inode_reopen (struct inode *);
block_sector_t inode_get_inumber (struct inode *);
void inode_close (struct inode *);
void inode_remove (struct inode *);
off_t inode_read_at (struct inode *, void *, off_t size, off_t offset);
off_t inode_write_at (struct inode *inode, const void *, off_t size, off_t offset); 
//off_t inode_write_at (struct inode *, const void *, off_t size, off_t offset);
void inode_deny_write (struct inode *);
void inode_allow_write (struct inode *);
off_t inode_length ( struct inode *);

bool inode_is_dir ( struct inode *);
block_sector_t inode_get_parent( struct inode *);
bool inode_add_parent(block_sector_t parent, block_sector_t child);
void lock_inode( struct inode *inode);
void lock_release_inode( struct inode *inode);
/* Added */
#define CACHE_SIZE 64
#define CLOCK_CHANCES 3

/* struct of an element inside the cache */
struct cache_elem
  {
    block_sector_t sector;          /* Sector number of disk location */
    bool dirty;                     /* True if dirty block, false o/w */
    bool empty;						/* True if slot does not contain block */
    uint32_t chances;				/* Chances used for clock alg */
    char data[BLOCK_SECTOR_SIZE];   /* block data. */
    struct lock lock;				/* Lock on the cache element */
  };

void cache_init(struct cache_elem **);
void wipe_cache_elem(struct cache_elem *);

void cache_read(block_sector_t, void *buffer, int, int);
void cache_write(block_sector_t, const void *buffer, int, int);
int next_free_cache_slot(void);
bool inode_resize(struct inode_disk *, off_t, block_sector_t);

/* Added */
struct cache_elem *cache[CACHE_SIZE];
uint32_t clock_hand;


 /* ***** */

#endif /* filesys/inode.h */
