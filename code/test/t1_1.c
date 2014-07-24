


#include "syscall.h"

int main(){
	int index;
	int i;
	index = CreateLock("t1_l1",sizeof("t1_l1"));
	Acquire(index);
	Write("t1_t1: Acquired Lock 1, waiting for t3\n", sizeof("t1_t1: Acquired Lock 1, waiting for t3\n"), ConsoleOutput);
	Write("t1_t1: working in CS\n",sizeof("t1_t1: working in CS\n"), ConsoleOutput);
	for (i = 0; i < 1000000; i++) ;
		Write("t1_t1: Releasing Lock 1\n", sizeof("t1_t1: Releasing Lock 1\n"), ConsoleOutput);
	Release(index);
	DestroyLock(index);
	Exit(0);

}