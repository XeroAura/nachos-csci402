/* testlock2.c
 * File using different threads in order to test effectiveness of acquire and release
 * Based on the testSuite given for project 1
 */

#include "syscall.h"
	int testLock1;
/* --------------------------------------------------
 * t1_t1() -- test1 thread 1
 *   This is the rightful lock owner
 * --------------------------------------------------
*/
void t1_t1() {
	int i;
	Acquire(testLock1);	
	Yield();  
	MyWrite("Thread 1: Acquired Lock %d, waiting for t3\n", sizeof("Thread 1: Acquired Lock %d, waiting for t3\n"), testLock1*100,0);
	Yield();
	Write("Thread 1: working in CS\n", sizeof("Thread 1: working in CS\n"), ConsoleOutput);
	for (i = 0; i < 1000; i++) ;
	MyWrite("Thread 1: Releasing Lock %d \n", sizeof("Thread 1: Releasing Lock %d \n"), testLock1*100, 0);
	Release(testLock1);
	Exit(0);
}

/* --------------------------------------------------
 * t1_t2() -- test1 thread 2
 *     This thread will wait on the held lock.
 * --------------------------------------------------
 */
void t1_t2() {
	int i;	

	Yield();
	MyWrite("Thread 2: Trying to acquire lock %d\n", sizeof("Thread 2: Trying to acquire lock %d\n"), testLock1*100,0);	
	Acquire(testLock1);	
	MyWrite("Thread 2: Acquired Lock %d, working in CS\n", sizeof("Thread 2: Acquired Lock %d, working in CS\n"), testLock1*100,0);
	for (i = 0; i < 10; i++)
		;
	MyWrite("Thread 2: Releasing Lock %d\n", sizeof("Thread 2: Releasing Lock %d\n"), testLock1*100,0);
	Release(testLock1);
	Exit(0);
}

/* --------------------------------------------------
 * t1_t3() -- test1 thread 3
 *     This thread will try to release the lock illegally
 * --------------------------------------------------
 */
void t1_t3() {
	int i;
	Yield();
	for ( i = 0; i < 3; i++ ) {
		MyWrite("Thread 3: Trying to release lock %d\n", sizeof("Thread 3: Trying to release lock %d\n"), testLock1*100,0);
		Release(testLock1);
	}
	Exit(0);
}

int main(){
	Write("Beginning lock test 2\n", sizeof("Beginning lock test 2\n"), ConsoleOutput);
	testLock1 = CreateLock(1);

	Fork(t1_t1);
	Fork(t1_t2);
	Fork(t1_t3);
}