/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include<paging.h>
/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
		// All frames which currently hold any of its pages should be written to the backing store and be freed.
	int i;
	for(i =0; i<NFRAMES; i++){
		if(frm_tab[i].fr_pid == pid){
			if(frm_tab[i].fr_type == FR_PAGE){
				int store;
				int pageth;
				bsm_lookup(pid, frm_tab[i].fr_vpno*NBPG, &store, &pageth);
				write_bs((char*)((FRAME0+i)*NBPG), store, pageth);
				free_frm(i);
			}	
		}
	}
	// All of its mappings should be removed from the backing store map.
	for(i = 0; i<8; i++){
		if(bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_pid[pid] == 1){
			bsm_tab[i].bs_pid[pid] = -1;
            bsm_tab[i].bs_vpno[pid] = 0;
            bsm_tab[i].bs_npages[pid] = 0;
		}
	}
	// The backing stores for its heap (and stack if have chosen to implement a private stack) should be released.
	release_bs(pptr->store);
	// The frame used for the page directory should be released
	free_frm((pptr->pdbr/NBPG) - FRAME0);

	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
