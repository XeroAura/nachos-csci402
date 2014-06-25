/* testCV3.c
 * File used to show that Signal only wakes one thread
 * Taken from the TestSuite provided for project 1. 
 */
#include "syscall.h"

/* --------------------------------------------------
 * t3_waiter()
 *     These threads will wait on the t3_c1 condition variable.  Only
 *     one t3_waiter will be released
 * --------------------------------------------------
*/
int lock;
int CV;

void t3_waiter() {
	Acquire(lock);
	Write("Waiter: Lock acquired, waiting on signaller.\n", sizeof("Waiter: Lock acquired, waiting on signaller.\n"), ConsoleOutput);
	Wait(CV,lock);
	Write("Waiter: Freed from signaller.\n", sizeof("Waiter: Freed from signaller.\n"), ConsoleOutput);
	Release(lock);
	Exit(0);
}


/* --------------------------------------------------
 * t3_signaller()
 *     This threads will signal the t3_c1 condition variable.  Only
 *     one t3_signaller will be released
 * --------------------------------------------------
 */
void t3_signaller() {

	Acquire(lock);
	Write("Signaller: Lock acquired, signalling waiter\n", sizeof("Signaller: Lock acquired, signalling waiter\n"), ConsoleOutput);
	Signal(CV,lock);
	Write("Signaller: Releasing lock\n", sizeof("Signaller: Releasing lock\n"), ConsoleOutput);
	Release(lock);
	Exit(0);
}

int main(){
	int i;
	lock = CreateLock(0);
	CV = CreateCondition(0);
	for (i = 0; i < 5; i++){
		Fork(t3_waiter);
	}
	Fork(t3_signaller);
	Yield();

}