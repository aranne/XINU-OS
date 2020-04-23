/* paging.h */

typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_avail : 3;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_avail : 3;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct{
  unsigned int pg_offset : 12;		/* page offset			*/
  unsigned int pt_offset : 10;		/* page table offset		*/
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

typedef struct bv_t{
  int bs_pid;        /* processes id using this slot */
  int bs_vpno;       /* starting virtual page number */
  int bs_npages;     /* number of pages in the store */
  struct bv_t *nextvp;/* next pair of virtual page and bs */
} bs_vp_t;

typedef struct{
  int bs_status;			/* SHARED or PRIVATE or UNMAPPED		*/
  bs_vp_t *bs_vp;      /* a pair of bs and virtual page */
  int bs_size;            /* number of pages in a backing store */
  int bs_sem;				  /* semaphore mechanism ?	*/
} bs_map_t;

typedef struct{
  int fr_status;			/* MAPPED or UNMAPPED		*/
  int fr_pid;				/* process id using this frame: for FR_DIR & FR_PAGE */
  int fr_vpno;				/* corresponding virtual page no: for FR_PAGE */
  int fr_refcnt;			/* lifetime reference count: for LFU policy */
  int fr_pgcnt;       /* page count in a page table: for FR_TBL */
  int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
  int fr_dirty;       /* 1--dirty, 0--not dirty: for FR_PAGE */
}fr_map_t;

typedef struct _node {
    int frmno;
    struct _node *next;
    struct _node *prev;
} node;


typedef struct {
    node *sentinel;
    int size;
} frm_list;

extern frm_list frmlist;
node* new_node(node *prev, int frmno, node *next); 
void free_node(node *n);
frm_list create_frmlist(void);
SYSCALL add_frmlist(int frmno);
SYSCALL remove_frmlist(int frmno);
void print_frmlist(void);
SYSCALL init_frmlist(void);

extern bs_map_t bsm_tab[];
extern fr_map_t frm_tab[];
extern bs_vp_t  bsvp_tab[];
/* Prototypes for required API calls */
SYSCALL xmmap(int virtpage, bsd_t source, int npages);
SYSCALL xmunmap(int virtpage);

/* given calls for dealing with backing store */

int get_bs(bsd_t bs_id, unsigned int npages);
SYSCALL release_bs(bsd_t bs_id);
SYSCALL read_bs(char *dst, bsd_t bs_id, int page);
int write_bs(char *src, bsd_t bs_id, int page);

/* given calls for dealing with backing store mapping */
SYSCALL init_bsm(void);
SYSCALL get_bsm(int* avail);
SYSCALL free_bsm(int i);
SYSCALL bsm_lookup(int pid, int vpno, int* store, int* pageth);
SYSCALL bsm_map(int pid, int vpno, int source, int npages, int flag);
SYSCALL bsm_unmap(int pid, int vpno);

/* given calls for dealing with frame mapping */
SYSCALL init_frm();
SYSCALL get_frm(int* avail);
SYSCALL evict_frm(int *avail);
SYSCALL free_frm(int pid, int frmno, int store, int pageth);
SYSCALL lookup_frm(int pid, int vpno, int* frmno);
SYSCALL clear_frm(int i);
char* getaddr_frm(int frmno);

/* given calls for dealing with paging */
SYSCALL create_pd(int pid);
SYSCALL set_globe_ptbls(void);
SYSCALL create_pt(int pid, int frame);

/* given calls for replacement policy */
SYSCALL srpolicy(int policy);
SYSCALL grpolicy(void);
extern int rpdebug;                 /* debug mode */

void printtbls(void);
void printdirs(void);
void printpgs(void);
void printbs(void);

#define IVNPF  14   /* page fault interrupt vector number */ 

#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/
#define NFRAMES 	1024	/* number of frames		*/

#define NBS       16    /* number of backing stores */
#define MNBSPG    128   /* max number of pages in a backing store */

#define NGPTBL    4      /* number of global page tables */
#define NPTBL     1024  /* max number of page tables in a directory */
#define NPAGE     1024   /* number of pages in a page table */

/* for create() or vcreate() process */
#define HASVIRTUALHEAP 1
#define NOVIRTUALHEAP  0
#define VHSNO         4096 /* virtual heap starting page number */

#define BSM_UNMAPPED	0
#define BSM_SHARED	  1
#define BSM_PRIVATE   2

#define FRM_UNMAPPED	0
#define FRM_MAPPED	1

#define FR_PAGE		0
#define FR_TBL		1
#define FR_DIR		2

#define SC 3
#define LFU 4

#define BACKING_STORE_BASE	0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00080000

#define FRAME_BASE 0x00400000
#define FRAME_UNIT_SIZE 0x00001000

#define isbadbs(id) (id<0 || id>=NBS)
#define isbadfrm(id) (id<0 || id>=NFRAMES)
#define isglobaltbl(id) (id==1025 || id==1026 || id==1027 || id==1028)
