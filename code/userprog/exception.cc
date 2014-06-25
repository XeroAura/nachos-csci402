 // exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synch.h"
#include "addrspace.h"
#include <stdio.h>
#include <iostream>

using namespace std;
int nextLockIndex = 0;
Lock* lockTableLock = new Lock("lockTableLock");
int nextCVIndex = 0;
Lock* CVTableLock = new Lock("CVTableLock");
extern ProcessEntry* processTable[10];
extern int processTableCount;
extern Lock* processTableLock;
extern const int MAX_CVS;
extern const int MAX_LOCKS;
extern KernelLock* kLocks[];
extern KernelCV* kCV[]; 

#ifdef CHANGED
bool
validateAddress(unsigned int vaddr){ //Validates a virtual address to be within bounds and not NULL
    if(vaddr == NULL)
        return false;
    int size = currentThread->space->numPages * PageSize;
    if( vaddr > 0 && vaddr < (unsigned int) (size-1) ){ //Check if vaddr is within bounds?
        return true;
    }
    return false;
}

bool validateBuffer(unsigned int vaddr, int len){ //Verifies a virtual address for a buffer is within bounds for its length and not null
    if(vaddr == NULL)
        return false;
    int size = currentThread->space->numPages * PageSize;
    if(vaddr > 0 && vaddr+len < (unsigned int)(size-1)){
        return true;
    }
    return false;
}
#endif


int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
	bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
    	result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
      {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
   		}	

   		buf[n++] = *paddr;

   		if ( !result ) {
	//translation failed
   			return -1;
   		}

   		vaddr++;
   	}

   	delete paddr;
   	return len;
   }

   int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
   	bool result; 
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
    	result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

    	if ( !result ) {
	//translation failed
    		return -1;
    	}

    	vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    #ifdef CHANGED
    if(!validateBuffer(vaddr, len)){
        printf("Bad buffer vaddr or length.\n");
        return;
    }
    #endif
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
    	printf("%s","Bad pointer passed to Create\n");
    	delete buf;
    	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    #ifdef CHANGED
    if(!validateBuffer(vaddr, len)){
        printf("Bad buffer vaddr or length.\n");
        return -1;
    }
    #endif
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
    	printf("%s","Can't allocate kernel buffer in Open\n");
    	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
    	printf("%s","Bad pointer passed to Open\n");
    	delete[] buf;
    	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
    	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
    		delete f;
    	return id;
    }
    else
    	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    #ifdef CHANGED
    if(!validateBuffer(vaddr, len)){
        printf("Bad buffer vaddr or length.\n");
        return;
    }
    #endif
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.

    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
    	printf("%s","Error allocating kernel buffer for write!\n");
    	return;
    } else {
    	if ( copyin(vaddr,len,buf) == -1 ) {
    		printf("%s","Bad pointer passed to to write: data not written\n");
    		delete[] buf;
    		return;
    	}
    }

    if ( id == ConsoleOutput) {
    	for (int ii=0; ii<len; ii++) {
    		printf("%c",buf[ii]);
    	}

    } else {
    	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
    		f->Write(buf, len);
    	} else {
    		printf("%s","Bad OpenFileId passed to Write\n");
    		len = -1;
    	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    #ifdef CHANGED
    if(!validateBuffer(vaddr, len)){
        printf("Bad buffer vaddr or length.\n");
        return -1;
    }
    #endif
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
    	printf("%s","Error allocating kernel buffer in Read\n");
    	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
    	scanf("%s", buf);

    	if ( copyout(vaddr, len, buf) == -1 ) {
    		printf("%s","Bad pointer passed to Read: data not copied\n");
    	}
    } else {
    	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
    		len = f->Read(buf, len);
    		if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
    			if ( copyout(vaddr, len, buf) == -1 ) {
    				printf("%s","Bad pointer passed to Read: data not copied\n");
    			}
    		}
    	} else {
    		printf("%s","Bad OpenFileId passed to Read\n");
    		len = -1;
    	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
	OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

	if ( f ) {
		delete f;
	} else {
		printf("%s","Tried to close an unopen file\n");
	}
}

#ifdef CHANGED

struct forkInfo{
	int vaddr;
	int pageAddr;
    forkInfo():vaddr(0), pageAddr(0){}
};

void fork_thread(int value){
	forkInfo *m = (forkInfo*) value;
	int vaddr = m->vaddr;
	int pageLoc = m->pageAddr;

    machine->WriteRegister(PCReg, vaddr);
    machine->WriteRegister(NextPCReg, vaddr+4);
	//Write to stack register the starting point of the stack for this thread
	machine->WriteRegister(StackReg, pageLoc); //numPages * PageSize - 16
	currentThread->space->RestoreState();
	
    machine->Run();
}

