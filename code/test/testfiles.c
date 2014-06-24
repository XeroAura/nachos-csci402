/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"


void t1_t1(){
  Acquire(0);
  Yield();
  Write("1: Releasing lock.\n", 19, ConsoleOutput);
  Release(0);

}

void t1_t2(){
  Write("2: Attempting to acquire lock.\n", 31, ConsoleOutput);
  Acquire(0);
  Write("2: Lock acquired.\n", 18, ConsoleOutput);
  Release(0);
}

void test(){

  Write("Test.\n", 6, ConsoleOutput);
  
  Exit(0);
}

int main() {
  
  Write("Test.\n", 6, ConsoleOutput);

/*

  OpenFileId fd;
  OpenFileId fd2;
  int bytesread;
  char buf[20];

  int maxLockCount;

    Create("testfile", 8);
    fd = Open("testfile", 8);

    Write("testing a write\n", 16, fd );
    Close(fd);


    fd = Open("testfile", 8);
    bytesread = Read( buf, 100, fd );
    Write( buf, bytesread, ConsoleOutput );
    Close(fd);

    Write("making locks...\n", 16, ConsoleOutput);

  for (maxLockCount = 0; maxLockCount < 250; maxLockCount++){
    CreateLock(maxLockCount);
  }

  t1_t1();
  t1_t2();



  maxLockCount = 0;
    Write("erasing locks...\n", 18, ConsoleOutput);
    for(maxLockCount = 0; maxLockCount < 250; maxLockCount++){
      DestroyLock(maxLockCount);
    }
    */
}

