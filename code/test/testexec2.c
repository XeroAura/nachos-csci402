/* testexec2.c
 * Test reaching limit of processes (10)
 */

#include "syscall.h"

int main() {
	Exec("../test/testfiles", sizeof("../test/testfiles"));
	Exec("../test/testfiles", sizeof("../test/testfiles"));
	Exec("../test/testfiles", sizeof("../test/testfiles"));
	Exec("../test/testfiles", sizeof("../test/testfiles"));
	Exec("../test/testfiles", sizeof("../test/testfiles"));
	Exec("../test/testfiles", sizeof("../test/testfiles"));
	Exec("../test/testfiles", sizeof("../test/testfiles"));
	Exec("../test/testfiles", sizeof("../test/testfiles"));
	Exec("../test/testfiles", sizeof("../test/testfiles"));
	Exec("../test/testfiles", sizeof("../test/testfiles"));
 	Exec("../test/testfiles", sizeof("../test/testfiles"));
}

