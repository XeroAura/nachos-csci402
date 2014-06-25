/* testCV2.c
 * CV test that shows that signals are not stored
 * Taken from the second test in the TestSuite provided for project 1
 */

#include "syscall.h"
/*--------------------------------------------------
 *t2_t1() -- test 2 thread 1
 *    This thread will signal a variable with nothing waiting
 *--------------------------------------------------
 */
int lock1;
int CV1;

void t2_t1() {
	Acquire(lock1);
	Write("Thread 1: Lock acquired, signalling thread 2\n", sizeof("Thread 1: Lock acquired, signalling thread 2\n"), ConsoleOutput);
	Signal(CV1,lock1);
	Write("Thread 1: Releasing Lock\n", sizeof("Thread 1: Releasing Lock\n"), ConsoleOutput);
	Release(lock1);
	Exit(0);
}

/* --------------------------------------------------
 * t2_t2() -- test 2 thread 2
 *     This thread will wait on a pre-signalled variable
 * --------------------------------------------------
 */
void t2_t2() {

	Acquire(lock1);
	Write("Thread 2: Lock acquired, waiting on CV\n", sizeof("Thread 2: Lock acquired, waiting on CV\n"), ConsoleOutput);
	Wait(CV1, lock1);
	Write("Thread 2: Releasing Lock\n", sizeof("Thread 2: Releasing Lock\n"), ConsoleOutput);
	Release(lock1);
	Exit(0);
}

int main(){
	lock1 = CreateLock(1);
	CV1 = CreateCondition(1);
	Fork(t2_t1);
	Fork(t2_t2);
	Yield();
	DestroyLock(lock1);
	DestroyCondition(CV1);
}
