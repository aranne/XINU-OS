/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lab1.h"

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	kprintf("\n\nHello World, Xinu lives\n\n");
	kprintf("%d\n", getschedclass());
	kprintf("%d\n", getschedclass());
	kprintf("%d\n", getschedclass());
	setschedclass(RANDOMSCHED);
	kprintf("%d\n", getschedclass());
	setschedclass(LINUXSCHED);
	kprintf("%d\n", getschedclass());
	return 0;
}
