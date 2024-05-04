#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
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
  backtrace();
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


// my syscall implemetations for alarm
int
sys_sigalarm(void)
{
  int ticks;
  uint64 handler;
  argint(0, &ticks);
  argaddr(1, &handler);
  struct proc* p = myproc();
  if(ticks == 0 && handler == 0)
  {
    p->alarm_hanler_on = 0;
    p->ticks = 0;
    p->ticks_from_last_handler = 0;
  }
  else
  {
    p->is_handling_now = 0;
    p->ticks = ticks;
    p->handler = handler;
    p->alarm_hanler_on = 1;
  }
  return 0;
}




int
sys_sigreturn(void)
{
  struct proc* p = myproc();
  
  // recovering the state of the CPU before the call to the uder handler
        p->trapframe->epc = p->prev_epc;
        p->trapframe->kernel_satp = p->prev_kernel_satp;   
        p->trapframe->kernel_sp = p->prev_kernel_sp;     
        p->trapframe->kernel_trap = p->prev_kernel_trap;   
        p->trapframe->epc = p->prev_epc;           
        p->trapframe->kernel_hartid = p->prev_kernel_hartid; 
        p->trapframe->ra = p->prev_ra;
        p->trapframe->sp = p->prev_sp;
        p->trapframe->gp = p->prev_gp;
        p->trapframe->tp = p->prev_tp;
        p->trapframe->t0 = p->prev_t0;
        p->trapframe->t1 = p->prev_t1;
        p->trapframe->t2 = p->prev_t2;
        p->trapframe->s0 = p->prev_s0;
        p->trapframe->s1 = p->prev_s1;
        p->trapframe->a0 = p->prev_a0;
        p->trapframe->a1 = p->prev_a1;
        p->trapframe->a2 = p->prev_a2;
        p->trapframe->a3 = p->prev_a3;
        p->trapframe->a4 = p->prev_a4;
        p->trapframe->a5 = p->prev_a5;
        p->trapframe->a6 = p->prev_a6;
        p->trapframe->a7 = p->prev_a7;
        p->trapframe->s2 = p->prev_s2;
        p->trapframe->s3 = p->prev_s3;
        p->trapframe->s4 = p->prev_s4;
        p->trapframe->s5 = p->prev_s5;
        p->trapframe->s6 = p->prev_s6;
        p->trapframe->s7 = p->prev_s7;
        p->trapframe->s8 = p->prev_s8;
        p->trapframe->s9 = p->prev_s9;
        p->trapframe->s10 = p->prev_s10;
        p->trapframe->s11 = p->prev_s11;
        p->trapframe->t3 = p->prev_t3;
        p->trapframe->t4 = p->prev_t4;
        p->trapframe->t5 = p->prev_t5;
        p->trapframe->t6 = p->prev_t6;
  p->is_handling_now = 0;
  return p->trapframe->a0;
}
