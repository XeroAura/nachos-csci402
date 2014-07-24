// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "IPTEntry.h"
#include <list>

using namespace std;

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.
class AddrSpace;
class Lock;
class Condition;
extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#ifdef CHANGED
class AddrSpace;
class Lock;

struct ThreadEntry { //Struct to represent a thread
    int firstStackPage;
    Thread* myThread;
    ThreadEntry() : firstStackPage(-1), myThread(NULL) {};
}; 

struct ProcessEntry { //Struct to represent a process
    int threadCount;
    int threadTotal;
    AddrSpace* as;
    ThreadEntry* threads[50];
    ProcessEntry() : threadCount(0), threadTotal(0), as(NULL) {
        for(int i = 0; i < 50; i++)
            threads[i] = new ThreadEntry();
    }
};

struct KernelLock{
    char* name;
    Lock* lock;
    int requestThreads;
    AddrSpace* as;
    bool isToBeDestroyed;
    KernelLock() : name(NULL),lock(NULL),requestThreads(0), as(NULL),isToBeDestroyed(false) {};
};


struct KernelCV{
    char* name;
    Condition* condition;
    int requestThreads;
    AddrSpace* as;
    bool isToBeDestroyed;
    KernelCV() : name(NULL), condition(NULL), requestThreads(0), as(NULL), isToBeDestroyed(false) {};
};

struct KernelMV{
    char* name;
    int* values;
    int maxValue;
    int requestThreads;
    AddrSpace* as;
    bool isToBeDestroyed;
    KernelMV() : name(NULL), values(NULL), maxValue(NULL), requestThreads(0), as(NULL), isToBeDestroyed(NULL) {};
};

struct ServerLock{
    int state; //0 = available, 1 = busy
    int owner;
    List* queue;
    ServerLock() : state(0),owner(0),queue(NULL) {};
};

struct ServerCV{
    int lock; //ServerLock table index
    List* queue;
    ServerCV() : lock(-1),queue(NULL) {};
};

extern ProcessEntry* processTable[10]; //Process table
extern int processTableCount;
extern Lock* processTableLock; //Lock for process table
extern const int MAX_CVS;
extern const int MAX_LOCKS;
extern KernelLock* kLocks[];
extern KernelCV* kCV[]; 

extern int currentTLB;
extern IPTEntry *ipt;
extern Lock* IPTLock;

extern int evictMethod; //0 - RAND, 1 - FIFO
extern OpenFile* swapFile;
extern BitMap* swapBitMap;
extern Lock* swapLock;
extern std::list<int> *fifoQueue;
extern Lock* fifoLock;

extern KernelMV* MVArray[500];
extern int mailboxID;

#endif

#endif // SYSTEM_H
