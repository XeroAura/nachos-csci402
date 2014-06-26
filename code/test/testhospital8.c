/* Test hospital file
 * 
*/

#include "syscall.h"
#include "testhospital.c"

 int main() {
 	Setup();
 	
 	recCount = 1;
 	docCount = 1;
 	doorBoyCount = 1;
 	cashierCount = 1;
 	clerkCount = 1;
 	numPatients = 5;

 	InitializeThreads();
 }
