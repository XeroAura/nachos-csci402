/* testfork3.c
 *	Test what happens when you hit 50 threads for the process (limit).
 */

#include "syscall.h"

void test(){
  Write("test\n", 5, ConsoleOutput);
  Exit(0);
}

int main() {
	int i = 0;
	for(i = 0; i<50; i++){
 		MyWrite("Creating test%d\n", sizeof("Creating test%d\n"), i*100, 0);
		Fork(test);
	}
}

