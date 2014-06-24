/* testCV.c
 * File used to test user CV functionality
 */

 #include "syscall.h"

void test1(){
	Acquire(0);
	Write("Negative Wait\n", sizeof("Negative Wait\n"),ConsoleOutput);
	Wait(-1, 0);
	Write("Positive Wait\n", sizeof("Positive Wait\n"),ConsoleOutput);
	Wait(0,0);
	Exit(0);
}

void test2(){
	Write("Things worked.\n", sizeof("Things worked.\n"),ConsoleOutput);
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
	DestroyCondition(0);
	DestroyCondition(1);
	Write("CV's destroyed.\n", sizeof("CV's destroyed.\n"), ConsoleOutput);
}