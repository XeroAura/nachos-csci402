/* testfork1.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void test(){
  Write("test\n", 5, ConsoleOutput);
  Exit(0);
}

int main() {
  Fork(test);
}

