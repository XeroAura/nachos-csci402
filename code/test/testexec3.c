/* testexec3.c
 *
 */

#include "syscall.h"

 int main() {
 	Exec("../test/testfile", sizeof("../test/testfile"));
 	Exec("../test/testfiles", -1);
 	Exec("../test/testfiles", 9999);
 	
}

