/* testexec3.c
 *
 */

#include "syscall.h"

 int main() {
  	Write("before testfiles\n", sizeof("before testfiles"), ConsoleOutput);
 	Exec("../test/testfiles", sizeof("../test/testfiles"));
  	Write("after testfiles\n", sizeof("after testfiles"), ConsoleOutput);
}

