/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void test(){
  Write("test\n", 5, ConsoleOutput);
  Exit(0);
}

int main() {
    Write("testa\n", 6, ConsoleOutput);
	Fork((void*) test);
    Write("testb\n", 6, ConsoleOutput);
}

