// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
#include "synch.h"

extern "C" { int bzero(char *, int); };

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
	table = new void *[size];
	lock = new Lock("TableLock");
}

Table::~Table() {
	if (table) {
		delete table;
		table = 0;
	}
	if (lock) {
		delete lock;
		lock = 0;
	}
}

void *Table::Get(int i) {
	// Return the element associated with the given if, or 0 if
	// there is none.
	
	return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
	// Put the element in the table and return the slot it used.  Use a
	// lock so 2 files don't get the same space.
	int i;	// to find the next slot
	
	lock->Acquire();
	i = map.Find();
	lock->Release();
	if ( i != -1)
		table[i] = f;
	return i;
}

void *Table::Remove(int i) {
	// Remove the element associated with identifier i from the table,
	// and return it.
	
	void *f =0;
	
	if ( i >= 0 && i < size ) {
		lock->Acquire();
		if ( map.Test(i) ) {
			map.Clear(i);
			f = table[i];
			table[i] = 0;
		}
		lock->Release();
	}
	return f;
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

#ifdef CHANGED
//Stuff for page table
BitMap* memoryBitMap = new BitMap(NumPhysPages); //Create new bitmap and lock to keep track of open physical pages
Lock* bitMapLock = new Lock("bitMapLock");

AddrSpace::AddrSpace(OpenFile *executable) : fileTable(MaxOpenFiles) {
	NoffHeader noffH;
	unsigned int i;
	unsigned int size;

	pageBitMap = new BitMap(50);
	pageBitMapLock = new Lock("pageBitMapLock");

	pageTableLock = new Lock("pageTableLock");

	// Don't allocate the input or output to disk files
	fileTable.Put(0);
	fileTable.Put(0);
	
	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
	
	if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
		SwapHeader(&noffH);
	ASSERT(noffH.noffMagic == NOFFMAGIC);
	codeSize = noffH.code.size;
	dataSize = noffH.initData.size + noffH.uninitData.size;
	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
	executablePageCount = divRoundUp(noffH.code.size + noffH.initData.size, PageSize);
	numPages = divRoundUp(size, PageSize) + 400; //<-- added in this semicolon  //divRoundUp(UserStackSize,PageSize);
	// printf("NumPages: %d\n", numPages);
	// we need to increase the size
	// to leave room for the stack
	size = numPages * PageSize;
	
	//ASSERT(numPages <= NumPhysPages);	// check we're not trying
	// to run anything too big --
	// at least until we have
	// virtual memory
	
	DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);
	// first, set up the translation
	pageTableLock->Acquire();
	pageTable = new PageTableEntry[numPages];
	for (i = 0; i < numPages; i++) {
		
		//bitMapLock->Acquire();
		//int ppn = memoryBitMap->Find(); //Use BitMap Find to get an unused page of memory
		//bitMapLock->Release();
		// if(ppn == -1){ //No open pages
		// 	printf("Out of physical pages to add to page table. \n");
		// }
		// else{
			pageTable[i].virtualPage = i;
			//pageTable[i].physicalPage = ppn;
			pageTable[i].valid = FALSE; //TRUE
			pageTable[i].use = TRUE;
			pageTable[i].dirty = FALSE;
			pageTable[i].readOnly = FALSE;
			pageTable[i].diskLocation = 2;
			if(i < executablePageCount)
				pageTable[i].diskLocation = 0;
			pageTable[i].offset = 40+i*PageSize;

			// IPTLock->Acquire();
			// ipt[ppn].virtualPage = i;
			// ipt[ppn].physicalPage = ppn;
			// ipt[ppn].valid = TRUE;
			// ipt[ppn].use = TRUE;
			// ipt[ppn].dirty = FALSE;
			// ipt[ppn].readOnly = FALSE;
			// ipt[ppn].as = this;
			// IPTLock->Release();

			//executable->ReadAt(&(machine->mainMemory[ppn*PageSize]), PageSize, 40+i*PageSize); //Read in executable
			
		}
		
	// }
	pageTableLock->Release();
	
	// then, copy in the code and data segments into memory
	// Copy from the executable to the memory, one page at a time
	// if (noffH.code.size > 0) {
	//     DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", noffH.code.virtualAddr, noffH.code.size);
	//     executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]), noffH.code.size, noffH.code.inFileAddr);
	// }
	// if (noffH.initData.size > 0) {
	//     DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", noffH.initData.virtualAddr, noffH.initData.size);
	//     executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]), noffH.initData.size, noffH.initData.inFileAddr);
	// }
	
}

