/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock1;
int lock2;
int cv;

/* --------------------------------------------------
 * t5_t1() -- test 5 thread 1
 *     This thread will wait on a condition under t5_l1
 * --------------------------------------------------
 */
void t5_t1() {
	Acquire(lock1);
	Write("Thread 1: Lock 1 acquired, waiting on CV\n", sizeof("Thread 1: Lock 1 acquired, waiting on CV\n"), ConsoleOutput);
	Wait(cv,lock1);
	Write("Thread 1: Releasing Lock 1\n", sizeof("Thread 1: Releasing Lock 1\n"), ConsoleOutput);
	Release(lock1);
	Exit(0);
}

/* --------------------------------------------------
 * t5_t1() -- test 5 thread 1
 *     This thread will wait on a t5_c1 condition under t5_l2, which is
 *     a Fatal error
 * --------------------------------------------------
 */
void t5_t2() {
	Acquire(lock1);
	Acquire(lock2);
	Write("Thread 2: Lock 2 acquired, signalling CV\n", sizeof("Thread 2: Lock 2 acquired, signalling CV\n"), ConsoleOutput);
	Signal(cv, lock2);
	Write("Thread 2: Releasing Lock 2\n", sizeof("Thread 2: Releasing Lock 2\n"), ConsoleOutput);
	Release(lock2);
	Write("Thread 2: Releasing Lock 1\n", sizeof("Thread 2: Releasing Lock 1\n"), ConsoleOutput);
	Release(lock1);
	Exit(0);
}


int main() {
	Write("Starting Test 5. Note that it is an error if thread t5_t1 completes\n",
	sizeof("Starting Test 5. Note that it is an error if thread t5_t1 completes\n"), 
	ConsoleOutput);
	lock1 = CreateLock(0);
	lock2 = CreateLock(1);
	cv = CreateCondition(0);

	Fork(t5_t1);
	Fork(t5_t2);
	Yield();
}

