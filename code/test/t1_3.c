
#include "syscall.h"
int main(){
	int i;
	for ( i = 0; i < 3; i++ ) {
		Write("t1_t3: Trying to release lock 1\n", sizeof("t1_t3: Trying to release lock 1\n"), ConsoleOutput);
		Release(0);
	}
	Exit(0);
}