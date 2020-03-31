/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>
extern unsigned long clktime;
extern unsigned long ctr1000;
int t = 1;
int prA,prB,prC,prD;
void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
prcha(a)
char a;
{
	int i;
	sleep(5);	
}
prchb(b)
char b;
{
	int i;
	for (i=0;i<4;i++)
		sleep(1);
}
prchc(pid)
int pid;
{
	stacktrace(pid);
}
prchd(pid)
int pid;
{
	kill(pid);
}
void f1_test(){
    	kprintf("Task 1 (zfunction)\n");
	long params[] = {0x12345678,0xddccaabb,0x00112233,0x00001111,0x89abcdef};
	long answers[] = {0x20016780, 0xD802ABB0, 0x22330, 0x11110, 0x9800DEF0};
	int i;
	int n_failed = 0;
	for (i=0;i<5;i++)
		if (zfunction(params[i]) != answers[i]){
			kprintf("FAILED  Input: 0x%08x, returned: 0x%08x, expected: 0x%08x\n",params[i],zfunction(params[i]),answers[i]);
			n_failed += 1;
		}
	kprintf("FAILED %d/5 test cases\n",n_failed);
	kprintf("----------------------\n");
			
}
void f2_test(){
	kprintf("Task 2 (printsegaddress)\n");
    	printsegaddress ();
	kprintf("----------------------\n");
}
void f3_test(){
    	kprintf("Task 3 (printtos)\n");
    	printtos();
	kprintf("----------------------\n");
}
void f4_test(){
    	kprintf("Task 4 (printprocstks)\n");
	kprintf("Test case1: should print only process \"main\"\n");
    	printprocstks(10);

	kprintf("Test case2: should print process \"main\" and process \"proc A\"\n");
	resume(prA = create(prcha,2000,30,"proc A",1,'A'));
	sleep(1);
	printprocstks(10);
	kill(prA);

	kprintf("Test case3: should still print process \"main\" and process \"proc A\"\n");
	resume(prA = create(prcha,2000,30,"proc A",1,'A'));
	resume(prB = create(prcha,2000,5,"proc A",1,'A'));
	printprocstks(10);
	kill(prA);
	kill(prB);

	kprintf("Test case4: should print process \"main\", process \"proc A\", process \"proc B\"\n");
	resume(prA = create(prcha,2000,30,"proc A",1,'A'));
	resume(prB = create(prcha,2000,5,"proc B",1,'B'));
	printprocstks(4);
	kill(prA);
	kill(prB);

	kprintf("Test case5: should not print any process\n");
	resume(prA = create(prcha,2000,30,"proc A",1,'A'));
	resume(prB = create(prcha,2000,5,"proc B",1,'B'));
	printprocstks(50);
	kill(prA);
	kill(prB);

	kprintf("----------------------\n");
}
void f5_test(){
	kprintf("Task 5 (printsyssummary)\n");
	kprintf("Test case1: should print info for only process \"main\" with at least sys_sleep(count:1,average execution time:10000) and sys_sleep10(count:1,average execution time:10000)  \n");
	syscallsummary_start();        
	sleep(10);
	syscallsummary_stop();	
	printsyscallsummary();
	
	kprintf("Test case2: should print info for process \"main\" as before and process \"proc A\" with at least sys_sleep(count:1,average execution time:5000) and sys_sleep10(count:1,average execution time:5000) \n");
	syscallsummary_start();
	resume(prA = create(prcha,2000,20,"proc A",1,'A'));	
	sleep(10);
	syscallsummary_stop();	
	printsyscallsummary();

	kprintf("Test case3: should print info for process \"main\" and \"proc A\" as in the 2nd test case and \"proc B\" with at least sys_sleep(count:4,average execution time:1000) and sys_sleep10(count:4,average execution time:1000) \n");
	syscallsummary_start(); 
	resume(prA = create(prcha,2000,20,"proc A",1,'A'));	
	resume(prB = create(prchb,2000,20,"proc B",1,'B'));		
	sleep(10);
	syscallsummary_stop();	
	printsyscallsummary();	
	
	kprintf("Test case4: should print info for process \"main\" and \"proc A\" as in the 2nd test case and \"proc C\" with at least stacktrace(count:1,average execution time:50)\n");
	syscallsummary_start(); 
	resume(prA = create(prcha,2000,20,"proc A",1,'A'));	
	resume(prC = create(prchc,2000,20,"proc C",1,prA));		
	sleep(10);
	syscallsummary_stop();	
	printsyscallsummary();	
	
	kprintf("Test case5: should print info for process \"main\" and \"proc A\" as in the 2nd test case and \"proc D\" with at least kill(count:2,average execution time:0)\n");
	syscallsummary_start(); 
	resume(prA = create(prcha,2000,20,"proc A",1,'A'));		
	sleep(10);
	resume(prD = create(prchd,2000,20,"proc D",1,prA));		
	syscallsummary_stop();	
	printsyscallsummary();	
}
int main()
{
	kprintf("\nHiep Nguyen (hcnguye3)\n\n");
	
	f1_test();


	f2_test();
	
	
	f3_test();


	f4_test();
	
	f5_test();

	return 0;
}
