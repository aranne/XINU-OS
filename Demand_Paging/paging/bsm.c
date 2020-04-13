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
        bsm->size = 0;
        bsm->bs_sem = 0;
    }
    return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
    int i;
    for (i = 0; i < NBS; i++) {
        if (bsm_tab[i].bs_status == BSM_UNMAPPED) {
            *avail = i;
            return OK;
        }
    }
    return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
    if (isbadbs(i)) 
        return SYSERR;
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
    bsm->size = 0;
    bsm->bs_sem = 0;
    return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
    if (isbadpid(pid) || vaddr < 0 || vaddr % NBPG != 0) {
        return SYSERR;
    }
    int vp_no = vaddr / NBPG;
    bs_map_t *bsm;
    bs_vp_t *bsvp;
    int i;
    for (i = 0; i < NBS; i++) {
        bsm = &bsm_tab[i];
        bsvp = bsm->bs_vp;
        while (bsvp->bs_pid != -1) {
            if (bsvp->bs_pid == pid && vp_no >= bsvp->bs_vpno && vp_no < bsvp->bs_vpno + bsvp->bs_npages) {
                *store = i;
                *pageth = vp_no - bsvp->bs_vpno;
                return OK;
            }
            bsvp = bsvp->nextvp;
        }
    }
    return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *      flag: 1--private or 0--shared
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages, int flag)
{
    if (isbadbs(source) || isbadpid(pid) || vpno < 0 || npages < 0) {
        return SYSERR;
    }
    bs_map_t *bsm = &bsm_tab[source];
    bs_vp_t *bsvp;
    int max_pages = get_bs(source, npages);
    if (max_pages < npages) {
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
        bsm->size = npages;
        return OK;
    } else if (bsm->bs_status == BSM_SHARED) {
        bsvp = (bs_vp_t*) getmem(sizeof(bs_vp_t));
        bsvp->bs_pid = pid;
        bsvp->bs_vpno = vpno;
        bsvp->bs_npages = npages;
        bsvp->nextvp = bsm->bs_vp;
        bsm->bs_vp = bsvp;
        return OK;
    } else {
        return SYSERR;
    }
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *       flag: 1--private, 0--shared
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
    if (isbadpid(pid) || vpno < 0) {
        return SYSERR;
    }
    bs_map_t *bsm;
    bs_vp_t *bsvp, *prev;
    int store, pageth;
    bsm_lookup(pid, vpno * NBPG, &store, &pageth);
    bsm = &bsm_tab[store];
    if (flag == 1 && bsm->bs_status == BSM_PRIVATE) {
        release_bs(store);
    } else if (flag == 0 && bsm->bs_status == BSM_SHARED) {
        prev = NULL;
        bsvp = bsm->bs_vp;
        while(bsvp->bs_pid != -1) {
            if (bsvp->bs_pid == pid) {
                if (prev != NULL) {
                    prev->nextvp = bsvp->nextvp;
                    freemem((struct mblock*)bsvp, sizeof(bs_vp_t));
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
    } else if (bsm->bs_status == BSM_UNMAPPED) {
        return OK;
    } else {
        return SYSERR;
    }
}


