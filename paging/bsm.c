/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

bs_map_t bsm_tab[8];
/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
    STATWORD ps;
    disable(ps);
    int i,j;
    for(i = 0; i<8; i++){
        bsm_tab[i].bs_status = BSM_UNMAPPED;
        bsm_tab[i].bs_heap = 0;
        for(j = 0; j<NPROC; j++){
            bsm_tab[i].bs_pid[j] = -1;
            bsm_tab[i].bs_vpno[j] = 0;
            bsm_tab[i].bs_npages[j] = 0;
        }
    }
    restore(ps);
    return SYSERR;
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
    for(i = 0; i < 8; i++){
        if(bsm_tab[i].bs_status == BSM_UNMAPPED){
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
    release_bs(i);
    restore(ps);
    return SYSERR;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
    STATWORD ps;
	disable(ps);
    unsigned long vp = vaddr>>12;
    int i;
    bs_map_t* bptr;
    for(i = 0; i<8; i++){
        bptr = &bsm_tab[i];
        if(bptr->bs_status == BSM_MAPPED && bptr->bs_pid[pid] == 1){
            if(bptr->bs_vpno[pid] <= vp && vp < bptr->bs_vpno[pid]+bptr->bs_npages[pid]){
                *store = i;
                *pageth = vp - bptr->bs_vpno[pid];
                restore(ps);
                return OK;
            }
        }
    }
    restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
    STATWORD ps;
    disable(ps);
    bs_map_t* bptr;
    bptr = &bsm_tab[source];
    if(bptr->bs_heap){
        restore(ps);
        return SYSERR;
    }
    else{
        bptr->bs_status = BSM_MAPPED;
        bptr->bs_pid[pid] = 1;
        bptr->bs_vpno[pid] = vpno;
        bptr->bs_npages[pid] = npages;
    }
    restore(ps);
    return OK;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */

SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
    STATWORD ps;
    disable(ps);
    bs_map_t* bptr;
    int source;
    int pageth;
    if(bsm_lookup(pid, vpno*4096, &source, &pageth) == SYSERR){
        restore(ps);
        return SYSERR;
    }

    bptr = &bsm_tab[source];

    if(bptr->bs_status == BSM_MAPPED && bptr->bs_pid[pid] && bptr->bs_heap){
        bptr->bs_status = BSM_UNMAPPED;
        bptr->bs_heap = 0;
        bptr->bs_pid[pid] = 0;
    }
    
    bptr->bs_pid[pid] = 0;
    bptr->bs_vpno[pid] = 0;
    bptr->bs_npages[pid] = 0;
    
    restore(ps);
    return OK;
}


