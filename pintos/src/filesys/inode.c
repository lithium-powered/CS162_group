#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"
#include <stdio.h>

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44

/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk
  {
    block_sector_t start;               /* First data sector. */
    off_t length;                       /* File size in bytes. */
    unsigned magic;                     /* Magic number. */
    //uint32_t unused[125];               /* Not used. */
    block_sector_t direct[123];         
    block_sector_t indirect;
    block_sector_t doubleind;
  };

/* Returns the number of sectors to allocate for an inode SIZE
   bytes long. */
static inline size_t
bytes_to_sectors (off_t size)
{
  return DIV_ROUND_UP (size, BLOCK_SECTOR_SIZE);
}

/* In-memory inode. */
struct inode 
  {
    struct list_elem elem;              /* Element in inode list. */
    block_sector_t sector;              /* Sector number of disk location. */
    int open_cnt;                       /* Number of openers. */
    bool removed;                       /* True if deleted, false otherwise. */
    int deny_write_cnt;                 /* 0: writes ok, >0: deny writes. */
  //  struct inode_disk data; //Part 2 removed this
  };


bool inode_resize(struct inode_disk *id, off_t size, block_sector_t origsector) {
  //printf("resizing %d",size);
  block_sector_t indirectaddr = id->indirect;

  block_sector_t sector;
  off_t ofs;
  block_sector_t writer;
  int i = 0;
  while (i<123){
    if (size <= 512 * i && id->direct[i] != 0) {
      free_map_release(id->direct[i],BLOCK_SECTOR_SIZE);
      writer = origsector;
      ofs = sizeof(block_sector_t)+sizeof(off_t)+sizeof(unsigned)+i*sizeof(block_sector_t);
      cache_write(writer, 0, 1, ofs);
      //id->direct[i] = 0;
    }
    if (size > 512 * i && id->direct[i] == 0) {
      if (free_map_allocate(1,&sector)==0){
        inode_resize(id, id->length, origsector);
        return false;        
      }
      writer = origsector;
      ofs = sizeof(block_sector_t)+sizeof(off_t)+sizeof(unsigned)+i*sizeof(block_sector_t);
      cache_write(writer, &sector, sizeof(sector), ofs);
    }
    i++;
  }


  if (id->indirect == 0 && size <= 123 * 512) {
    return true;
  }
  block_sector_t buffer[128];
  if (id->indirect == 0) {
    memset(buffer, 0, 512);
    if (free_map_allocate(1,&sector)==0){
      inode_resize(id, id->length, origsector);
      return false;
    }
    writer = origsector;
    indirectaddr = sector;
    cache_write(writer, &sector, sizeof(sector), sizeof(off_t)+sizeof(unsigned)+123*sizeof(block_sector_t));
  }
  else {
    cache_read (id->indirect, &buffer, 128, 0);
    //block_read(id->indirect, buffer);
  }
  i = 0;
  while (i<128){
    if ((size <= (123 + i) * 512) && buffer[i] != 0) {
      free_map_release(buffer[i],BLOCK_SECTOR_SIZE);
      buffer[i] = 0;
    }
    if ((size > (123 + i) * 512) && buffer[i] == 0) {
      if (free_map_allocate(1, &sector)==0){
        inode_resize(id, id->length, origsector);
        return false;
      }
      buffer[i] = sector;
    }
    i++;
  }
  cache_write(indirectaddr, &buffer, sizeof(buffer), 0);
  //block_write(id->indirect, buffer);
  ofs = sizeof(block_sector_t);
  cache_write(origsector, &size, sizeof(size),ofs);
  //id->length = size;





/*
  if (id->doubleind == 0 && size <= 123 * 512 + 512 * 512 / 4) {
    return true;
  }

  if (id->doubleind == 0) {
    memset(buffer, 0, 512);
    if (free_map_allocate(1,&sector)==0){
      inode_resize(id, id->length, origsector);
      return false;
    }
    writer = origsector;
    cache_write(writer, &sector, sizeof(sector), sizeof(off_t)+sizeof(unsigned)+123*sizeof(block_sector_t)+sizeof(block_sector_t));
  }
  else {
    cache_read (id->doubleind, &buffer, 128, 0);
    //block_read(id->indirect, buffer);
  }
  i = 0;
  while (i<128){
    if ((size <= 123 * 512 + 512 * i) && buffer[i] != 0) {
      free_map_release(buffer[i],BLOCK_SECTOR_SIZE);
      buffer[i] = 0;
    }
    if ((size > (123 + i) * 512) && buffer[i] == 0) {
      if (free_map_allocate(1, &sector)==0){
        inode_resize(id, id->length, origsector);
        return false;
      }
      buffer[i] = sector;
    }
    i++;
  }
  cache_write(id->indirect, &buffer, sizeof(buffer), 0);
  //block_write(id->indirect, buffer);
  ofs = sizeof(block_sector_t);
  cache_write(origsector, &size, sizeof(size),ofs);



*/

  return true;
}



