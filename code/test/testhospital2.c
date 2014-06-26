/* Test hospital file
 * 
*/

#include "syscall.h"
#include "testhospital.c"

int main() {
	Setup();

	recCount = 2;
	cashierCount = 2;
	clerkCount = 2;

	testNum = 2;
	docCount = 1;
	doorBoyCount = 3;
	numPatients = 5;

	InitializeThreads();

}
