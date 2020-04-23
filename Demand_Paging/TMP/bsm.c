/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

bs_map_t bsm_tab[NBS];

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
    STATWORD ps;
    disable(ps);

    bs_map_t *bsm;
    bs_vp_t *bsvp;
    int i;
    for (i = 0; i < NBS; i++) {
        bsm = &bsm_tab[i];
        bsvp = (bs_vp_t*) getmem(sizeof(bs_vp_t));
        bsvp->bs_pid = -1;
        bsvp->bs_vpno = 0;
        bsvp->bs_npages = 0;
        bsvp->nextvp = (struct bv_t *)NULL;
        bsm->bs_status = BSM_UNMAPPED;
        bsm->bs_vp = bsvp;
        bsm->bs_size = 0;
        bsm->bs_sem = 0;
    }

    restore(ps);
    return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
    STATWORD ps;
    disable(ps);

    int i;
    for (i = 0; i < NBS; i++) {
        if (bsm_tab[i].bs_status == BSM_UNMAPPED) {
            *avail = i;
            restore(ps);
            return OK;
        }
    }
    restore(ps);
    return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
    STATWORD ps;
    disable(ps);

    if (isbadbs(i)) {
        restore(ps);
        return SYSERR;
    }
    bs_map_t *bsm = &bsm_tab[i];
    bs_vp_t *bsvp, *next;
    bsvp = bsm->bs_vp;
    while (bsvp->bs_pid != -1) {
        next = bsvp->nextvp;
        freemem((struct mblock*)bsvp, sizeof(bs_vp_t));
        bsvp = next;
    }
    bsm->bs_status = BSM_UNMAPPED;
    bsm->bs_vp = bsvp;
    bsm->bs_size = 0;
    bsm->bs_sem = 0;

    restore(ps);
    return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, int vpno, int* store, int* pageth)
{
    STATWORD ps;
    disable(ps);

    if (isbadpid(pid) || vpno < VHSNO) {
        restore(ps);
        return SYSERR;
    }
    bs_map_t *bsm;
    bs_vp_t *bsvp;
    int i;
    for (i = 0; i < NBS; i++) {
        bsm = &bsm_tab[i];
        bsvp = bsm->bs_vp;
        while (bsvp->bs_pid != -1) {
            if (bsvp->bs_pid == pid && vpno >= bsvp->bs_vpno && vpno < bsvp->bs_vpno + bsvp->bs_npages) {
                *store = i;
                *pageth = vpno - bsvp->bs_vpno;
                restore(ps);
                return OK;
            }
            bsvp = bsvp->nextvp;
        }
    }
    restore(ps);
    return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *      flag: 1--private or 0--shared
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages, int flag)
{
    STATWORD ps;
    disable(ps);

    if (isbadbs(source) || isbadpid(pid) || vpno < VHSNO || npages < 0) {
        restore(ps);
        return SYSERR;
    }
    bs_map_t *bsm = &bsm_tab[source];
    bs_vp_t *bsvp;
    int max_pages = get_bs(source, npages);
    if (max_pages < npages) {
        restore(ps);
        return SYSERR;
    }
    if (bsm->bs_status == BSM_UNMAPPED) {
        if (flag == 1) {
            bsm->bs_status = BSM_PRIVATE;
        } else {
            bsm->bs_status = BSM_SHARED;
        }
        bsvp = (bs_vp_t*) getmem(sizeof(bs_vp_t));
        bsvp->bs_pid = pid;
        bsvp->bs_vpno = vpno;
        bsvp->bs_npages = npages;
        bsvp->nextvp = bsm->bs_vp;
        bsm->bs_vp = bsvp;
        restore(ps);
        return OK;
    } else if (bsm->bs_status == BSM_SHARED) {
        bsvp = (bs_vp_t*) getmem(sizeof(bs_vp_t));
        bsvp->bs_pid = pid;
        bsvp->bs_vpno = vpno;
        bsvp->bs_npages = npages;
        bsvp->nextvp = bsm->bs_vp;
        bsm->bs_vp = bsvp;
        restore(ps);
        return OK;
    } else {
        restore(ps);
        return SYSERR;
    }
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *       flag: 1--private, 0--shared
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno)
{
    STATWORD ps;
    disable(ps);

    if (isbadpid(pid) || vpno < VHSNO) {
        restore(ps);
        return SYSERR;
    }
    bs_map_t *bsm;
    bs_vp_t *bsvp, *prev;
    int store, pageth;
    int frmno;
    if (bsm_lookup(pid, vpno, &store, &pageth) == SYSERR) {
        restore(ps);
        return SYSERR;
    }
    bsm = &bsm_tab[store];
    if (bsm->bs_status != BSM_UNMAPPED) {
        prev = NULL;
        bsvp = bsm->bs_vp;
        while(bsvp->bs_pid != -1) {
            if (bsvp->bs_pid == pid && vpno >= bsvp->bs_vpno && vpno < bsvp->bs_vpno + bsvp->bs_npages) {
                int vp = bsvp->bs_vpno;
                int i;
                for (i = 0; i < bsvp->bs_npages; i++) {
                    if (lookup_frm(pid, vp, &frmno) == OK) {
                        free_frm(pid, frmno, store, pageth + i);
                    }
                    vp++;
                }
                if (prev != NULL) {
                    prev->nextvp = bsvp->nextvp;
                    freemem((struct mblock *)bsvp, sizeof(bs_vp_t));
                    bsvp = prev->nextvp;
                } else {
                    bs_vp_t *next = bsvp->nextvp;
                    freemem((struct mblock*)bsvp, sizeof(bs_vp_t));
                    bsm->bs_vp = next;
                    bsvp = next;
                }
            } else {
                prev = bsvp;
                bsvp = prev->nextvp;
            }
        }
        release_bs(store);
        restore(ps);
        return OK;
    } else {
        restore(ps);
        return OK;
    }
}



void printbs() {
    int i;
    bs_map_t *bsm;
    for (i = 0; i < NBS; i++) {
        bsm = &bsm_tab[i];
        if (bsm->bs_status != BSM_UNMAPPED) {
            bs_vp_t *bsvp = bsm->bs_vp;
            if (bsm->bs_status == BSM_PRIVATE) {
                kprintf("BS--private-");
            } else {
                kprintf("BS--shared-");
            }
            kprintf("store:%d, size:%d, ", i, bsm->bs_size);
            while (bsvp->bs_pid != -1) {
                kprintf("id:%d, vp:%d, npages:%d |", bsvp->bs_pid, bsvp->bs_vpno, bsvp->bs_npages);
                bsvp = bsvp->nextvp;
            }
            kprintf("\n");
        }
    }
}