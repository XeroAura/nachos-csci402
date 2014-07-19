// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"
#include "scheduler.h"

#ifdef CHANGED
#include <iostream>
#endif

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!

#ifdef CHANGED
//----------------------------------------------------------------------
// Lock::Lock(char* debugName)
//       Initializes a lock, so that it can be used for synchronization
//
//  "debugName" is an arbitrary name useful for  debugging
//
//----------------------------------------------------------------------

Lock::Lock(char* debugName){
  name = debugName;
  isFree = true;
  waitList = new List;
}

//----------------------------------------------------------------------
// Lock::~Lock
//       De-allocates the lock when it is no longer needed. Assumes that
//       no one is waiting on the lock. 
//----------------------------------------------------------------------

Lock::~Lock() {
  delete waitList;
}

//----------------------------------------------------------------------
// Lock::Acquire
//       Attempts to acquire the lock. Acquiring must be done
//       atomically, so disable interrupts before acquiring. If the 
//       thread already owns the lock, nothing happens. If the lock is
//       in the FREE state, the lock  is acquired by the thread and set
//       to BUSY. If the lock is not available, the thread is added to the
//       wait queue and put to sleep. 
//----------------------------------------------------------------------

void Lock::Acquire() {
  IntStatus oldLevel = interrupt->SetLevel(IntOff); //disables interrupts
  if (lockOwner == currentThread){
    printf("Thread already owns this lock.\n");
    (void)interrupt->SetLevel(oldLevel);
    return;
  }
  if (isFree){
    isFree = false;
    lockOwner = currentThread;
  }else{
    waitList->Append((void*)currentThread);
    currentThread->Sleep();
  }
  (void)interrupt->SetLevel(oldLevel);
  return;
}

//----------------------------------------------------------------------
// Lock::Release
//       Attempts to release the lock. If the thread is no the lock
//       owner, nothing happens. If the wait queue is not empty, the 
//       thread at the front of the queue becomes the lock owner. If
//       the wait queue is empty, the lock is put in the FREE state and
//       the lock's owner is set to NULL.
//----------------------------------------------------------------------

void Lock::Release() {
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  if (currentThread != lockOwner){
    if (lockOwner != NULL){
      printf("Error: Only %s, the owner of %s, can release the lock. \n", lockOwner->getName(), getName());
    } else {
      printf("Error: The lock %s has not been acquired yet.\n", getName());
    }
    (void)interrupt->SetLevel(oldLevel);
    return;
  }
  if (!waitList->IsEmpty()){
    Thread* thread;
    thread = (Thread *)waitList->Remove();
    scheduler->ReadyToRun(thread);
    lockOwner = thread;
    //    std::cout << getName() << ": " << "Passing ownership of " << getName() << " to " << lockOwner->getName() << std::endl;
  } else {
    isFree = true;
    lockOwner = NULL;
    //    std::cout << getName() << ": " << currentThread->getName() <<  " is clearing ownership of " << getName() << std::endl;
  }
  (void)interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Condition::Condition(char* debugName)
//       Initializes a condition variable so it can be used.
//       "debugName" is an arbitrary name useful for debugging.
//----------------------------------------------------------------------

Condition::Condition(char* debugName) {
  name = debugName;
  waitingLock = NULL;
  waitList = new List;
}

//----------------------------------------------------------------------
// Condition::~Condition
//       De-allocates the condition variable when no longer needed. 
//       Assumes no one is waiting on the condition variable. 
//
//----------------------------------------------------------------------

Condition::~Condition() {
  delete waitList;
}

//----------------------------------------------------------------------
// Condition::Wait
//       Puts a thread to sleep waiting on a condition. Makes sure that
//       the conditionLock is not NULL. If there is no lock assigned to
//       the condition yet, the conditionLock is made the lock. If the
//       assigned lock is different from the conditionLock, nothing 
//       happens. The thread is then put in the condition's wait queue
//       and put to sleep.
//----------------------------------------------------------------------

void Condition::Wait(Lock* conditionLock){
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  if (conditionLock == NULL){
    std::cout << "Error: Need to have a lock." << std::endl;
    (void)interrupt->SetLevel(oldLevel);
    return;
  }
  if (waitingLock == NULL){
    waitingLock = conditionLock;
  }
  if(waitingLock != conditionLock){
    std::cout << "Error in Wait: Incorrect lock used." << std::endl;
    (void)interrupt->SetLevel(oldLevel);
    return;
  }
  waitList->Append((void*) currentThread);
  conditionLock->Release();
  currentThread->Sleep();
  conditionLock->Acquire();
  (void)interrupt->SetLevel(oldLevel);
  return;
}

//----------------------------------------------------------------------
// Condition::Signal
//       Wakes up 1 sleeping thread, if there is one. If there are no 
//       threads in the wait queue, nothing happens. If the conditionLock
//       is not the same as the waiting lock, nothing happens. The 
//       thread from the top of the queue is taken and put on the ready
//       queue. If that was the only thread in the queue, the conditionLock
//       is cleared. 
//----------------------------------------------------------------------

void Condition::Signal(Lock* conditionLock) {
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  if (waitList->IsEmpty()){
    printf("Nothing waiting on CV %s.\n",name);
    (void)interrupt->SetLevel(oldLevel);
    return;
  }
  if(waitingLock != conditionLock){
    std::cout << "Error in Signal: Incorrect lock used." << std::endl;
    (void)interrupt->SetLevel(oldLevel);
    return;
  }
  Thread* thread = (Thread *)waitList->Remove();
  scheduler->ReadyToRun(thread);
  if (waitList->IsEmpty()){
    waitingLock = NULL;
  }
  (void)interrupt->SetLevel(oldLevel);
  return;
}

//----------------------------------------------------------------------
// Condition::Broadcast
//       Sends the Signal command to every thread in the waiting queue
//       for the condition variable
//----------------------------------------------------------------------

void Condition::Broadcast(Lock* conditionLock) {
  while(!(waitList->IsEmpty())){
    Signal(conditionLock);
  }
}
#endif