void Fork_Syscall(unsigned int vaddr){
    // printf("Vaddr: %d\n", vaddr);
    if(!validateAddress(vaddr)){
        printf("Bad vaddr passed to fork.\n");
        return;
    }
	Thread* t = new Thread("forkThread"); //Create new thread
	t->space = currentThread->space; //Allocate the addrspace to the thread being forked, same as current thread's
	forkInfo* tmp = new forkInfo();
	tmp->vaddr = vaddr;
	//Find 8 pages of stack to give to thread?
    int pageAddr = t->space->AllocatePages();
    if(pageAddr != -1) {
        tmp->pageAddr = pageAddr;
        //Multiprogramming: Update process table
        ThreadEntry* te = new ThreadEntry(); //Give first stack page?
        te->myThread = t;
        te->firstStackPage = pageAddr;
        processTableLock->Acquire();
        for(int i = 0; i < 10; i++){
            if(processTable[i]->as == currentThread->space){
                processTable[i]->threads[processTable[i]->threadCount] = te;
                processTable[i]->threadCount++;
                break;
            }
        }
        processTableLock->Release();
        t->Fork((VoidFunctionPtr) fork_thread, (int) tmp);
    }
    else
        printf("Unable to allocate pages for stack in Fork.\n");

}

struct execInfo{
    int pageAddr;
    execInfo(): pageAddr(-1){}
};

void exec_thread(int value){
    execInfo *m = (execInfo*) value;

	//Initialize the register by using currentThread->space.
    machine->WriteRegister(PCReg, 0);
    machine->WriteRegister(NextPCReg, 4);

    //Write to stack register the starting point of the stack for this thread
    machine->WriteRegister(StackReg, m->pageAddr); //numPages * PageSize - 16

    currentThread->space->RestoreState();
    machine->Run();
}

void Exec_Syscall(unsigned int vaddr, int size){
    if(!validateAddress(vaddr)){
        printf("Bad vaddr passed to exec.\n");
        return;
    }
    processTableLock->Acquire();
    if(processTableCount > 10){
        printf("Too many processes for any more to be made!\n");
        processTableLock->Release();
        return;
    }
    processTableLock->Release();

    OpenFile *f;
    char *buf = new char[size+1];    // Kernel buffer to put the name in
    if (!buf) {
        printf("%s","Can't allocate kernel buffer in Open\n");
        return;
    }
    if( copyin(vaddr,size,buf) == -1 ) {
        printf("%s","Bad pointer passed to Open\n");
        delete[] buf;
        return;
    }

    buf[size]='\0';

    // printf("%s\n", buf);
    f = fileSystem->Open(buf);
    if (f == NULL) {
        printf("Unable to open file %s\n", buf);
        return;
    }
    delete[] buf;
    

	AddrSpace *space = new AddrSpace(f); // Create new addresspace for this executable file.

    space->file = f; // Store its openfile pointer


	Thread *t = new Thread(""); //Create a new thread
	t->space = space; // Allocate the space created to this thread's space.

	// Update the process table and related data structures.
    processTableLock->Acquire();

    ThreadEntry* te = new ThreadEntry();

    int pageAddr = space->AllocatePages();
    if(pageAddr == -1){
        printf("Unable to allocate stack pages for exec. \n");
        return;
    }
    te->firstStackPage = pageAddr;
    te->myThread = t;
    ProcessEntry* pe = new ProcessEntry();
    pe->threadCount = 1;
    pe->as = space;
    pe->threads[0] = te;
    processTable[processTableCount] = pe;
    processTableCount++;
    processTableLock->Release();

    execInfo* tmp = new execInfo();
    tmp->pageAddr = pageAddr;

    t->Fork(exec_thread, (int) tmp);
}

void Yield_Syscall(){
	currentThread->Yield();
}

int CreateLock_Syscall(int debugInt){
    char* debugName = new char[10];
    sprintf(debugName, "Lock %d",debugInt);
    KernelLock* tempLock = new KernelLock;
    tempLock->as = currentThread->space;
    tempLock->isToBeDestroyed = false;
    tempLock->lock = new Lock(debugName);
    lockTableLock->Acquire();
    if (nextLockIndex < MAX_LOCKS){
        kLocks[nextLockIndex] = tempLock;
        nextLockIndex++;
    } else {
        printf("ERROR: Maximum number of locks reached. Current number of locks is %d. \n", nextLockIndex);
    }
    lockTableLock->Release();
    return nextLockIndex-1;
}