/* Returns the block device sector that contains byte offset POS
   within INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static block_sector_t
byte_to_sector (const struct inode *inode, off_t pos) 
{
  ASSERT (inode != NULL);
  struct inode_disk data;
  cache_read (inode->sector, &data, BLOCK_SECTOR_SIZE, 0);
  
  if (pos<512*123){
    //printf("small");
    int index = (int)(double)(int)(pos/512);
    //printf("small index is %"PRDSNu"",data.direct[index]);
    return data.direct[index];
  }
  if (pos<(512*123)+(512*512/4)){
    //printf("med");
    int index = (int)(double)(int)((pos-512*123)/512);
    block_sector_t indirect[128];
    cache_read (data.indirect, &indirect, 128, 0);
    return indirect[index];
  }
  /*if (pos<(512*123)+(512*512/4)+(128*128*512)){
    //printf("big");
    int index1 = (int)(double)(int)((pos-(512*123)-(512*512/4))/(512*128));
    int index2 = (int)(double)(int)((pos-(512*123)-(512*512/4)-(index1*512*128))/512);
    block_sector_t doubleind[128];
    cache_read (data.doubleind, &doubleind, 128, 0); 
    block_sector_t finalind[128];
    cache_read (doubleind[index1], &finalind, 128, 0);
    return finalind[index2];
  }*/
  //out of bounds of file

  return -1;


/*  if (pos<512*123){
    printf("pos is : %d",pos);
    int index = (int)(double)(int)(pos/512);
    printf("index is : %d",index);
    printf("direct is %s",data.direct[0]);
    
    free_map_allocate(1,data.direct[index]);
    return data.direct[index];
  }*/


/*  ASSERT (inode != NULL);
  
  if (pos < data.length)
    return data.start + pos / BLOCK_SECTOR_SIZE;
  else
    return -1;*/
}

/* List of open inodes, so that opening a single inode twice
   returns the same `struct inode'. */
static struct list open_inodes;

/* Added */
struct cache_elem *cache[CACHE_SIZE];
uint32_t clock_hand;


/* Initializes the inode module. */
void
inode_init (void) 
{
  list_init (&open_inodes);
}

/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   device.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
bool
inode_create (block_sector_t sector, off_t length)
{
  struct inode_disk *disk_inode = NULL;
  bool success = false;

  ASSERT (length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT (sizeof *disk_inode == BLOCK_SECTOR_SIZE);

  disk_inode = calloc (1, sizeof *disk_inode);
  if (disk_inode != NULL)
    {
      size_t sectors = bytes_to_sectors (length);
      disk_inode->length = length;
      disk_inode->magic = INODE_MAGIC;
      /*if (free_map_allocate (sectors, &disk_inode->start)) 
        {
          cache_write(sector, disk_inode, BLOCK_SECTOR_SIZE, 0);
          if (sectors > 0) 
            {
              static char zeros[BLOCK_SECTOR_SIZE];
              size_t i;
              
              for (i = 0; i < sectors; i++) 
                cache_write (disk_inode->start + i, zeros, BLOCK_SECTOR_SIZE, 0);
            }
          success = true; 
        } 
      free (disk_inode);*/
      disk_inode->indirect = 0;
      disk_inode->doubleind = 0;
      memset(disk_inode->direct,0,123);

      success = true;
      cache_write(sector, disk_inode, BLOCK_SECTOR_SIZE, 0);
      inode_resize(disk_inode, length, sector);
      free (disk_inode);
    }
  return success;
}

/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode *
inode_open (block_sector_t sector)
{
  struct list_elem *e;
  struct inode *inode;

  /* Check whether this inode is already open. */
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e)) 
    {
      inode = list_entry (e, struct inode, elem);
      if (inode->sector == sector) 
        {
          inode_reopen (inode);
          return inode; 
        }
    }

  /* Allocate memory. */
  inode = malloc (sizeof *inode);
  if (inode == NULL)
    return NULL;

  /* Initialize. */
  list_push_front (&open_inodes, &inode->elem);
  inode->sector = sector;
  inode->open_cnt = 1;
  inode->deny_write_cnt = 0;
  inode->removed = false;
  struct inode_disk data;
  cache_read (inode->sector, &data, BLOCK_SECTOR_SIZE, 0);
  return inode;
}

