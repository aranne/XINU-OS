/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lab0.h"

int prX;
void halt();

prch(c)
char c;
{
	int i;
	sleep(5);	
}

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	kprintf("\n\nHello World, Xinu lives\n\n");
	long x = zfunction(0xaabbccdd);
	kprintf("0x%0x\n\n", x);

	printsegaddress();

	printtos();

	printprocstks(-2);

	syscallsummary_start();
	resume(prX = create(prch,2000,20,"proc X",1,'A'));
	
	sleep(10);
	syscallsummary_stop();
	printsyscallsummary();

	return 0;
}
