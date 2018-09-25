#include <stdarg.h>

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "vga.h"
#include "memlayout.h"

static struct {
  struct spinlock lock;
  int locking;
} disp;

int
displayread(struct file *f, char *buf, int n)
{
  return -1;
}

int
displaywrite(struct file *f, char *buf, int n)
{
  int i;
  ushort *vid = (ushort*)P2V(0xa0000);
  for (i=0;i<n;i++)
  {
    vid[i] = buf[i];
  }
  return 1;
}

void displayinit(void)
{
  initlock(&disp.lock, "display");
  devsw[DISPLAY].write = displaywrite;
  devsw[DISPLAY].read = displayread;
  disp.locking = 1;
}

int displayioctl(struct file * f,int param,int value)
{
  if(param==1){
    if(value==0x3)
      vgaMode3();
    if(value==0x13)
      vgaMode13();
  }
  return 1;
}
