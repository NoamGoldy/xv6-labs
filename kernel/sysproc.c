#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#define MAXPGS 32

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

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
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


// saving in *bit_mask the mask of the pages that
// got accesed
int
sys_pgaccess(void)
{
  uint64 starting_va;
  int num_of_pages; 
  uint64 bit_mask_va;
  unsigned int bit_mask = 0;
  argaddr(0, &starting_va);
  argint(1, &num_of_pages);
  argaddr(2, &bit_mask_va);
  if(num_of_pages > MAXPGS){
    printf("pgaccess cannot track so mush pages...\n");
    return -1;
  }
  //checking accesss bit loop
  for(int i = 1; i < num_of_pages; i++){
    if((*walk(myproc()->pagetable, starting_va + PGSIZE * i, 0) & PTE_A) == PTE_A){ // the ith page was accessed
      bit_mask |= (1 << i);
    }
    else{
      *walk(myproc()->pagetable, starting_va + PGSIZE * i, 0) &= ~PTE_A;
    }
  }
  copyout(myproc()->pagetable, bit_mask_va, (char*)(&bit_mask), MAXPGS);
  return 0;
}


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
