/* Test hospital file
 * 
*/

#include "syscall.h"
#include "testhospital.c"

 int main() {

	testNum = 8; 	
 	recCount = 5;
 	docCount = 5;
 	doorBoyCount = 5;
 	cashierCount = 5;
 	clerkCount = 5;
 	numPatients = 20;

 	
 	Setup();

 	InitializeThreads();
 }
