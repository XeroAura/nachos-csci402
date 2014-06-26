/* Test hospital file
 * 
*/

#include "syscall.h"
#include "testhospital.c"

int main() {

	Setup();

	docCount = 2;
	doorBoyCount = 2;
	numPatients = 5;

	recCount = 0;
	cashierCount = 0;
	clerkCount = 0;
	testNum = 4;

	InitializeThreads();

}
