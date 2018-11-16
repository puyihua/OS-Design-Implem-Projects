#include "types.h"
#include "param.h"
#include "defs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "fs.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "memlayout.h"

#define T_DIR ((1))

  void
procfs_ipopulate(struct inode* ip)
{
  ip->size = 0;
  ip->flags |= I_VALID;

  // inum < 10000 are reserved for directories
  // use inum > 10000 for files in procfs
  ip->type = ip->inum < 10000 ? T_DIR : 100;
}

  void
procfs_iupdate(struct inode* ip)
{
}

  static int
procfs_writei(struct inode* ip, char* buf, uint offset, uint count)
{
  return -1;
}

  static void
sprintuint(char* buf, uint x)
{
  uint stack[10];
  uint stack_size = 0;
  if (x == 0) {
    buf[0] = '0';
    buf[1] = '\0';
    return;
  }
  while (x) {
    stack[stack_size++] = x % 10u;
    x /= 10u;
  }
  uint buf_size = 0;
  while (stack_size) {
    buf[buf_size++] = '0' + stack[stack_size - 1];
    stack_size--;
  }
  buf[buf_size] = 0;
}

  static void
sprintx32(char * buf, uint x)
{
  buf[0] = x >> 28;
  for (int i = 0; i < 8; i++) {
    uint y = 0xf & (x >> (28 - (i * 4)));
    buf[i] = (y < 10) ? (y + '0') : (y + 'a' - 10);
  }
}

extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

#define PROCFILES ((2))
struct dirent procfiles[PROCFILES+NPROC+1] = {{10001,"meminfo"}, {10002,"cpuinfo"}};

// returns the number of active processes, and updates the procfiles table
  static uint
updateprocfiles()
{
  int num = 0, index = 0;
  acquire(&ptable.lock);
  while (index < NPROC) {
    if (ptable.proc[index].state != UNUSED && ptable.proc[index].state != ZOMBIE) {
      procfiles[PROCFILES+num].inum = index+1;
      sprintuint(procfiles[PROCFILES+num].name,ptable.proc[index].pid);
      num++;
      // also checks if the process at [index] is the current process
      // if yes, create a "self" directory
	  if (ptable.proc[index].state == RUNNING)
	  {
		  procfiles[PROCFILES+num].inum = index+1;
		  char * name = procfiles[PROCFILES+num].name;
		  char self_str[4] = "self";
		  memcpy(name, self_str, 4);
		  num++;
	  }
	  /*
	  int that_pid = ptable.proc[index].pid;
      procfiles[PROCFILES+num].inum = 20000+that_pid;
	  char tmp[10] = "name";
	  memcpy(procfiles[PROCFILES+num].name,tmp,4);
	  num++;	
	  //{30000+that_pid,"pid"},{40000+that_pid,"ppid"},{50000+that_pid,"mappings"}};
*/

    }
    index++;
  }
  release(&ptable.lock);
  return PROCFILES + num;
}

  static int
readi_helper(char * buf, uint offset, uint maxsize, char * src, uint srcsize)
{
  if (offset > srcsize)
    return -1;
  uint end = offset + maxsize;
  if (end > srcsize)
    end = srcsize;
  memmove(buf, src+offset, end-offset);
  return end-offset;
}

  int
procfs_readi(struct inode* ip, char* buf, uint offset, uint size)
{
  const uint procsize = sizeof(struct dirent)*updateprocfiles();
  // the mount point
  if (ip->mounted_dev == 2) {
    return readi_helper(buf, offset, size, (char *)procfiles, procsize);
  }

  // directory - can only be one of the process directories
  if (ip->type == T_DIR) {
    // List the files in a process directory:
    // It contains "name", "pid", "ppid", and "mappings".
    // Choose a good pattern for inum.
    // You will need to check inum to see what should be the file content (see below)

   int inum = ip->inum;
   int i;
   for(i =0;i<updateprocfiles();i++)
   {
	   if(procfiles[i].inum == inum)
		   break;
   }
   char * name = procfiles[i].name;
   int that_pid = 0;
   while('0' <= *name && *name <= '9')
	   that_pid = that_pid*10 + *name++ - '0';

  // int index = updateprocfiles();
  // procfiles[index].inum = 20000+that_pid;
   //char tmp[10] = "name";
  // procfiles[index].name
   
   struct dirent inner_procfiles[4] = {{20000+that_pid,"name"},{30000+that_pid,"pid"},{40000+that_pid,"ppid"},{50000+that_pid,"mappings"}};

    return readi_helper(buf, offset, size, (char *)inner_procfiles, sizeof(struct dirent)*4);
  }

  // files
  char buf1[32];
  switch (((int)ip->inum)) {
    case 10001: // meminfo: print the number of free pages
      sprintuint(buf1, kmemfreecount());
      return readi_helper(buf, offset, size, buf1, strlen(buf1));
    case 10002: // cpuinfo: print the total number of cpus. See the 'ncpu' global variable
      sprintuint(buf1, ncpu);
	  return readi_helper(buf, offset, size, buf1, strlen(buf1));


      return -1; // remove this after implementation
    default: break;
  }

   int inum = ip->inum;
   int i;

  struct proc *p;
  for(i=0;i<NPROC;i++)
  {
	  if (ptable.proc[i].pid == inum%10)
		  break;
  }
  p = &ptable.proc[i];

  if(inum>20000 && inum<30000){             //name 
      memcpy(buf1, p->name, 16);
      return readi_helper(buf, offset, size, buf1, strlen(buf1));}
   else if (inum>30000 && inum<40000)  {    //pid
	  sprintuint(buf1, p->pid);
      return readi_helper(buf, offset, size, buf1, strlen(buf1));}
   else if (inum>40000 && inum<50000)  {   //ppid
	  sprintuint(buf1,p->parent->pid);
      return readi_helper(buf, offset, size, buf1, strlen(buf1));}
   else if (inum>50000 && inum<60000)  {   //mappings
      char buf2[128];
	  addr_t i = 0;
	  int j = 0;
	  //for(;i+PGSIZE <= PGROUNDUP(p->sz); i+=PGSIZE)
	  for(;i+PGSIZE <= PGROUNDUP(p->sz);i+=PGSIZE)
	  {
		  //walkpgdir(p->pgdir,i,0);
		  sprintx32(buf2+j,i);
		  j = j+8;
		  *(buf2+j) = ' ';
		  j++;
		  sprintx32(buf2+j,PTE_ADDR(*walkpgdir(p->pgdir,(char*)i,0)));
		  j = j+8;
		  *(buf2+j) = '\n';
		  j++;
		 // sprintx32(buf1,walkpgdir(p->pgdir,i,0));
	  }
      return readi_helper(buf, offset, size, buf2, strlen(buf2));}
      
  return -1; // return -1 on error
}

struct inode_functions procfs_functions = {
  procfs_ipopulate,
  procfs_iupdate,
  procfs_readi,
  procfs_writei,
};

  void
procfsinit(char * const path)
{
  begin_op();
  struct inode* mount_point = namei(path);
  if (mount_point) {
    ilock(mount_point);
    mount_point->i_func = &procfs_functions;
    mount_point->mounted_dev = 2;
    iunlock(mount_point);
  }
  end_op();
}
