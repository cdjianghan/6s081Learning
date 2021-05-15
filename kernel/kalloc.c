// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct kmem{
  struct spinlock lock;
  struct run *freelist;
} ;


struct kmem kmemArray[NCPU];

void
kinit()
{

  for(int i = 0 ; i < NCPU ; i++){
    initlock(&(kmemArray[i].lock), "kmem");
    
  }
  freerange(end, (void*)PHYSTOP);

}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  push_off();
  int c = cpuid();
  pop_off();

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmemArray[c].lock);
  r->next = kmemArray[c].freelist;
  kmemArray[c].freelist = r;
  release(&kmemArray[c].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int c = cpuid();
  pop_off();

  acquire(&kmemArray[c].lock);
  r = kmemArray[c].freelist;
  if(r){
    kmemArray[c].freelist = r->next;
  }else{
    for(int i = (c + 1)%NCPU  ; i != c; i=(i+1)%NCPU ){
      if(kmemArray[i].freelist){
        r = kmemArray[i].freelist;
        kmemArray[i].freelist = kmemArray[i].freelist->next;
        // r->next = kmemArray[c].freelist;
        // kmemArray[c].freelist = r;
        break;
      }
    }
  }
  

  // if(r){
  //   kmemArray[c].freelist = r->next;
  //   }

  release(&kmemArray[c].lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
