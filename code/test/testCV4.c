/* testCV4.c
 * Tests the functionality of Broadcast
 * Taken from the TestSuite given for project 1
 */

#include "syscall.h"
int lock;
int CV;

/* --------------------------------------------------
 * t4_waiter()
 *     These threads will wait on the t4_c1 condition variable.  All
 *     t4_waiters will be released
 * --------------------------------------------------
*/
void t4_waiter() {
	Acquire(lock);
	Write("Waiter: Lock acquired, waiting on signaller.\n", sizeof("Waiter: Lock acquired, waiting on signaller.\n"), ConsoleOutput);
	Wait(CV,lock);
	Write("Waiter: Freed from signaller.\n", sizeof("Waiter: Freed from signaller.\n"), ConsoleOutput);
	Release(lock);
	Exit(0);
}


/* --------------------------------------------------
 * t2_signaller()
 *     This thread will broadcast to the t4_c1 condition variable.
 *     All t4_waiters will be released
 * --------------------------------------------------
 */
void t4_signaller() {
	
	Acquire(lock);
	Write("Signaller: Lock acquired, broadcasting\n", sizeof("Signaller: Lock acquired, broadcasting\n"), ConsoleOutput);
	Broadcast(CV,lock);
	Write("Signaller: Releasing lock\n", sizeof("Signaller: Releasing lock\n"), ConsoleOutput);
	Release(lock);
	Exit(0);
}


int main(){
	int i;
	lock = CreateLock(0);
	CV = CreateCondition(0);
	for (i = 0; i < 5; i++){
		Fork(t4_waiter);
	}
	Fork(t4_signaller);
}