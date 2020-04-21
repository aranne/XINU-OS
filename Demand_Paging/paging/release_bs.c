#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {
  STATWORD ps;
  disable(ps);

  /* release the backing store with ID bs_id */
  if (isbadbs(bs_id)) {
    restore(ps);
    return SYSERR;
  }
  bs_map_t *bsm = &bsm_tab[bs_id];
  if (bsm->bs_status == BSM_UNMAPPED) {
    free_bsm(bs_id);
    restore(ps);
    return OK;
  } else if (bsm->bs_status == BSM_SHARED) {
    if (bsm->bs_vp->bs_pid == -1) {      // no process is mapped to this store.
      free_bsm(bs_id);
      restore(ps);
      return OK;
    } else {
      restore(ps);
      return SYSERR;
    }
  } else {
    if (bsm->bs_vp->bs_pid == currpid) {
      free_bsm(bs_id);
      restore(ps);
      return OK;
    } else {
      restore(ps);
      return SYSERR;
    }
  }
}

