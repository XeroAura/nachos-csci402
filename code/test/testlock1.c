/* testlock1.c
 * A test for the lock functionality. Taken in part from the testSuite used in project 1.
*/

 #include "syscall.h"

int lockArray[255];

void test(){
  Write("Attempting to acquire an invalid lock\n", sizeof("Attempting to acquire an invalid lock\n"), ConsoleOutput);
  Acquire(-1);
  Write("Attempting to acquire lock 0\n", sizeof("Acquiring lock 0\n"), ConsoleOutput);
  Acquire(0);
  Write("Attempting to acquire lock 0 again\n", sizeof("Attempting to acquire lock 0 again\n"), ConsoleOutput);
  Acquire(0);
  Write("Attempting to release an unowned lock\n", sizeof("Attempting to release an unowned lock\n"), ConsoleOutput);
  Release(1);
  Release(0);
  Exit(0);
}

int main(){
	int maxLockCount;
	Write("making 255 locks...\n", 20, ConsoleOutput);
	for (maxLockCount = 0; maxLockCount < 255; maxLockCount++){
		lockArray[maxLockCount] = CreateLock(maxLockCount);
	}
  Fork(test);
  Yield();

	maxLockCount = 0;
	Write("erasing locks...\n", 18, ConsoleOutput);
	for(maxLockCount = 0; maxLockCount < 255; maxLockCount++){
		DestroyLock(maxLockCount);
    }
}