void DestroyLock_Syscall(int index){
    if (index >= MAX_LOCKS){
        printf("ERROR: The attempted index exceeds the maximum number of locks. \n");
        return;
    }
    if (index < 0){
        printf("ERROR: The entered index is below 0. \n");
        return;
    }
    if (kLocks[index]->lock == NULL){
        printf("ERROR: No lock exists here.\n");
        return;
    }
    kLocks[index]->isToBeDestroyed = true;
    if (kLocks[index]->isToBeDestroyed && kLocks[index]->lock->getFree()){
      delete kLocks[index]->lock;
      delete kLocks[index];
      kLocks[index] = NULL;
  }
  return;
}

void Acquire_Syscall(int index){
    printf("Acquiring lock %d\n", index);
    if (index >= 0 && index < MAX_LOCKS){
        kLocks[index]->lock->Acquire();
    } else {
        printf("ERROR: Index exceeds bounds.\n");
    }
    return;
}

void Release_Syscall(int index){
    printf("Releasing lock %d\n", index);
    if (index >= 0 && index < MAX_LOCKS){ //checks if index is valid
        if (kLocks[index]->lock != NULL){ //checks if the lock exists
            kLocks[index]->lock->Release();
            if (kLocks[index]->lock->getFree() && kLocks[index]->isToBeDestroyed){
                DestroyLock_Syscall(index);
            }
        } else {
            printf("ERROR: No lock exists.\n");
        }
    } else {
        printf("ERROR: Index exceeds bounds.\n");
    }
    return;
}


int CreateCondition_Syscall(int debugInt){
    char* debugName = new char[20];
    sprintf(debugName, "Condition %d",debugInt);
    KernelCV* tempCV = new KernelCV;
    tempCV->as = currentThread->space;
    tempCV->isToBeDestroyed = false;
    tempCV->condition = new Condition(debugName);
    CVTableLock->Acquire();
    if (nextCVIndex < MAX_LOCKS){
        kCV[nextCVIndex] = tempCV;
        nextCVIndex++;
    } else {
        printf("ERROR: Maximum number of CV's reached. \n");
    }
    CVTableLock->Release();
    return nextCVIndex-1;
}

void DestroyCondition_Syscall(int index){
    if (index >= MAX_CVS){
        printf("ERROR: The entered index exceeds the maximum allowed locks. \n");
        return;
    }
    if (index < 0){
        printf("ERROR: The entered index is below 0. \n");
        return;
    }
    if (kCV[index]->condition == NULL){
        printf("ERROR: No condition exists here.\n");
        return;
    }
    kCV[index]->isToBeDestroyed = true;
    if (kCV[index]->isToBeDestroyed && kCV[index]->condition->getLock() == NULL){
      delete kCV[index]->condition;
      delete kCV[index];
      kCV[index] = NULL;
  }
  return;
}

void Wait_Syscall(int index, int lockIndex){
    if (index >= 0 && index < MAX_LOCKS){
        KernelLock* cvLock = kLocks[lockIndex];
        kCV[index]->condition->Wait(cvLock->lock);
    } else {
        printf("ERROR: Index exceeds bounds.\n");
    }
    return;
}

void Signal_Syscall(int index, int lockIndex){
    if (index >= 0 && index < MAX_LOCKS){
        KernelLock* cvLock = kLocks[lockIndex];
        if (cvLock->lock != NULL){
            kCV[index]->condition->Signal(cvLock->lock);
        }
    } else {
        printf("ERROR: Index exceeds bounds.\n");
    }
    return;
}

void Broadcast_Syscall(int index, int lockIndex){
    if (index >= 0 && index < MAX_LOCKS){
        KernelLock* cvLock = kLocks[lockIndex];
        kCV[index]->condition->Broadcast(cvLock->lock);
    } else {
        printf("ERROR: Index exceeds bounds.\n");
    }
    return;
}

void MyWrite_Syscall(unsigned int vaddr, int len, int one, int two){
    char *buf = new char[len+1];    // Kernel buffer to put the name in
    if (!buf) return;
    if( copyin(vaddr,len,buf) == -1 ) {
        printf("%s","Bad pointer passed to Create\n");
        delete buf;
        return;
    }
    buf[len]='\0';

    int a = one/100;
    int b = one%100;
    int c = two/100;
    int d = two%100;
    printf(buf, a, b, c, d);
    return;
}


