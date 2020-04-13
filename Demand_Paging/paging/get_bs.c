#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

    /* requests a new mapping of npages with ID map_id */
    if (npages == 0 || npages > MNBSPG || isbadbs(bs_id)) {
      return SYSERR;
    }
    bs_map_t *bsm = &bsm_tab[bs_id];
    if (bsm->bs_status == BSM_UNMAPPED) {             /* UNMAPPED */
      return npages;
    } else if (bsm->bs_status == BSM_SHARED) {        /* SHARED   */                   
        return bsm->size;
    } else {                                         /* PRIVATE  */
      return SYSERR;
    }
}


