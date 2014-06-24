/* testlocks.c
 * A test for the lock functionality. Taken in part from the testSuite used in project 1.
*/

 #include "syscall.h"

void t1_t1(){
  Acquire(0);
  Acquire(0);
  Release(1);
  Acquire(1);
  Release(1);
  Release(-1);
  Acquire(-1);
  Release(0);

}

void t1_t2(){
}

int main(){
	int maxLockCount;
	Write("making locks...\n", 16, ConsoleOutput);
	for (maxLockCount = 0; maxLockCount < 255; maxLockCount++){
		CreateLock(maxLockCount);
	}
	t1_t1();

	maxLockCount = 0;
	Write("erasing locks...\n", 18, ConsoleOutput);
	for(maxLockCount = 0; maxLockCount < 255; maxLockCount++){
		DestroyLock(maxLockCount);
    }
}