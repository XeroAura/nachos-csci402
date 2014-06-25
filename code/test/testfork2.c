/* testfork2.c
 *	Test to see if fork handles incorrect input gracefully.
 */

#include "syscall.h"


int main() {
	Fork(-1);
	Fork("test");
	while(1){	
		Yield();
	}
}

