/* Test hospital file
 * 
*/

#include "syscall.h"
#include "testhospital.c"

 int main() {
 	Setup();
 	
 	recCount = 5;
 	docCount = 5;
 	doorBoyCount = 5;
 	cashierCount = 5;
 	clerkCount = 5;
 	numPatients = 20;

 	InitializeThreads();
 }
