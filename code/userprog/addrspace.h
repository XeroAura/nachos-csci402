// addrspace.h
//  Data structures to keep track of executing user programs
//  (address spaces).
//
//  For now, we don't keep any information about address spaces.
//  The user level CPU state is saved and restored in the thread
//  executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "table.h"

#define MaxOpenFiles 256
#define MaxChildSpaces 256

#ifdef CHANGED
#define MaxThreadsPerProccess 50
#define UserStackSize (SectorSize* MaxThreadsPerProccess) //1024 increase this as necessary!
#endif

extern BitMap* memoryBitMap;
extern Lock* bitMapLock;

class PageTableEntry {
  public:
    int virtualPage;    // The page number in virtual memory.
    int physicalPage;   // The page number in real memory (relative to the
            //  start of "mainMemory"
    bool valid;         // If this bit is set, the translation is ignored.
            // (In other words, the entry hasn't been initialized.)
    bool readOnly;  // If this bit is set, the user program is not allowed
            // to modify the contents of the page.
    bool use;           // This bit is set by the hardware every time the
            // page is referenced or modified.
    bool dirty;         // This bit is set by the hardware every time the
            // page is modified.
    int diskLocation; //0 - exec, 1 - swap, 2 - neither
    int offset; //Byte offset of page in file
};

class AddrSpace {
    public:
    AddrSpace(OpenFile *executable);    // Create an address space,
    // initializing it with the program
    // stored in the file "executable"
    ~AddrSpace();          // De-allocate an address space
    
    void InitRegisters();       // Initialize user-level CPU registers,
    // before jumping to user code
    
    void SaveState();           // Save/restore address space-specific
    void RestoreState();        // info on a context switch
    Table fileTable;            // Table of openfiles

    #ifdef CHANGED
    int AllocatePages(); //Returns start address of 8 pages for stack
    void EmptyPages(); //Dumps the pages for process exit
    void Empty8Pages(int startPage); //Dumps the 8 pages for thread exit
    
    int executablePageCount; //Counter for which page its on
    OpenFile* file;
    unsigned int numPages;      // Number of pages in the virtual
    // address space
    unsigned int codeSize;
    unsigned int dataSize;

    Lock* pageTableLock; //Lock for pageTable
    PageTableEntry *pageTable;    // Assume linear page table translation
    // for now!'

    private:
    BitMap* pageBitMap; //Create new bitmap and lock to keep track of open physical pages
    Lock* pageBitMapLock; //Lock for page bitmap
    #endif
};

#endif // ADDRSPACE_H