int Exit_Syscall(){
    currentThread->Finish();
    // 1. Last thread in the last process, just call halt
    int count = 0;
    int slot = 0;
    processTableLock->Acquire();
    for(int i = 0; i < 10; i++){
        if(processTable[i]->threadCount > 0)
            count++;
        if(processTable[i]->as == currentThread->space)
            slot = i;
    }
    if(count == 1){
        interrupt->Halt();
    }

    // 2. Last thread in a process - not the last process
    //     a. Reclaim all memory (Use pageTable - All valid entries)
    //          memoryBitMap->Clear(ppn);
    //     b. Reclaim all locks/CVs that were allocated to that process

    if(processTable[slot]->threadCount == 1){ //Last thread in process
        for (int i = 0; i < MAX_LOCKS; i++){
            if (kLocks[i]->as == processTable[slot]->as){
                DestroyLock_Syscall(i);
                CreateLock_Syscall(i);            
            }
        }

        for (int i = 0; i < MAX_CVS; i++){
            if (kCV[i]->as == processTable[slot]->as){
                DestroyCondition_Syscall(i);
                CreateCondition_Syscall(i);            
            }
        }
        processTable[slot]->as->EmptyPages();
        ProcessEntry* blank = new ProcessEntry();
        processTable[slot] = blank;
        //Clear locks/CVS here!
        currentThread->Finish();
        return 0;
    }
    
    // 3. Not last thread in a process
    //     a. Reclaim 8 stack pages
    for(int j = 0; j< processTable[slot]->threadCount; j++){
        if(currentThread == processTable[slot]->threads[j]->myThread){
            processTable[slot]->as->Empty8Pages(processTable[slot]->threads[j]->firstStackPage);
            processTable[slot]->threads[j] = new ThreadEntry();
            currentThread->Finish();
            return 1;
            break;
        }
    }

    printf("Exit syscall called with no acceptable results.\n");
    return -1;
}



#endif

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
    	switch (type) {
    		default:
    		DEBUG('a', "Unknown syscall - shutting down.\n");
            break;
            case SC_Halt:
            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;
            case SC_Create:
            DEBUG('a', "Create syscall.\n");
            Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;
            case SC_Open:
            DEBUG('a', "Open syscall.\n");
            rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;
            case SC_Write:
            DEBUG('a', "Write syscall.\n");
            Write_Syscall(machine->ReadRegister(4),
             machine->ReadRegister(5),
             machine->ReadRegister(6));
            break;
            case SC_Read:
            DEBUG('a', "Read syscall.\n");
            rv = Read_Syscall(machine->ReadRegister(4),
             machine->ReadRegister(5),
             machine->ReadRegister(6));
            break;
            case SC_Close:
            DEBUG('a', "Close syscall.\n");
            Close_Syscall(machine->ReadRegister(4));
            break;

		#ifdef CHANGED
            case SC_MyWrite:
            DEBUG('a', "MyWrite syscall.\n");
            MyWrite_Syscall(machine->ReadRegister(4), machine->ReadRegister(5),machine->ReadRegister(6), machine->ReadRegister(7));
            break;

    		case SC_Fork:
    		DEBUG('a', "Fork syscall.\n");
    		Fork_Syscall(machine->ReadRegister(4));
    		break;

    		case SC_Exec:
    		DEBUG('a', "Exec syscall.\n");
    		Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
    		break;

    		case SC_Exit:
    		DEBUG('a', "Exit syscall.\n");
    		rv = Exit_Syscall();
    		break;

    		case SC_Yield:
    		DEBUG('a', "Yield syscall.\n");
    		Yield_Syscall();
    		break;

            case SC_CreateLock:
            DEBUG('a', "Create Lock syscall.\n");
            rv = CreateLock_Syscall(machine->ReadRegister(4));
            break;

            case SC_DestroyLock:
            DEBUG('a', "Destroy Lock syscall.\n");
            DestroyLock_Syscall(machine->ReadRegister(4));            
            break;

            case SC_Acquire:
            DEBUG('a', "Lock Acquire syscall.\n");
            Acquire_Syscall(machine->ReadRegister(4));
            break;

            case SC_Release:
            DEBUG('a', "Lock Release syscall.\n");
            Release_Syscall(machine->ReadRegister(4));
            break;

            case SC_CreateCondition:
            DEBUG('a', "Create Condition syscall.\n");
            rv = CreateCondition_Syscall(machine->ReadRegister(4));
            break;

            case SC_DestroyCondition:
            DEBUG('a', "Destroy Condition syscall.\n");
            DestroyCondition_Syscall(machine->ReadRegister(4));
            break;

            case SC_Wait:
            DEBUG('a', "Wait syscall.\n");
            Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;

            case SC_Signal:
            DEBUG('a', "Signal syscall.\n");
            Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;

            case SC_Broadcast:
            DEBUG('a', "Broadcast syscall.\n");
            Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;
	    #endif
        }

	// Put in the return value and increment the PC
        machine->WriteRegister(2,rv);
        machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
        machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
        machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
        return;
    } else {
    	cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
    	interrupt->Halt();
    }
}
