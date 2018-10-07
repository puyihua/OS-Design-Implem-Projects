#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "fcntl.h"

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
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int argfd(int n, int *pfd, struct file **pf)
{
	int fd;
	struct file *f;
	if (argint(n, &fd)<0)
		return -1;
	if(fd<0||fd>=NOFILE||(f=proc->ofile[fd])==0)
		return -1;
	if(pfd)
		*pfd=fd;
	if(pf)
		*pf = f;
	return 0;
}

addr_t sys_mmap() {
  int fd;
  int flags;
  struct file *f;
  pde_t* pgdir = proc->pgdir;
  char *mem;
  
  addr_t oldsz, fsz, newsz, a;
  
  if(argfd(0,&fd,&f) < 0 || argint(1,&flags) < 0) {
    return -1;
  }

  cprintf("mmapping fd %d with %s\n",fd,(flags==0?"eager":"lazy"));
  switch (flags){
	case 0:
	  ;
	  oldsz = proc->mmapsz + MMAPBASE;
	  fsz = f->ip->size;
	  newsz = PGROUNDUP(oldsz + fsz);
	  a = PGROUNDUP(oldsz);
	  for(;a < newsz;a+=PGSIZE){
		  mem = kalloc();
		  if(mem == 0) return 0;
		  memset(mem,0,PGSIZE);
		  mappages(pgdir, (char*)a, PGSIZE, V2P(mem),PTE_W|PTE_U);
		  fileread(f,a,PGSIZE);
	  }
	  proc->mmapsz = newsz-MMAPBASE;
	  switchuvm(proc);
	  return oldsz;
	  break;
	case 1:
	  ;
	  oldsz = proc->mmapsz + MMAPBASE;
	  //fsz = f->ip->size;
	  //newsz = PGROUNDUP(oldsz+fsz);
	  
	  //proc->mmapsz = newsz-MMAPBASE;//the lowest addr of the next unused page
	  //proc->lazy_mmap_loaded++;
	  switchuvm(proc);
	  return oldsz;
	  break;
	default:
	  cprintf("ERROR: unknown kind of mmap flag.\n");
	  return -1;
  }
}
void pageintr(addr_t a){
  int fd;
  int flags;
  struct file *f;
  pde_t* pgdir = proc->pgdir;
  char *mem;
  
  addr_t aa;
  aa = PGROUNDDOWN(a);
  addr_t oldsz,fsz,newsz;

  if(argfd(0,&fd,&f) < 0 || argint(1,&flags) < 0) {
    return -1;
  }
  
  oldsz = proc->mmapsz + MMAPBASE;
  newsz = PGROUNDUP(oldsz + PGSIZE);

  mem = kalloc();
  if(mem == 0) return -1;
  memset(mem,0,PGSIZE);
  
  mappages(pgdir, (char*)aa, PGSIZE, V2P(mem),PTE_W|PTE_U);
  readi(f->ip, (char*)aa, aa-oldsz, PGSIZE);
  //switchuvm(proc);
  proc->mmapsz = newsz - MMAPBASE;
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
    if(proc->killed){
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
