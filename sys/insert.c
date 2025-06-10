/* insert.c  -  insert */

#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <sched.h>

/*------------------------------------------------------------------------
 * insert.c  --  insert an process into a q list in key order
 *------------------------------------------------------------------------
 */
int insert(int proc, int head, int key)
{
	int	next;			/* runs through list		*/
	int	prev;

	next = q[head].qnext;
	if(getschedclass() == EXPDISTSCHED)
		while (q[next].qkey <= key)	/* tail has maxint as key	*/        /*Since we are picking the first process that gives us prio greater than the random number, we insert it last to preserve round robin*/
			next = q[next].qnext;
	else
		while (q[next].qkey < key)	/* tail has maxint as key	*/        /*In this case, I have not implemented any technique, it just picks the last process using getlast() function, hence we insert it at the first location to preserve RR*/
			next = q[next].qnext;
	q[proc].qnext = next;
	q[proc].qprev = prev = q[next].qprev;
	q[proc].qkey  = key;
	q[prev].qnext = proc;
	q[next].qprev = proc;
	return(OK);
}