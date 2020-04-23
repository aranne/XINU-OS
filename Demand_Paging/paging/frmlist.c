#include <conf.h>
#include <kernel.h>
#include <paging.h>

frm_list frmlist;       /* frmlist of pages for replacement policy */
node* sc_curr;          /* current node for second chance policy */

SYSCALL init_frmlist(void) {
    frmlist = create_frmlist();
    sc_curr = frmlist.sentinel;
}

node* new_node(node *prev, int frmno, node *next) {
    node *n = getmem(sizeof(node));
    n->prev = prev;
    n->frmno = frmno;
    n->next = next;
    return n;
}

void free_node(node *n) {
    freemem(n, sizeof(node));
}

frm_list create_frmlist(void) {
    frm_list list;
    node* n = new_node(NULL, -1, NULL);
    n->prev = n;
    n->next = n;
    list.size = 0;
    list.sentinel = n;
    return list;
}

SYSCALL add_frmlist(int frmno) {
    node* sen = frmlist.sentinel;
    node *p = sen->next;
    while (p != sen) {
        if (p->frmno == frmno) {
            return OK;
        }
        p = p->next;
    }
    node* n = new_node(sen, frmno, sen->next);
    sen->next->prev = n;
    sen->next = n;
    frmlist.size++;
    return OK;
}

SYSCALL remove_frmlist(int frmno) {
    node* sen = frmlist.sentinel;
    node* q = sen;
    node* p = sen->next;
    while (p->frmno != -1) {
        if (p->frmno == frmno) {
            q->next = p->next;
            free_node(p);
            break;
        } else {
            q = p;
            p = p->next;
        }
    }
    frmlist.size--;
    return OK;
}

void print_frmlist(void) {
    node *sen = frmlist.sentinel;
    node *p = sen->next;
    kprintf("frmlist--");
    while (p != sen) {
        kprintf("frm: %d, ", p->frmno);
        p = p->next;
    }
    kprintf("\n");
}