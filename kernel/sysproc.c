#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

// Modify sys_sbrk in kernel/sysproc.c

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;
  struct proc *p = myproc();

  argint(0, &n);  // Remove the if() check
  
  addr = p->sz;
  
  // Check if we should use superpages
  if(n >= SUPERPGSIZE) {
    // Calculate how much can be allocated with superpages
    uint64 start = addr;
    uint64 end = addr + n;
    
    // Find 2MB-aligned region within the range
    uint64 super_start = SUPERPGROUNDUP(start);
    uint64 super_end = SUPERPGROUNDDOWN(end);
    
    if(super_start < super_end) {
      // We have at least one superpage to allocate
      
      // 1. Allocate normal pages before the superpage region
      if(super_start > start) {
        if(growproc(super_start - start) < 0)
          return -1;
      }
      
      // 2. Allocate superpages
      uint64 num_superpages = (super_end - super_start) / SUPERPGSIZE;
      for(uint64 i = 0; i < num_superpages; i++) {
        void *mem = superalloc();
        if(mem == 0) {
          return -1;
        }
        
        uint64 va = super_start + i * SUPERPGSIZE;
        if(mappages_super(p->pagetable, va, SUPERPGSIZE, (uint64)mem,
                         PTE_W | PTE_X | PTE_R | PTE_U) < 0) {
          superfree(mem);
          return -1;
        }
      }
      
      p->sz = super_end;
      
      // 3. Allocate normal pages after the superpage region
      if(end > super_end) {
        if(growproc(end - super_end) < 0)
          return -1;
      }
      
      return addr;
    }
  }
  
  // Normal allocation (no superpages)
  if(growproc(n) < 0)
    return -1;
  
  return addr;
}


uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgpte(void)
{
  uint64 va;
  struct proc *p;  

  p = myproc();
  argaddr(0, &va);
  pte_t *pte = pgpte(p->pagetable, va);
  if(pte != 0) {
      return (uint64) *pte;
  }
  return 0;
}
#endif

#ifdef LAB_PGTBL
int
sys_kpgtbl(void)
{
  struct proc *p;  

  p = myproc();
  vmprint(p->pagetable);
  return 0;
}
#endif


uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 sys_getcycles(void) {
//   uint64 cycles;
//   asm volatile("rdcycle %0" : "=r" (cycles));
  // return cycles;
  return r_cycle();
} 

uint64 sys_gettime(void) {
  // uint64 time;
  // asm volatile("rdtime %0" : "=r" (time));
  // return time;
  return r_time();
} 

uint64 sys_getinstret(void) {
  // uint64 instret;
  // asm volatile("rdinstret %0" : "=r" (instret));
  // return instret;
  return r_instret();
} 

uint64
sys_gettotalmem(void)
{
  return get_total_memory();
}