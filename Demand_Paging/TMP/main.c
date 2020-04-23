#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>


#define PROC1_VADDR	0x40000000
#define PROC1_VPNO      0x40000
#define PROC2_VADDR     0x80000000
#define PROC2_VPNO      0x80000
#define TEST1_BS	1

void proc1_test1(char *msg, int lck) {
	char *addr;
	int i;

	get_bs(TEST1_BS, 100);

	if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
		kprintf("xmmap call failed\n");
		sleep(3);
		return;
	}

	addr = (char*) PROC1_VADDR;
	for (i = 0; i < 26; i++) {
		*(addr + i * NBPG) = 'A' + i;
	}

	sleep(3);
        kprintf("Starting reading\n");

	for (i = 0; i < 26; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	int *s = BACKING_STORE_BASE + 1 * BACKING_STORE_UNIT_SIZE + 25 * NBPG;
	kprintf("Store value:%c\n", *s);

	printbs();
	printdirs();
	printtbls();
	printpgs();

	xmunmap(PROC1_VPNO);

	printbs();
	printdirs();
	printtbls();
	printpgs();

	s = BACKING_STORE_BASE + 1 * BACKING_STORE_UNIT_SIZE + 25 * NBPG;
	kprintf("Store value:%c\n", *s);

	return;
}

void proc1_test2(char *msg, int lck) {
	int *x;
	kprintf("ready to allocate heap space\n");
	x = vgetmem(1024);
	kprintf("heap allocated at 0x%x\n", x);
	*x = 150;
	*(x + 10) = 200;

	printbs();
	printdirs();
	printtbls();
	printpgs();

	kprintf("vmemlen:%d\n", proctab[currpid].vmemlist->mnext->mlen);

	kprintf("heap variable: %d %d\n", *x, *(x + 10));

	int *a = getaddr_frm(8);
	kprintf("frm value:%d\n", *(a+10));
	int *s = BACKING_STORE_BASE;
	kprintf("store value:%d\n", *(s+10));

	vfreemem(x, 1024);

	printbs();
	printdirs();
	printtbls();
	printpgs();
	
	kprintf("vmemlen:%d\n", proctab[currpid].vmemlist->mnext->mlen);

	kprintf("heap variable: %d %d\n", *x, *(x + 10));
	
	a = getaddr_frm(8);
	kprintf("frm value:%d\n", *(a+10));
	kprintf("store value:%d\n", *(s+10));

	printbs();
	printdirs();
	printtbls();
	printpgs();
}

