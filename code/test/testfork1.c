/* testfork1.c
 *	Simple program to test the fork system call.
 */

#include "syscall.h"

void test1(){
  Write("test1\n", 6, ConsoleOutput);
  Exit(0);
}

void test2(){
  Write("test2\n", 6, ConsoleOutput);
  Exit(0);
}

int main() {
  Fork(test1);
  Fork(test2);
}

