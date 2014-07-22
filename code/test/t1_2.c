
#include "syscall.h"

int main(){
	int index;
	int i;
	index = CreateLock("Lock1",sizeof("Lock1"));
	Write("t1_t2: trying to acquire lock 1\n",sizeof("t1_2: trying to acquire lock 1\n"), ConsoleOutput);
	Acquire(0);
	
	Write("t1_t2:Acquired Lock 1, working in CS\n", sizeof("t1_t2:Acquired Lock 1, working in CS\n"), ConsoleOutput);
	for (i = 0; i < 10; i++)
		;
	Write("t1_t2: Releasing Lock 1\n",sizeof("t1_t2: Releasing Lock 1\n"), ConsoleOutput);
	Release(0);
	DestroyLock(0);
	Exit(0);
}