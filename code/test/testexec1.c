/* testexec1.c
 * Basic test to see if exec works.
 */

#include "syscall.h"

int main() {
 	Exec("../test/testfiles", sizeof("../test/testfiles"));
}

