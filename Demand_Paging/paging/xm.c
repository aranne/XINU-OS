/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  if (virtpage < 0 || isbadbs(source) || npages < 0) {
    return SYSERR;
  } 
  return bsm_map(currpid, virtpage, source, npages, 0);
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  if (virtpage < 0) {
    return SYSERR;
  }
  return bsm_unmap(currpid, virtpage, 0);
}
