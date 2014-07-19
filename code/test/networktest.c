/* networktest.c
 *	Simple program to test the networking system calls
 */

#include "syscall.h"


int main() {
	CreateLock("Lock1",sizeof("Lock1"));
	Acquire(0);
	Exit(0);
}