/* Reopens and returns INODE. */
struct inode *
inode_reopen (struct inode *inode)
{
  if (inode != NULL)
    inode->open_cnt++;
  return inode;
}

/* Returns INODE's inode number. */
block_sector_t
inode_get_inumber (const struct inode *inode)
{
  return inode->sector;
}

/* Closes INODE and writes it to disk.
   If this was the last reference to INODE, frees its memory.
   If INODE was also a removed inode, frees its blocks. */
void
inode_close (struct inode *inode) 
{
  /* Ignore null pointer. */
  if (inode == NULL)
    return;

  /* Release resources if this was the last opener. */
  if (--inode->open_cnt == 0)
    {
      /* Remove from inode list and release lock. */
      list_remove (&inode->elem);
 
      /* Deallocate blocks if removed. */
      if (inode->removed){ 
        /*
          struct inode_disk data;
          cache_read (inode->sector, &data, BLOCK_SECTOR_SIZE, 0);
          free_map_release (inode->sector, 1);
          free_map_release (data.start,
                            bytes_to_sectors (data.length)); 
        */
        struct inode_disk data;
        cache_read (inode->sector, &data, BLOCK_SECTOR_SIZE, 0);
        inode_resize(&data, 0, inode->sector);
        free (inode);
      }
    }
}

/* Marks INODE to be deleted when it is closed by the last caller who
   has it open. */
