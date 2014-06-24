/* testlocks.c
 * A test for the lock functionality. Taken in part from the testSuite used in project 1.
*/

 #include "syscall.h"

int lockArray[255];
void t1_t1(){
  Acquire(0);
  Acquire(0);
  Release(1);
  Acquire(1);
  Release(1);
  Release(-1);
  Release(255);
  Acquire(-1);
  Acquire(255);
  Release(0);
  Exit(0);
}

void t1_t2(){
}

int main(){
	int maxLockCount;
	Write("making 255 locks...\n", 20, ConsoleOutput);
	for (maxLockCount = 0; maxLockCount < 255; maxLockCount++){
		lockArray[maxLockCount] = CreateLock(maxLockCount);
	}
  Fork(t1_t1);
  Yield();

	maxLockCount = 0;
	Write("erasing locks...\n", 18, ConsoleOutput);
	for(maxLockCount = 0; maxLockCount < 255; maxLockCount++){
		DestroyLock(maxLockCount);
    }
}