void proc1_test3(char *msg, int lck) {

	char *addr;
	int i;

	addr = (char*) 0x0;

	for (i = 0; i < 1024; i++) {
		*(addr + i * NBPG) = 'B';
	}

	for (i = 0; i < 1024; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	return;
}

void proc_kill() {
	get_bs(1, 100);
	xmmap(6000, 1, 100);
	int *a = 6000 * NBPG;
	*(a+1) = 100;
	printbs();
	printdirs();
	printtbls();
	printpgs();
	sleep(3);
}

void test_kill(char *msg, int lck) {
	int pid1, pid2;
	pid1 = vcreate(proc_kill, 2000, 100, 50, "test_kill", 0, NULL);
	resume(pid1);
	kprintf("Killing %d\n", pid1);
	kill(pid1);
	printbs();
	printdirs();
	printtbls();
	printpgs();
	int *a = BACKING_STORE_BASE + 1 * BACKING_STORE_UNIT_SIZE;
	kprintf("Value in backing store:%d\n", *(a+1));
}

void test_invlpg(char *msg, int addr) {
	get_bs(1, 100);
	xmmap(6000, 1, 50);
	printpgs();
	int *a = 6000 * NBPG;
	*a = 1000;
	int *s = BACKING_STORE_BASE + 1 * BACKING_STORE_UNIT_SIZE;
	kprintf("Value: %d\n", *a);
	kprintf("Store value: %d\n", *s);
	printbs();
	printpgs();
	kprintf("Free frame\n");
	free_frm(currpid, 8, 1, 0);
	printbs();
	printpgs();
	kprintf("Store value: %d\n", *s);
	kprintf("Value: %d\n", *a);
}

void virtual_access(char *msg, int addr) {
	xmmap(6000, 1, 50);
	int *a = 6000 * NBPG;
	*a = 2000;
	sleep(10);
}

void test_frmlist(char *msg, int add) {
	add_frmlist(8);
	add_frmlist(8);
	add_frmlist(9);
	add_frmlist(12);
	print_frmlist();
	remove_frmlist(11);
	remove_frmlist(8);
	remove_frmlist(12);
	print_frmlist();
	remove_frmlist(9);
	print_frmlist();
	add_frmlist(8);
	print_frmlist();
}

void test_sc(char *msg) {
	int i;
	fr_map_t *frm;

	srpolicy(SC);

	get_bs(1, 100);
	xmmap(5000, 1, 50);
	int *a = 5000 * NBPG;
	*a = 1;
	int *b = 5001 * NBPG;
	*b = 2;
	int *c = 5002 * NBPG;
	*c = 3;
	printpgs();

	/* replacement happens */
	for (i = 11; i < NFRAMES; i++) {
		frm = &frm_tab[i];
		frm->fr_status = 3;
	}

	print_frmlist();
	int *d = 5003 * NBPG;
	*d = 4;

	kprintf("value in vp 5002 should be 3\n");
	int store, pageth;
	bsm_lookup(currpid, 5002, &store, &pageth);
	int *s = BACKING_STORE_BASE + store * BACKING_STORE_UNIT_SIZE + pageth * NBPG;
	kprintf("value: %d\n", *s);

	print_frmlist();
	int *e = 5004 * NBPG;
	*e = 5;

	*b++;

	print_frmlist();
	int *f = 5005 * NBPG;
	*f = 6;

	print_frmlist();
	int *g = 5006 * NBPG;
	*g = 7;

	printpgs();
}

void test_lfu(char *msg) {
	int i;
	fr_map_t *frm;

	srpolicy(LFU);

	get_bs(1, 100);
	xmmap(5000, 1, 50);

	int *a = 5000 * NBPG;
	*a = 1;
	int *b = 5001 * NBPG;
	*b = 2;
	int *c = 5002 * NBPG;
	*c = 3;
	printpgs();

	frm = &frm_tab[8];
	int store, pageth;
	bsm_lookup(currpid, 5000, &store, &pageth);
	free_frm(currpid, 8, store, pageth);

	*a = 4;

	printpgs();

	/* replacement happens */
	for (i = 11; i < NFRAMES; i++) {
		frm = &frm_tab[i];
		frm->fr_status = 3;
	}
	
	print_frmlist();

	int *d = 5003 * NBPG;
	*d = 5;
	
	print_frmlist();
	int *e = 5004 * NBPG;
	*e = 6;

	printpgs();

	print_frmlist();
	int *f = 5005 * NBPG;
	*f = 7;

	print_frmlist();
	int *g = 5006 * NBPG;
	*g = 8;

	print_frmlist();
	int *h = 5007 * NBPG;
	*h = 8;

	printpgs();
}

void proc1() {
	char *x;
	char temp;
	get_bs(4, 100);
	xmmap(7000, 4, 50);
	x = 7000 * NBPG;
	*x = 'Y';
	temp = *x;
	kprintf("Read from frame:%c\n", temp);
	xmunmap(7000);
	sleep(3);
}

void proc2() {
	char *x;
	char temp;
	xmmap(6000, 4, 80);
	x = 6000 * NBPG;
	temp = *x;
	kprintf("Read from backing store:%c\n", temp);
}

void test_read_write_bs() {
	int pid1;
	int pid2;
	pid1 = vcreate(proc1, 2000, 100, 30, "proc1", 0, NULL);
	pid2 = vcreate(proc2, 2000, 100, 30, "proc2", 0, NULL);
	resume(pid1);
	sleep(2);
	resume(pid2);
}

int main() {
	int pid1;
	int pid2;

	// kprintf("\n1: shared memory\n");
	// pid1 = create(proc1_test1, 2000, 30, "proc1_test1", 0, NULL);
	// resume(pid1);
	// sleep(3);

	// kprintf("\n2: vgetmem/vfreemem\n");
	// pid1 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	// kprintf("pid %d has private heap\n", pid1);
	// resume(pid1);
	// sleep(3);

	// kprintf("\n3: Frame test\n");
	// pid1 = create(proc1_test3, 2000, 20, "proc1_test3", 0, NULL);
	// resume(pid1);
	// sleep(3);

	// kprintf("\n4: test kill\n");
	// pid1 = create(test_kill, 2000, 30, "test_kill", 0, NULL);
	// resume(pid1);
	// kprintf("Killing %d\n", pid1);
	// sleep(3);
	// printbs();
	// printdirs();
	// printtbls();
	// printpgs();

	// kprintf("\n5: test invlpg\n");
	// pid1 = create(test_invlpg, 2000, 30, "test_invlpg", 0, NULL);
	// resume(pid1);
	// sleep(3);

	// kprintf("\n6: test LFU\n");
	// pid1 = vcreate(test_lfu, 2000, 100, 30, "test_lfu", 0, NULL);
	// resume(pid1);
	// sleep(3);

	// kprintf("\n7: test frame lists\n");
	// pid1 = create(test_frmlist, 2000, 30, "test_frmlist", 0, NULL);
	// resume(pid1);
	// sleep(3);

	// kprintf("\n8: second chance\n");
	// pid1 = vcreate(test_sc, 2000, 100, 30, "test_sc", 0, NULL);
	// resume(pid1);
	// sleep(3);

	kprintf("\n9: test read write with backing store\n");
	pid1 = create(test_read_write_bs, 2000, 100, 20, "test_rw_bs", 0, NULL);
	resume(pid1);
	sleep(5);

	shutdown();
}