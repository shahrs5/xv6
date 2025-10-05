// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define SUPERPAGE_SIZE (2 * 1024 * 1024)  // 2MB
#define NUM_SUPERPAGES 8  // Reserve 8 superpages (16MB total)
extern char end[]; // first address after kernel (from kernel.ld)

struct {
  struct spinlock lock;
  void *pages[NUM_SUPERPAGES];
  int available[NUM_SUPERPAGES];
} superpage_mem;

// Initialize superpages during kinit
void
superpage_init(void)
{
  initlock(&superpage_mem.lock, "superpage");
  
  // Start after kernel
  char *p = (char*)PGROUNDUP((uint64)end);
  
  // Calculate how much space we need to reserve
  uint64 total_super_mem = NUM_SUPERPAGES * SUPERPAGE_SIZE;
  
  // Make sure we don't go past PHYSTOP
  if((uint64)p + total_super_mem > PHYSTOP) {
    panic("superpage_init: not enough memory for superpages");
  }
  
  for(int i = 0; i < NUM_SUPERPAGES; i++) {
    // Align to 2MB boundary
    uint64 addr = (uint64)p;
    if(addr % SUPERPAGE_SIZE != 0) {
      addr = ((addr / SUPERPAGE_SIZE) + 1) * SUPERPAGE_SIZE;
    }
    
    // Check if this superpage would exceed physical memory
    if(addr + SUPERPAGE_SIZE > PHYSTOP) {
      printf("superpage_init: only allocated %d superpages (not enough memory)\n", i);
      // Mark remaining as unavailable
      for(int j = i; j < NUM_SUPERPAGES; j++) {
        superpage_mem.pages[j] = 0;
        superpage_mem.available[j] = 0;
      }
      return;
    }
    
    superpage_mem.pages[i] = (void*)addr;
    superpage_mem.available[i] = 1;
    
    printf("superpage_init: superpage[%d] = %p\n", i, (void*)addr);  // DEBUG
    
    p = (char*)(addr + SUPERPAGE_SIZE);
  }
}

// Allocate a 2MB superpage
void*
superalloc(void)
{
  void *page = 0;
  
  acquire(&superpage_mem.lock);
  
  for(int i = 0; i < NUM_SUPERPAGES; i++) {
    if(superpage_mem.available[i]) {
      page = superpage_mem.pages[i];
      superpage_mem.available[i] = 0;
      
      // Zero out the superpage
      memset(page, 0, SUPERPAGE_SIZE);
      break;
    }
  }
  
  release(&superpage_mem.lock);
  
  return page;
}

// Free a 2MB superpage
void
superfree(void *pa)
{
  if((uint64)pa % SUPERPAGE_SIZE != 0)
    panic("superfree: not aligned");

  acquire(&superpage_mem.lock);

  int found = 0;
  for(int i = 0; i < NUM_SUPERPAGES; i++) {
    if(superpage_mem.pages[i] == pa) {
      if(superpage_mem.available[i])
        panic("superfree: already free");
      superpage_mem.available[i] = 1;
      found = 1;
      break;
    }
  }

  release(&superpage_mem.lock);

  if(!found)
    panic("superfree: not a superpage");
}

// Check if address is a superpage
int
is_superpage(void *pa)
{
  acquire(&superpage_mem.lock);
  
  for(int i = 0; i < NUM_SUPERPAGES; i++) {
    if(superpage_mem.pages[i] == pa) {
      release(&superpage_mem.lock);
      return 1;
    }
  }
  
  release(&superpage_mem.lock);
  return 0;
}

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  
  superpage_init();  // Initialize superpages first
  
  // Superpages are from pages[0] to pages[NUM_SUPERPAGES-1]
  // Free memory starts after the last superpage
  void *free_start = (char*)superpage_mem.pages[NUM_SUPERPAGES - 1] + SUPERPAGE_SIZE;
  
  printf("kinit: freeing memory from %p to %p\n", free_start, (void*)PHYSTOP);
  
  freerange(free_start, (void*)PHYSTOP);
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

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}




uint64
get_total_memory(void)
{
  // PHYSTOP is the physical address where RAM ends
  // RAM starts at 0x80000000 on RISC-V
  // So actual RAM size = PHYSTOP - 0x80000000
  return (uint64)PHYSTOP - 0x80000000;
}