int
AddrSpace::AllocatePages(){ //Function to allocate 8 pages on the stack for a new thread
	pageBitMapLock->Acquire();
	int pageStart = pageBitMap->Find();
	if(pageStart == -1){
		pageBitMapLock->Release();
		return -1;
	}
	pageStart = (executablePageCount + pageStart*8) * PageSize; 
	pageBitMapLock->Release();
	return pageStart;
}

void
AddrSpace::EmptyPages(){
	pageTableLock->Acquire();
	bitMapLock->Acquire();
	IPTLock->Acquire();
	for(int i = 0; i < numPages; i++){
		if(ipt[pageTable[i].physicalPage].as == this && ipt[pageTable[i].physicalPage].valid == TRUE){
			ipt[pageTable[i].physicalPage].valid = FALSE; //Mark as invalid

			memoryBitMap->Clear(pageTable[i].physicalPage); //Free up page in physical memory

			IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
			for(int j = 0; j < TLBSize; j++){
				if(machine->tlb[j].physicalPage == pageTable[i].physicalPage){
		        	ipt[pageTable[i].physicalPage].dirty = machine->tlb[j].dirty; //Pass dirty bit to IPT from TLB
					machine->tlb[j].valid = FALSE; //Invalidate TLB entry
					break;
				}
			}
		    (void) interrupt->SetLevel(oldLevel); // re-enable interrupts

			if(evictMethod == 1){
				fifoLock->Acquire();
				fifoQueue->remove(pageTable[i].physicalPage);
				fifoLock->Release();
			}
		}
	}
	IPTLock->Release();
	bitMapLock->Release();
	pageTableLock->Release();
}

void 
AddrSpace::Empty8Pages(int startPage){
	startPage = startPage/PageSize;
	pageTableLock->Acquire();
	bitMapLock->Acquire();
	IPTLock->Acquire();
	for(int i = 0; i < 8; i++){
		if(ipt[pageTable[startPage+i].physicalPage].as == this && ipt[pageTable[startPage+i].physicalPage].valid == TRUE){
			ipt[pageTable[startPage+i].physicalPage].valid = FALSE; //Mark as invalid

			memoryBitMap->Clear(pageTable[startPage+i].physicalPage); //Free up page in physical memory

			IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
			for(int j = 0; j < TLBSize; j++){
				if(machine->tlb[j].physicalPage == pageTable[startPage+i].physicalPage){
		        	ipt[pageTable[startPage+i].physicalPage].dirty = machine->tlb[j].dirty; //Pass dirty bit to IPT from TLB
					machine->tlb[j].valid = FALSE; //Invalidate TLB entry
					break;
				}
			}
		    (void) interrupt->SetLevel(oldLevel); // re-enable interrupts

			if(evictMethod == 1){
				fifoLock->Acquire();
				fifoQueue->remove(pageTable[startPage+i].physicalPage);
				fifoLock->Release();
			}
		}
	}
	IPTLock->Release();
	bitMapLock->Release();
	pageTableLock->Release();

}

#endif


//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
	delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
	int i;
	
	for (i = 0; i < NumTotalRegs; i++)
		machine->WriteRegister(i, 0);
	
	// Initial program counter -- must be location of "Start"
	machine->WriteRegister(PCReg, 0);
	
	// Need to also tell MIPS where next instruction is, because
	// of branch delay possibility
	machine->WriteRegister(NextPCReg, 4);
	
	// Set the stack register to the end of the address space, where we
	// allocated the stack; but subtract off a bit, to make sure we don't
	// accidentally reference off the end!
	#ifdef CHANGED
	int stackStart = AllocatePages();
	if(stackStart == -1){
		printf("Unable to allocate stack pages in InitRegisters()!");
		ASSERT(FALSE);
	}
	machine->WriteRegister(StackReg, stackStart);
	#endif
	DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
	//machine->pageTable = pageTable;
	machine->pageTableSize = numPages;
}
