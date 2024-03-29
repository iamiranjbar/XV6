#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "syscall.h"
#include "spinlock.h"
#include "ticket_lock.h"
#include "semaphore.h"
#include "rw_lock.h"
#include "wr_lock.h"

extern struct node* first_proc;
struct ticket_lock ticketlock;
struct rw_lock rwLock;
struct wr_lock wrLock;
int safe_count = 0;
int read_write_race = 0;
int race = 0;

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_inc_num(void)
{
  int num;
 // register int *num asm ("ebx");
   if(argint(0, &num) < 0)
     return -1;
  cprintf("num : %d\n", num+1);
  return 0;
}

int
sys_invoked_syscalls(void)
{
  int pid, i;

  if(argint(0, &pid) < 0)
    return -1;
  cprintf("num : %d\n", pid);
  i = invocation_log(pid);
  if(i >= 0){
    return 0;
  }
  return -1;
}

int
sys_sort_syscalls(void)
{
  int pid, i;

  if(argint(0, &pid) < 0)
    return -1;
  cprintf("num : %d\n", pid);
  i = invocation_log(pid);
  if(i >= 0){
    return 0;
  }
  return -1;
}

int
sys_get_count(void)
{
  int pid, sysnum, result;

  if (argint(0, &pid) < 0 || argint(1, &sysnum) < 0)
    return -1;
  result = get_syscall_count(pid, sysnum);
  cprintf("count_syscall: pid: %d sysnum: %d res:%d\n", pid, sysnum, result);
  return result;
}

void
sys_log_syscalls(void)
{
  log_syscalls(first_proc);
}

void
sys_ticketlockinit(void)
{
  init_ticket_lock(&ticketlock, "ticket_lock");
}

void
sys_ticketlocktest(void)
{
  ticket_acquire(&ticketlock);
  safe_count++;
  //cprintf("ali: %d\n", safe_count);
  ticket_release(&ticketlock);
  cprintf("ali: %d\n", safe_count);
}

void
sys_rwinit(void)
{
	init_rw_lock(&rwLock);
}

void
sys_rwtest(uint pattern)
{
  int bits[32];
  int i =0;
  int j;
  int readData = 0;
  while (pattern > 0){
    bits[i] =  pattern % 2;
    pattern = pattern /2;
    i++;
  }
  i -= 2;
  for (j = i; j >= 0; j--){
    if (bits[j] == 0){
      acquire_reader(&rwLock);
      readData = read_write_race;
      cprintf(">>>>> %d\n", readData);
      release_reader(&rwLock);
    }else{  
      acquire_writer(&rwLock);
      read_write_race++;
      cprintf("<<<<<< %d\n", read_write_race);
      release_writer(&rwLock);
    }
  }
}

void
sys_wrinit(void)
{
	init_wr_lock(&wrLock);
}

void
sys_wrtest(uint pattern)
{
  int bits[32];
  int i =0;
  int j;
  int readData = 0;
  while (pattern > 0){
    bits[i] =  pattern % 2;
    pattern = pattern /2;
    i++;
  }
  i -= 2;
  for (j = i; j >= 0; j--){
    if (bits[j] == 0){
      acquire_reader_b(&wrLock);
      readData = read_write_race;
      cprintf(">>>>> %d\n", readData);
      release_reader_b(&wrLock);
    }else{  
      acquire_writer_b(&wrLock);
      read_write_race++;
      cprintf("<<<<<< %d\n", read_write_race);
      release_writer_b(&wrLock);
    }
  }
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int sys_chtickets(void)
{
  int pid, ticks_in;

  if (argint(0, &pid) < 0 || argint(1, &ticks_in) < 0)
    return -1;
  change_tickets(pid, ticks_in);
  return 0;
}

int sys_chpr(void)
{
  int pid, pr;

  if (argint(0, &pid) < 0 || argint(1, &pr) < 0)
    return -1;
  chpr(pid, pr);
  return 0;
}

int sys_chmfq(void)
{
  int pid, pr;

  if (argint(0, &pid) < 0 || argint(1, &pr) < 0)
    return -1;
  chmfq(pid, pr);
  return 0; 
}

int sys_ps(void)
{
  ps();
  return 0;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_halt(void)
{
  outb(0xf4, 0x00);
  return 0;
}
