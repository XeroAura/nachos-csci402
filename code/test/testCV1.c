/* testCV1.c
 * File used to test user CV functionality
 */

 #include "syscall.h"

void test1(){
	Acquire(0);
	MyWrite("Thread 1: Attempting to Wait on CV -1\n", sizeof("Thread 1: Attempting to Wait on CV -1\n"), 0,0);
	Wait(-1, 0);
	MyWrite("Thread 1: Attempting to Wait on CV 0\n", sizeof("Thread 1: Attempting to Wait on CV 0\n"), 0,0);
	Wait(0,0);
	Exit(0);
}

void test2(){
	Write("Thread 2: Successfully moved to next thread\n", sizeof("Thread 2: Successfully moved to next thread\n"),ConsoleOutput);
	Exit(0);
}

int main(){
	int maxCV;
	CreateLock(0);
	Write("Making CV's...\n", sizeof("Making CV's...\n"),ConsoleOutput);
	for (maxCV = 0; maxCV < 255; maxCV++){
		CreateCondition(maxCV);
	}
	Fork(test1);
	Fork(test2);
	Yield();

	Write("Destroying CV's...\n", sizeof("Destroying CV's...\n"), ConsoleOutput);
	for (maxCV = 0; maxCV < 255; maxCV++){
		DestroyCondition(maxCV);
	}

	Write("CV's destroyed.\n", sizeof("CV's destroyed.\n"), ConsoleOutput);
}