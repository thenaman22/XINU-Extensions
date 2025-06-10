/* ready.c - ready */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>

/*------------------------------------------------------------------------
 * ready  --  make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
int ready(int pid, int resch)
{
	register struct	pentry	*pptr;

	if (isbadpid(pid))
		return(SYSERR);
	
	pptr = &proctab[pid];
	pptr->pstate = PRREADY;
	
	if(getschedclass() == LINUXSCHED){						// This is to make sure we are using Goodness factor to order the queue, rather than using the priority when shcedclass = LINUXSCHED
		pptr->goodness = pptr->counter + pptr->epoch_prio;   // Setting its goodness in case it has been woken up from sleep!
		insert(pid,rdyhead,pptr->goodness);
	}
	else
		insert(pid,rdyhead,pptr->pprio);
	
	if (resch)
		resched();
	return(OK);
}