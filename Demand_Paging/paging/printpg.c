#include <conf.h>
#include <kernel.h>
#include <paging.h>

void printtbls() {
    int i;
    fr_map_t *frm;
    for (i = 0; i < NFRAMES; i++) {
        frm = &frm_tab[i];
        if (frm->fr_status == FRM_MAPPED && frm->fr_type == FR_TBL) {
            kprintf("TBL--id:%d, pgcnt:%d, frm:%d, refcnt:%d\n", frm->fr_pid, frm->fr_pgcnt, i, frm->fr_refcnt);
        }
    }
}

void printdirs() {
    int i;
    fr_map_t *frm;
    for (i = 0; i < NFRAMES; i++) {
        frm = &frm_tab[i];
        if (frm->fr_status == FRM_MAPPED && frm->fr_type == FR_DIR) {
            kprintf("DIR--id:%d, frm:%d, refcnt:%d\n", frm->fr_pid, i, frm->fr_refcnt);
        }
    }
}

void printpgs() {
    int i;
    fr_map_t *frm;
    for (i = 0; i < NFRAMES; i++) {
        frm = &frm_tab[i];
        if (frm->fr_status == FRM_MAPPED && frm->fr_type == FR_PAGE) {
            kprintf("PG--id:%d, vp:%d, frm:%d, refcnt:%d\n", frm->fr_pid, frm->fr_vpno, i, frm->fr_refcnt);
        }
    }
}