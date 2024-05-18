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

struct kmem_list {
  struct spinlock lock;
  struct run *freelist;
};

struct kmem_list kmem_per_CPU[NCPU];


void
kinit()
{
  for(int i = 0; i < NCPU; i++){
    initlock(&kmem_per_CPU[i].lock, "kmem");
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

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  
  // understanding which is the current working cpu,
  // giving the page to its free list
  push_off();
  int cpu_idx = cpuid();
  pop_off();

  acquire(&kmem_per_CPU[cpu_idx].lock);
  r->next = kmem_per_CPU[cpu_idx].freelist;
  kmem_per_CPU[cpu_idx].freelist = r;
  release(&kmem_per_CPU[cpu_idx].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r, *ri;

  // understanding which is the current working cpu,
  // trying to alloc from its free_list
  push_off();
  int cpu_idx = cpuid();
  pop_off();

  acquire(&kmem_per_CPU[cpu_idx].lock);
  r = kmem_per_CPU[cpu_idx].freelist;
  if(r)
    kmem_per_CPU[cpu_idx].freelist = r->next;
  release(&kmem_per_CPU[cpu_idx].lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk


  if(!r){
    for(int i = 0; i < NCPU; i++){

      if(i == cpu_idx)
        continue;

      acquire(&kmem_per_CPU[cpu_idx].lock);
      ri = kmem_per_CPU[i].freelist;
      if(ri)
        kmem_per_CPU[i].freelist = ri->next;
      release(&kmem_per_CPU[cpu_idx].lock);
      if(ri){
        r = ri;
        memset((char*)ri, 5, PGSIZE); // fill with junk
        break;
      }
    } 
  }
  return (void*)r;
}
