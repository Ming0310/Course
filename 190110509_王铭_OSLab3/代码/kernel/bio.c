// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"
#define NBUCKETS 17
struct {
  struct spinlock lock[NBUCKETS];
  struct buf buf[NBUF];
  struct buf hashbucket[NBUCKETS];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  // struct buf head;
} bcache;

char* name[NBUCKETS] = {
  "bcache0",
  "bcache1",
  "bcache2",
  "bcache3",
  "bcache4",
  "bcache5",
  "bcache6",
  "bcache7",
  "bcache8",
  "bcache9",
  "bcache10",
  "bcache11",
  "bcache12",
  "bcache13",
  "bcache14",
  "bcache15",
  "bcache16"
};

void
binit(void)
{
  struct buf *b;
  // 初始化锁和各个哈希桶的头结点
  for(int i = 0;i < NBUCKETS;i++){
    initlock(&bcache.lock[i],name[i]);
    bcache.hashbucket[i].next = &bcache.hashbucket[i];
    bcache.hashbucket[i].prev = &bcache.hashbucket[i];
  }
  // initlock(&bcache.lock, "bcache");
  // Create linked list of buffers
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;
  // 将各个缓存映射到对应的桶中
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    // 取散列函数，得到对应的桶号
    int key = b->blockno % NBUCKETS;
    b->next = bcache.hashbucket[key].next;
    b->prev = &bcache.hashbucket[key];
    initsleeplock(&b->lock, "buffer");
    bcache.hashbucket[key].next->prev = b;
    bcache.hashbucket[key].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int key = blockno % NBUCKETS;
  acquire(&bcache.lock[key]);

  // Is the block already cached?
  for(b = bcache.hashbucket[key].next; b != &bcache.hashbucket[key]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[key]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bcache.hashbucket[key].prev; b != &bcache.hashbucket[key]; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock[key]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  // 在当前哈希桶中未找到空闲块，从别的链中找没用的缓存块，要避免死锁
  release(&bcache.lock[key]);
  for(int i = 0;i < NBUCKETS;i++){
    if(i == key) continue;
    acquire(&bcache.lock[i]);
    for(b = bcache.hashbucket[i].prev;b != &bcache.hashbucket[i];b = b->prev){
      if(b->refcnt == 0) {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        // 将b原来所属的链表结构改变
        b->next->prev = b->prev;
        b->prev->next = b->next;
        // 将b加入到key所指的哈希桶中
        acquire(&bcache.lock[key]);
        b->next = bcache.hashbucket[key].next;
        b->prev = &bcache.hashbucket[key];
        bcache.hashbucket[key].next->prev = b;
        bcache.hashbucket[key].next = b;
        release(&bcache.lock[key]);

        release(&bcache.lock[i]);
        acquiresleep(&b->lock);
        return b;
      }
    }
    // 避免死锁
    release(&bcache.lock[i]);
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");
  int key = b->blockno % NBUCKETS;
  releasesleep(&b->lock);
  acquire(&bcache.lock[key]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.hashbucket[key].next;
    b->prev = &bcache.hashbucket[key];
    bcache.hashbucket[key].next->prev = b;
    bcache.hashbucket[key].next = b;
  }
  
  release(&bcache.lock[key]);
}

void
bpin(struct buf *b) {
  int key = b->blockno % NBUCKETS;
  acquire(&bcache.lock[key]);
  b->refcnt++;
  release(&bcache.lock[key]);
}

void
bunpin(struct buf *b) {
  int key = b->blockno % NBUCKETS;
  acquire(&bcache.lock[key]);
  b->refcnt--;
  release(&bcache.lock[key]);
}


