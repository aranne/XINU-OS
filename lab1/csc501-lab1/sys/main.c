#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <stdio.h>
#include <lab1.h>

#define LOOP	50

int prA, prB, prC, prD, prE;
int proc_a(char), proc_b(char), proc_c(char), proc_d(char), proc_e(char);
int proc(char);
volatile int a_cnt = 0;
volatile int b_cnt = 0;
volatile int c_cnt = 0;
volatile int d_cnt = 0;
volatile int e_cnt = 0;
volatile int s = 0;

int main() {
	int i;
	int count = 0;
	char buf[8];
    
	srand(1234);

	kprintf("Please Input:\n");
	while ((i = read(CONSOLE, buf, sizeof(buf))) < 1)
		;
	buf[i] = 0;
	s = atoi(buf);
	kprintf("Get %d\n", s);

	// RANDOMSCHED
	if (s < 2) {
		setschedclass(RANDOMSCHED);
		prA = create(proc_a, 2000, 10, "proc A", 1, 'A');
		prB = create(proc_b, 2000, 9, "proc B", 1, 'B');
		prC = create(proc_c, 2000, 10, "proc C", 1, 'C');
		prD = create(proc_d, 2000, 10, "proc D", 1, 'D');
		prE = create(proc_e, 2000, 20, "proc E", 1, 'E');
		resume(prA);
		resume(prB);
		resume(prC);
		resume(prD);
		resume(prE);
		chprio(prB, 15);
		chprio(prE, 5);
	    // chprio(prE,  5);
		sleep(5);
		kill(prA);
		kill(prB);
		kill(prC);
		kill(prD);
		kill(prE);

		kprintf("\nTest Result: A = %d, B = %d, C = %d, D = %d, E = %d\n", a_cnt, b_cnt, c_cnt, d_cnt, e_cnt);
	}
	// LINUXSCHED
	else {
		setschedclass(LINUXSCHED);
		resume(prA = create(proc, 2000, 5, "proc A", 1, 'A'));
		resume(prB = create(proc, 2000, 50, "proc B", 1, 'B'));
		resume(prC = create(proc, 2000, 90, "proc C", 1, 'C'));

		while (count++ < LOOP) {
			kprintf("M");
			for (i = 0; i < 1000000; i++)
				;
		}
        kprintf("\n");
	}
	return 0;
}

int proc_a(char c) {
	int i;
	kprintf("Start... %c\n", c);
	b_cnt = 0;
	c_cnt = 0;
	d_cnt = 0;
	e_cnt = 0;

	while (1) {
		for (i = 0; i < 10000; i++)
			;
		a_cnt++;
	}
	return 0;
}

int proc_b(char c) {
	int i;
	kprintf("Start... %c\n", c);
	a_cnt = 0;
	c_cnt = 0;
	d_cnt = 0;
	e_cnt = 0;

	while (1) {
		for (i = 0; i < 10000; i++)
			;
		b_cnt++;
	}
	return 0;
}

int proc_c(char c) {
	int i;
	kprintf("Start... %c\n", c);
	a_cnt = 0;
	b_cnt = 0;
	d_cnt = 0;
	e_cnt = 0;

	while (1) {
		for (i = 0; i < 10000; i++)
			;
		c_cnt++;
	}
	return 0;
}

int proc_d(char c) {
	int i;
	kprintf("Start... %c\n", c);
	a_cnt = 0;
	b_cnt = 0;
	c_cnt = 0;
	e_cnt = 0;

	while (1) {
		for (i = 0; i < 10000; i++)
			;
		d_cnt++;
	}
	return 0;
}

int proc_e(char c) {
	int i;
	kprintf("Start... %c\n", c);
	a_cnt = 0;
	b_cnt = 0;
	c_cnt = 0;
	d_cnt = 0;

	while (1) {
		for (i = 0; i < 10000; i++)
			;
		e_cnt++;
	}
	return 0;
}

int proc(char c) {
	int i;
	int count = 0;

	while (count++ < LOOP) {
		kprintf("%c", c);
		for (i = 0; i < 1000000; i++)
			;
	}
	return 0;
}