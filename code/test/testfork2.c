/* testfork2.c
 *	Test to see if fork handles incorrect input gracefully.
 */

#include "syscall.h"


int main() {
	Fork((void*)-1);
	Fork((void*)9999);
	Fork((void*)"test"); 
	while(1){	
		Yield();
	}
}