void
inode_remove (struct inode *inode) 
{
  ASSERT (inode != NULL);
  inode->removed = true;
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t
inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset) 
{
  uint8_t *buffer = buffer_;
  off_t bytes_read = 0;
  uint8_t *bounce = NULL;
  while (size > 0) 
    {
      /* Disk sector to read, starting byte offset within sector. */

      block_sector_t sector_idx = byte_to_sector (inode, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually copy out of this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;


      cache_read(sector_idx, buffer + bytes_read, chunk_size, sector_ofs);
        
      /*
      if (sector_ofs == 0 && chunk_size == BLOCK_SECTOR_SIZE)
        {
          block_read (fs_device, sector_idx, buffer + bytes_read);
        }
      else 
        {
          if (bounce == NULL) 
            {
              bounce = malloc (BLOCK_SECTOR_SIZE);
              if (bounce == NULL)
                break;
            }
          block_read (fs_device, sector_idx, bounce);
          memcpy (buffer + bytes_read, bounce + sector_ofs, chunk_size);
        }
        */
      
      
      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_read += chunk_size;
    }
  free(bounce);
  return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t
inode_write_at (struct inode *inode, const void *buffer_, off_t size,
                off_t offset) 
{
  const uint8_t *buffer = buffer_;
  off_t bytes_written = 0;
  uint8_t *bounce = NULL;
  if (inode->deny_write_cnt)
    return 0;

  struct inode_disk data;
  cache_read (inode->sector, &data, BLOCK_SECTOR_SIZE, 0);
  
  if (offset>=data.length){
    inode_resize(&data,offset-data.length+1,inode->sector);
  }


  while (size > 0) 
    {
      /* Sector to write, starting byte offset within sector. */
      //printf("inode writing");
      block_sector_t sector_idx = byte_to_sector (inode, offset);
      //printf("sector idx is %"PRDSNu"",sector_idx);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually write into this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;

      /*
      if (sector_ofs == 0 && chunk_size == BLOCK_SECTOR_SIZE)
        {
          block_write (fs_device, sector_idx, buffer + bytes_written);
        }
      else 
        {
          if (bounce == NULL) 
            {
              bounce = malloc (BLOCK_SECTOR_SIZE);
              if (bounce == NULL)
                break;
            }
          if (sector_ofs > 0 || chunk_size < sector_left) 
            block_read (fs_device, sector_idx, bounce);
          else
            memset (bounce, 0, BLOCK_SECTOR_SIZE);
          memcpy (bounce + sector_ofs, buffer + bytes_written, chunk_size);
          block_write (fs_device, sector_idx, bounce);
        }
      */

      cache_write(sector_idx, buffer + bytes_written, chunk_size, sector_ofs);

      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_written += chunk_size;
    }
    free(bounce);
  return bytes_written;
}

/* Disables writes to INODE.
   May be called at most once per inode opener. */
void
inode_deny_write (struct inode *inode) 
{
  inode->deny_write_cnt++;
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
}

/* Re-enables writes to INODE.
   Must be called once by each inode opener who has called
   inode_deny_write() on the inode, before closing the inode. */
void
inode_allow_write (struct inode *inode) 
{
  ASSERT (inode->deny_write_cnt > 0);
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
  inode->deny_write_cnt--;
}

/* Returns the length, in bytes, of INODE's data. */
off_t
inode_length (const struct inode *inode)
{
  struct inode_disk data;
  cache_read (inode->sector, &data, BLOCK_SECTOR_SIZE, 0);
  return data.length;
}

/* Added */
void cache_init(struct cache_elem **cache){
  int i;
  for(i = 0; i < CACHE_SIZE; i++){
    cache[i] = malloc(sizeof(struct cache_elem));
    lock_init(&(cache[i]->lock));
    wipe_cache_elem(cache[i]);
  }
}

/* empties a cache slot */
void wipe_cache_elem(struct cache_elem *elem){
  elem->sector = 0;
  elem->dirty = false;
  elem->empty = true;
  elem->chances = 0;
  memset(elem->data, 0, BLOCK_SECTOR_SIZE);
}

/* interface to read from cache first */
void cache_read(block_sector_t sector, void *buffer, int chunk_size, int sector_ofs){
  int i;
  for(i = 0; i < CACHE_SIZE; i++){
    if(!cache[i]->empty && (cache[i]->sector == sector)){
      //do we need to do a lock check? in case of write?
      memcpy (buffer, cache[i]->data + sector_ofs, chunk_size);
      cache[i]->chances = CLOCK_CHANCES;
      return;
    }
  }
  int cache_to_evict = next_free_cache_slot();
  if (cache[cache_to_evict]->dirty){
    block_write (fs_device, cache[cache_to_evict]->sector, 
      cache[cache_to_evict]->data);
  }
  struct cache_elem *cache_slot;
  cache_slot = cache[cache_to_evict];
  cache_slot->sector = sector;
  cache_slot->dirty = false;
  cache_slot->empty = false;
  cache_slot->chances = CLOCK_CHANCES;
  block_read(fs_device, sector, cache_slot->data);
  memcpy (buffer, cache_slot->data + sector_ofs, chunk_size);
}

/* interface to write to cache first */
void cache_write(block_sector_t sector, const void *buffer, int chunk_size, int sector_ofs){
  int i;
  for(i = 0; i < CACHE_SIZE; i++){
    if(!cache[i]->empty && (cache[i]->sector == sector)){
      //do we need to do a lock check? in case of write?
      memcpy (cache[i]->data + sector_ofs, buffer, chunk_size);
      cache[i]->dirty = true;
      cache[i]->chances = CLOCK_CHANCES;
      return;
    }
  }  
  int cache_to_evict = next_free_cache_slot();
  if (cache[cache_to_evict]->dirty){
    block_write (fs_device, cache[cache_to_evict]->sector, 
      cache[cache_to_evict]->data);
  }
  struct cache_elem *cache_slot;
  cache_slot = cache[cache_to_evict];
  cache_slot->sector = sector;
  cache_slot->dirty = true;
  cache_slot->empty = false;
  cache_slot->chances = CLOCK_CHANCES;
  block_read(fs_device, sector, cache_slot->data);
  memcpy (cache_slot->data + sector_ofs, buffer, chunk_size);
}

int next_free_cache_slot(){
  while(true){
    clock_hand = (clock_hand + 1) % CACHE_SIZE;
    if(cache[clock_hand]->empty || (cache[clock_hand]->chances == 0)){
      return clock_hand;
    }else{
      cache[clock_hand]->chances = cache[clock_hand]->chances - 1;
    }
  }
}
