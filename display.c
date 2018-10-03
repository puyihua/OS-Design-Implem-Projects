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

#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000);
ushort contentBuf[2000];
//2000 = 80*25
static struct {
  struct spinlock lock;
  int locking;
} disp;
unsigned int pixel = 0;
int curPosition = 0;

void getCurPosition()
{
	outb(CRTPORT,14);
	curPosition = inb(CRTPORT+1)<<8;
	outb(CRTPORT,15);
	curPosition |= inb(CRTPORT+1);
}

void setCurPosition()
{
	outb(CRTPORT,14);
	outb(CRTPORT+1,curPosition>>8);
	outb(CRTPORT,15);
	outb(CRTPORT+1,curPosition);
	crt[curPosition] = ' '| 0x0700;
}
void getContent()
{ 
	int i;
	for(i=0;i<curPosition;i++)
		contentBuf[i]=crt[i];
}
void setContent()
{
	int i;
	for(i=0;i<2000;i++)
		crt[i] = 0;
	for(i=0;i<curPosition;i++)
		crt[i]=contentBuf[i];
}
int
displayread(struct file *f, char *buf, int n)
{
  return -1;
}

int
displaywrite(struct file *f, char *buf, int n)
{
  int i;
  char *vid = (char*)P2V(0xa0000);
  for(i=0;i<n;i++)
  {
    vid[i+1000*pixel] = buf[i];
  }
  pixel++;
  return n;
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
  switch(param){
    case 1:
      if(value==0x13)
	  {
	    getCurPosition();
		getContent();
        vgaMode13();
	  }
      if(value==0x3)
	  {
		vgaMode3();
		setCurPosition();
		setContent();

	  }
	  break;

	case 2:
	  ;
	  int index = value>>24;
	  int r = (value>>16)&0xff;
	  int g = (value>>8)&0xff;
	  int b = value&0xff;
	  vgaSetPalette(index,r,g,b);
	  break;
  }
  return 1;
}
