/* Test hospital file
 * 
*/

#include "syscall.h"
#include "testhospital.c"

 int main() {
 	Setup();

 	recCount = 2;
 	docCount = 2;
 	doorBoyCount = 2;
 	cashierCount = 2;
 	clerkCount = 2;
 	numPatients = 5;

 	testNum = 7;
 	InitializeThreads();
 }
