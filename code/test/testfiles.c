/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int main() {
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

  for (maxLockCount = 0; maxLockCount < 255; maxLockCount++){
    CreateLock(maxLockCount);
  }
  maxLockCount = 0;
    Write("erasing locks...\n", 17, ConsoleOutput);
    for(maxLockCount = 0; maxLockCount < 252; maxLockCount++){
      DestroyLock(maxLockCount);
    }

    Write("locks erased.\n", 14, ConsoleOutput);



}

