/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD ps;
	disable(ps);
	if(hsize<= 0 || hsize>256){
		restore(ps);
		return SYSERR;
	}
	struct pentry* pptr;
	int heap_store;
	if(get_bsm(&heap_store)== SYSERR)
	{	
		restore(ps);
		return SYSERR;
	}
	
	int pid;
	if((pid = create(procaddr,ssize,priority,name,nargs,args)) == SYSERR){
		restore(ps);
		return SYSERR;
	}
	pptr = &proctab[pid];
	if(bsm_map(pid,4096,heap_store,hsize) == SYSERR){
		restore(ps);
		return SYSERR;
	}
	bsm_tab[heap_store].bs_heap = 1;		
	pptr->store=heap_store;
	pptr->vhpno=4096;
	pptr->vhpnpages=hsize;
	pptr->vmemlist->mnext = NBPG*4096;
			
	struct mblock *backin_store = BACKING_STORE_BASE+(heap_store*BACKING_STORE_UNIT_SIZE);
	backin_store->mlen=hsize*NBPG;
	backin_store->mnext=NULL;

	restore(ps);
	return pid;
}
/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
