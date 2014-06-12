// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#include <stdlib.h>
#include <map>
#endif

#ifdef CHANGED
// --------------------------------------------------
// Test suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
// lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the
// lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
// lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
// done
Lock t1_l1("t1_l1");  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
	t1_l1.Acquire();
	t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
	
	printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
		t1_l1.getName());
	t1_s3.P();
	printf ("%s: working in CS\n",currentThread->getName());
	for (int i = 0; i < 1000000; i++) ;
		printf ("%s: Releasing Lock %s\n",currentThread->getName(),
			t1_l1.getName());
	t1_l1.Release();
	t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {
	
	t1_s1.P();// Wait until t1 has the lock
	t1_s2.V();  // Let t3 try to acquire the lock
	
	printf("%s: trying to acquire lock %s\n",currentThread->getName(),
		t1_l1.getName());
	t1_l1.Acquire();
	
	printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
		t1_l1.getName());
	for (int i = 0; i < 10; i++)
		;
	printf ("%s: Releasing Lock %s\n",currentThread->getName(),
		t1_l1.getName());
	t1_l1.Release();
	t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {
	
	t1_s2.P();// Wait until t2 is ready to try to acquire the lock
	
	t1_s3.V();// Let t1 do it's stuff
	for ( int i = 0; i < 3; i++ ) {
		printf("%s: Trying to release Lock %s\n",currentThread->getName(),
			t1_l1.getName());
		t1_l1.Release();
	}
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");// For mutual exclusion
Condition t2_c1("t2_c1");// The condition variable to test
Semaphore t2_s1("t2_s1",0);// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
// done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
	t2_l1.Acquire();
	printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
		t2_l1.getName(), t2_c1.getName());
	t2_c1.Signal(&t2_l1);
	printf("%s: Releasing Lock %s\n",currentThread->getName(),
		t2_l1.getName());
	t2_l1.Release();
	t2_s1.V();// release t2_t2
	t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
	t2_s1.P();// Wait for t2_t1 to be done with the lock
	t2_l1.Acquire();
	printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
		t2_l1.getName(), t2_c1.getName());
	t2_c1.Wait(&t2_l1);
	printf("%s: Releasing Lock %s\n",currentThread->getName(),
		t2_l1.getName());
	t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");// For mutual exclusion
Condition t3_c1("t3_c1");// The condition variable to test
Semaphore t3_s1("t3_s1",0);// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
// done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
	t3_l1.Acquire();
	t3_s1.V();// Let the signaller know we're ready to wait
	printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
		t3_l1.getName(), t3_c1.getName());
	t3_c1.Wait(&t3_l1);
	printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
	t3_l1.Release();
	t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {
	
	// Don't signal until someone's waiting
	
	for ( int i = 0; i < 5 ; i++ )
		t3_s1.P();
	t3_l1.Acquire();
	printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
		t3_l1.getName(), t3_c1.getName());
	t3_c1.Signal(&t3_l1);
	printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
	t3_l1.Release();
	t3_done.V();
}

// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");// For mutual exclusion
Condition t4_c1("t4_c1");// The condition variable to test
Semaphore t4_s1("t4_s1",0);// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
// done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
	t4_l1.Acquire();
	t4_s1.V();// Let the signaller know we're ready to wait
	printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
		t4_l1.getName(), t4_c1.getName());
	t4_c1.Wait(&t4_l1);
	printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
	t4_l1.Release();
	t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {
	
	// Don't broadcast until someone's waiting
	
	for ( int i = 0; i < 5 ; i++ )
		t4_s1.P();
	t4_l1.Acquire();
	printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
		t4_l1.getName(), t4_c1.getName());
	t4_c1.Broadcast(&t4_l1);
	printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
	t4_l1.Release();
	t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");// For mutual exclusion
Lock t5_l2("t5_l2");// Second lock for the bad behavior
Condition t5_c1("t5_c1");// The condition variable to test
Semaphore t5_s1("t5_s1",0);// To make sure t5_t2 acquires the lock after
// t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
	t5_l1.Acquire();
	t5_s1.V();// release t5_t2
	printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
		t5_l1.getName(), t5_c1.getName());
	t5_c1.Wait(&t5_l1);
	printf("%s: Releasing Lock %s\n",currentThread->getName(),
		t5_l1.getName());
	t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
	t5_s1.P();// Wait for t5_t1 to get into the monitor
	t5_l1.Acquire();
	t5_l2.Acquire();
	printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
		t5_l2.getName(), t5_c1.getName());
	t5_c1.Signal(&t5_l2);
	printf("%s: Releasing Lock %s\n",currentThread->getName(),
		t5_l2.getName());
	t5_l2.Release();
	printf("%s: Releasing Lock %s\n",currentThread->getName(),
		t5_l1.getName());
	t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
// 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
	Thread *t;
	char *name;
	int i;
	
	// Test 1
	
	printf("Starting Test 1\n");
	
	t = new Thread("t1_t1");
	t->Fork((VoidFunctionPtr)t1_t1,0);
	
	t = new Thread("t1_t2");
	t->Fork((VoidFunctionPtr)t1_t2,0);
	
	t = new Thread("t1_t3");
	t->Fork((VoidFunctionPtr)t1_t3,0);
	
	// Wait for Test 1 to complete
	for (  i = 0; i < 2; i++ )
		t1_done.P();
	
	// Test 2
	
	printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
	printf("completes\n");
	
	t = new Thread("t2_t1");
	t->Fork((VoidFunctionPtr)t2_t1,0);
	
	t = new Thread("t2_t2");
	t->Fork((VoidFunctionPtr)t2_t2,0);
	
	// Wait for Test 2 to complete
	t2_done.P();
	
	// Test 3
	
	printf("Starting Test 3\n");
	
	for (  i = 0 ; i < 5 ; i++ ) {
		name = new char [20];
		sprintf(name,"t3_waiter%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)t3_waiter,0);
	}
	t = new Thread("t3_signaller");
	t->Fork((VoidFunctionPtr)t3_signaller,0);
	
	// Wait for Test 3 to complete
	for (  i = 0; i < 2; i++ )
		t3_done.P();
	
	// Test 4
	
	printf("Starting Test 4\n");
	
	for (  i = 0 ; i < 5 ; i++ ) {
		name = new char [20];
		sprintf(name,"t4_waiter%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)t4_waiter,0);
	}
	t = new Thread("t4_signaller");
	t->Fork((VoidFunctionPtr)t4_signaller,0);
	
	// Wait for Test 4 to complete
	for (  i = 0; i < 6; i++ )
		t4_done.P();
	
	// Test 5
	
	printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
	printf("completes\n");
	
	t = new Thread("t5_t1");
	t->Fork((VoidFunctionPtr)t5_t1,0);
	
	t = new Thread("t5_t2");
	t->Fork((VoidFunctionPtr)t5_t2,0);
	
}
#endif


//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
	int num;
	
	for (num = 0; num < 5; num++) {
		printf("*** thread %d looped %d times\n", which, num);
		currentThread->Yield();
	}
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
	DEBUG('t', "Entering SimpleTest");
	
	Thread *t = new Thread("forked thread");
	
	t->Fork(SimpleThread, 1);
	SimpleThread(0);
	DEBUG('t', "Entering TestSuite");
	TestSuite();
}
//Simple test cases for the threads assignment.

#ifdef CHANGED

//Global variables
int completedPatientThreads = 0; //for debugging
int numPatients = 0; //Number of patients
int testNum = 0;

//Receptionist globals
Lock* recLineLock = new Lock("recLineLock");
int recLineCount[5] = {0,0,0,0,0}; //Number of patients in each receptionist line
Condition* recLineCV[5]; //CV for receptionist line patients
int recState[5] = {1,1,1,1,1}; //0 available, 1 busy, 2 on-break

int recTokens[5] = {0,0,0,0,0}; //Used to transfer token from patient to receptionist
Lock* recLock[5]; //Lock for interaction between receptionist and patient
Condition* recCV[5]; //Condition for interaction between receptionist and patient
Lock* tokenLock = new Lock("tokenLock"); //Lock for token assignment
int nextToken = 0; //Next token value
int recCount = 5; //Number of receptionists

//Doctor globals
Lock* docLock[5]; //Lock for doctor and patient meeting
Condition* docCV[5]; //CV for doctor and patient meeting
int docState[5] = {1,1,1,1,1}; //0 available, 1 busy, 2 on-break, 3 waiting
int docToken[5] = {-1,-1,-1,-1,-1}; //Used to give doctor token from patients
Lock* docTokenLock = new Lock("docTokenLock"); //Lock for docTokens use
Lock * docPrescriptionLock = new Lock("dockPrescriptionLock"); //Lock for passing prescriptions
int docPrescription[5] = {0,0,0,0,0}; //1-4 represent problems
int docCount = 5; //Number of doctors

//Doorboy globals
Lock* doorBoyLineLock = new Lock("doorBoyLineLock"); //Lock to manage line
Condition* doorBoyLineCV = new Condition("doorBoyLineCV"); //CV for doctor line
int doorBoyLineCount = 0; //Number of people in doorboy line

int doorBoyDoctorCount = 0; //Number of doorboys waiting on doctor
int doctorDoorBoyCount = 0; //Number of doctors waiting on doorboy
Lock* dbbLock = new Lock("dbbLock"); //Lock for doorboy
Condition* doctorDoorBoyCV[5]; //CV for doctors waiting on doorboy
Condition* doorBoyDoctorCV[5]; //CV for doorboy waiting on doctor

Lock* docReadyLock = new Lock("docReadyLock"); //Lock for doctor readiness
Condition* docReadyCV[5]; //Condition variable for doctor readiness call

int doorBoyCount = 5; //Number of doorboys
int doorBoyState[5] = {1,1,1,1,1}; //0 free, 1 busy, 2 onbreak, 3 waiting for patient, 9 waiting doctor

Condition* doorBoyPatientCV[5]; //Used for interaction between doorboy and patient
Lock* doorBoyPatientLock = new Lock("doorBoyPatientLock");
int doorBoyToken[5] = {-1,-1,-1,-1,-1}; //Used to pass token between patient and doorboy

Lock* doorBoyTokenLock = new Lock("doorBoyTokenLock"); // Lock for doorBoyToken
Lock* doorBoyPatientRoomLock = new Lock("doorBoyPatientRoomLock"); //Lock for doorBoyPatientRoom
int doorBoyPatientRoom[5] = {-1,-1,-1,-1,-1}; //Used to tell patient which examination room

//Cashier globals
Lock* consultLock = new Lock("consultLock"); //Lock for consultation fee map
std::map<int, int> consultationFee; //Map of consultation fees tied to token
Lock* totalFeeLock = new Lock("totalFeeLock"); //Lock for total consultation fee
int totalConsultationFee = 0; //Total consultation fee
int cashierCount = 5; //Number of cashiers

Lock* cashierLineLock = new Lock("cashierLineLock"); //Used for lines for cashier
int cashierLineCount[5] = {0,0,0,0,0}; //Used for counting people in lines for cashier
Condition* cashierLineCV[5]; //Cashier line CV

Lock* cashierLock[5]; //Lock for interaction with cashier
Condition* cashierCV[5]; //CV for interaction with cashier

int cashierState[5] = {1,1,1,1,1}; //0 available, 1 busy, 2 on-break
int cashierToken[5] = {0,0,0,0,0}; //Used to pass token to cashier
Lock* cashierTokenLock = new Lock("cashierTokenLock"); //Lock for cashierToken
int cashierFee[5] = {0,0,0,0,0}; //Fee charged by cashier to patient
Lock* cashierFeeLock = new Lock("cashierFeeLock"); //Lock for cashierFee

//Clerk globals
Lock* medicineFeeLock = new Lock("medicineFeeLock"); //Lock for medicine costs
int medicineFee[5] = {0,0,0,0,0}; //Used to pass costs to patient
Lock* totalMedicineLock = new Lock("totalMedicineLock"); //Used to lock total medicine cost
int totalMedicineCost = 0; //Used to keep track of total medicine costs
int clerkCount = 5; // Number of clerks

Lock* clerkLineLock = new Lock("clerkLineLock"); //Lock for clerk lines
int clerkLineCount[5] = {0,0,0,0,0}; //Number of people in each line
Condition* clerkLineCV[5]; //CV for clerk line calling
int clerkState[5] = {1,1,1,1,1}; //0 available, 1 busy, 2 on-break
int clerkPrescription[5] = {0,0,0,0,0}; //Medicine types 1-4
Lock* clerkPrescriptionLock = new Lock("clerkPrescriptionLock"); //Lock for prescription passing

Lock* clerkTokenLock = new Lock("clerkTokenLock"); //Lock for token passing
int clerkToken[5] = {0,0,0,0,0}; //Used to hold tokens passed to clerk by patient

Lock* clerkLock[5]; //Lock for interaction with clerks
Condition* clerkCV[5]; //CV for interaction with clerks

//Manager globals
//Locks and condition variables used to put and wake up people in breaks
Lock* receptionistBreakLock = new Lock("receptionistBreakLock");
Condition* receptionistBreakCV[5];
Lock* doorBoyBreakLock = new Lock("doorBoyBreakLock");
Condition* doorBoyBreakCV[5];
Lock* cashierBreakLock = new Lock("cashierBreakLock");
Condition* cashierBreakCV[5];
Lock* clerkBreakLock = new Lock("clerkBreakLock");
Condition* clerkBreakCV[5];

/* Hospital members*/
void
Patient(int index){
	printf("Patient %d has arrived at the Hospital. \n",index);
	
	/*
	* Receptionist
	*/
	recLineLock->Acquire();
	//Find shortest line or receptionist
	int shortest = recLineCount[0]; //Shortest line length
	int lineIndex = 0; //Index of line
	for(int i=0; i<recCount; i++){ //Go through each receptionist
		if(recLineCount[i] < shortest){ //If the next receptionist has a shorter line
			lineIndex = i; //Set index to this receptionist
			shortest = recLineCount[i]; //Set shortest line length to this one's
		}
		if(recState[i] == 0){ //If receptionist is open
			recState[i] = 1; //Set receptionist's state to busy
			lineIndex = i; //Change line index to this receptionist
			shortest = -1;
			break;
		}
	}
	if (testNum == 3 || testNum == 4 || testNum == 8){
		printf("Patient %d is waiting on Receptionist %d. \n", index, lineIndex);
	}
	if(shortest > -1 && (recState[lineIndex] == 1 || recState[lineIndex] == 2)){ //All Receptionists are busy, wait in line
		if (testNum == 3){
			printf("The line Patient %d is entering is currently %d people long. \n",index, clerkLineCount[lineIndex]);
		}
		recLineCount[lineIndex]++; //Increment shortest line length
		recLineCV[lineIndex]->Wait(recLineLock); //Wait till called
		recLineCount[lineIndex]--; //Decrement after being woken
	}
	recLineLock->Release(); //Release lock on line
	recLock[lineIndex]->Acquire(); //Acquire lock to receptionist
	recCV[lineIndex]->Signal(recLock[lineIndex]); //Notify receptionist ready
	recCV[lineIndex]->Wait(recLock[lineIndex]); //Wait for receptionist to reply
	int myToken = recTokens[lineIndex]; //Take token from receptionist
	if ( testNum == 3  || testNum == 8){	
		printf("Patient %d has recieved Token %d from Receptionist %d \n",index, myToken, lineIndex);
	}
	recCV[lineIndex]->Signal(recLock[lineIndex]); //Notify receptionist token taken
	
	recLock[lineIndex]->Release(); //Release lock to receptionist
	doorBoyLineLock->Acquire(); //Acquires lock for doctor line
	
	
	/*
	* Doorboy
	*/
	doorBoyLineCount++; //Increments line count by one
	if (testNum == 1 || testNum == 2 || testNum == 8){
		printf("Patient %d is waiting on a DoorBoy \n",index);
	}
	doorBoyLineCV->Wait(doorBoyLineLock); //Wait for doorboy to call
	if (testNum == 1 || testNum == 2 || testNum == 8){
		printf("Patient %d was signaled by a DoorBoy\n", index);
	}
	
	int myDoorBoy = 0;
	for(int i = 0; i < doorBoyCount; i++){
		if(doorBoyState[i] == 3){
			doorBoyState[i] = 1;
			myDoorBoy = i;
			break;
		}
	}
	
	doorBoyLineLock->Release();
	doorBoyPatientLock->Acquire();
	
	doorBoyTokenLock->Acquire();
	doorBoyToken[myDoorBoy] = myToken; //Give doorboy my token
	doorBoyTokenLock->Release();
	
	doorBoyPatientCV[myDoorBoy]->Signal(doorBoyPatientLock); //Tell doorboy to take token
	doorBoyPatientCV[myDoorBoy]->Wait(doorBoyPatientLock); //Wait for doorboy to respond
	
	doorBoyPatientRoomLock->Acquire();
	int docIndex = doorBoyPatientRoom[myDoorBoy];
	doorBoyPatientRoomLock->Release();
	if (testNum == 1 || testNum == 2 || testNum == 8){
		printf("Patient %d has been told by DoorBoy %d to go to Examining Room %d \n", myToken, myDoorBoy, docIndex);		
	}
	
	doorBoyPatientCV[myDoorBoy]->Signal(doorBoyPatientLock);
	
	doorBoyPatientLock->Release();
	docReadyLock->Acquire(); //Acquire lock for doctor
	
	/*
	* Doctor
	*/
	if (testNum == 1 || testNum == 2 || testNum == 8){
		printf("Patient %d is going to Examining Room %d \n", myToken, docIndex);
	}
	docState[docIndex] = 1; //Set doctor to busy
	
	docReadyLock->Release();
	
	docLock[docIndex]->Acquire(); //Acquire doctor lock

	docTokenLock->Acquire();
	docToken[docIndex] = myToken; //Give token to doctor
	docTokenLock->Release();

	if (testNum == 1 || testNum == 2 || testNum == 5 || testNum == 8){	
		printf("Patient %d is waiting to be examined by the Doctor in ExaminingRoom %d \n", myToken, docIndex);
	}
	if(myToken == 0 || myToken == 1){
	//	printf(">>>Patient %d signaling.\n",myToken);
	}
	docCV[docIndex]->Signal(docLock[docIndex]); //Tell doctor to take token
	if(myToken == 0 || myToken == 1){
	//	printf(">>>Patient %d done signaling.\n",myToken);
	}
	docCV[docIndex]->Wait(docLock[docIndex]); //Wait for doctor to do checkup
	
	docPrescriptionLock->Acquire();
	int myPrescription = docPrescription[docIndex]; //Takes prescription
	if (testNum == 8){
		if(myPrescription == 0){
			printf("Patient %d is not sick in Examining Room %d \n", myToken, docIndex);
		}
		else{
			printf("Patient %d is sick with disease %d in Examining Room %d \n", myToken, myPrescription, docIndex);
			printf("Patient %d has been prescribed medicine %d \n", myToken, myPrescription);
		}
	}
	docPrescriptionLock->Release();
	docCV[docIndex]->Signal(docLock[docIndex]); //Notifies doctor that patient took prescrip.
	if (testNum == 8){
		printf("Patient %d in Examining Room %d is waiting for the Doctor to come back from the Cashier \n", myToken, docIndex);
	}
	docCV[docIndex]->Wait(docLock[docIndex]); //Wait for doctor to return from cashier
	if (testNum == 5 ||testNum == 7 ||testNum == 8){
		printf("Patient %d is leaving Examining Room %d\n", myToken, docIndex);
	}	
	docLock[docIndex]->Release();
	cashierLineLock->Acquire(); //Find shortest cashier line
	
	/*
	* Cashier
	*/
	shortest = cashierLineCount[0]; //Shortest line length
	lineIndex = 0; //Index of line
	for(int i=0; i<cashierCount; i++){ //Go through each cashier
		if(cashierLineCount[i] < shortest){ //If the next cashier has a shorter line
			lineIndex = i; //Set index to this cashier
			shortest = cashierLineCount[i]; //Set shortest line length to this one's
		}
		if(cashierState[i] == 0){ //If cashier is open
			cashierState[i] = 1; //Set cashier's state to busy
			lineIndex = i; //Change line index to this cashier
			shortest = -1;
			break;
		}
	}
	if(shortest > -1 && (cashierState[lineIndex] == 1 || cashierState[lineIndex] == 2)){ //All cashier are busy, wait in line
		if (testNum == 3){
			printf("The line Patient %d is entering is currently %d people long. \n",index, clerkLineCount[lineIndex]);
		}
		cashierLineCount[lineIndex]++; //Increment shortest line length
		cashierLineCV[lineIndex]->Wait(cashierLineLock); //Wait till called
		cashierLineCount[lineIndex]--; //Decrement after being woken
	}
	cashierLineLock->Release(); //Release lock on line
	cashierLock[lineIndex]->Acquire(); //Acquire lock to cashier
	if ( testNum == 3 || testNum == 8){
		printf("Patient %d is waiting to see Cashier %d\n", myToken, lineIndex);
	}
	cashierTokenLock->Acquire();
	cashierToken[lineIndex] = myToken; //Give token to cashier
	cashierTokenLock->Release();
	cashierCV[lineIndex]->Signal(cashierLock[lineIndex]); //Notify cashier ready
	cashierCV[lineIndex]->Wait(cashierLock[lineIndex]); //Wait for cashier to reply with fee
	
	cashierFeeLock->Acquire();
	int myFee = cashierFee[lineIndex]; //Get fee from cashier
	cashierFeeLock->Release();
	if (testNum == 8){
		printf("Patient %d is paying their consultancy fees of %d\n", myToken, myFee);
	}
	cashierCV[lineIndex]->Signal(cashierLock[lineIndex]); //Give cashier cash
	if (testNum == 8){	
		printf("Patient %d is leaving Cashier %d\n", myToken, lineIndex);
	}
	cashierLock[lineIndex]->Release();
	/*
	* Pharmacy Clerk
	*/
	if(myPrescription != 0){ //If prescription assigned
		clerkLineLock->Acquire(); //Find shortest clerk line
		shortest = clerkLineCount[0]; //Shortest line length
		lineIndex = 0; //Index of line
		for(int i=0; i<clerkCount; i++){ //Go through each clerk
			if(clerkLineCount[i] < shortest){ //If the next clerk has a shorter line
				lineIndex = i; //Set index to this clerk
				shortest = clerkLineCount[i]; //Set shortest line length to this one's
			}
			if(clerkState[i] == 0){ //If clerk is open
				clerkState[i] = 1; //Set clerk's state to busy
				lineIndex = i; //Change line index to this clerk
				shortest = -1;
				break;
			}
		}
		if(shortest > -1 && (clerkState[lineIndex] == 1|| clerkState[lineIndex] == 2)){ //All clerk are busy, wait in line
			if (testNum == 3){
				printf("The line Patient %d is entering is currently %d people long. \n",index, clerkLineCount[lineIndex]);
			}
			clerkLineCount[lineIndex]++; //Increment shortest line length
			clerkLineCV[lineIndex]->Wait(clerkLineLock); //Wait till called
			clerkLineCount[lineIndex]--; //Decrement after being woken
		}
		
		clerkLineLock->Release(); //Release lock on clerk line
		clerkLock[lineIndex]->Acquire(); //Acquire lock to clerk
		
		clerkTokenLock->Acquire();
		clerkToken[lineIndex] = myToken; //Give token to cashier
		clerkTokenLock->Release();
		
		clerkPrescriptionLock->Acquire();
		clerkPrescription[lineIndex] = myPrescription; //Give clerk prescription
		clerkPrescriptionLock->Release();
		
		clerkCV[lineIndex]->Signal(clerkLock[lineIndex]); //Wake clerk up
		if(testNum == 3 || testNum == 8){
			printf("Patient %d is waiting to see PharmacyClerk %d\n", myToken, lineIndex);
		}
		clerkCV[lineIndex]->Wait(clerkLock[lineIndex]);
		
		medicineFeeLock->Acquire();
		int myMedicineFee = medicineFee[lineIndex]; //Receive fee and meds from clerk
		if(testNum == 8){
			printf("Patient %d is paying their prescription fees of %d\n", myToken, myMedicineFee);
		}
		medicineFeeLock->Release();
		
		clerkCV[lineIndex]->Signal(clerkLock[lineIndex]); //Pay clerk
		if(testNum == 3 || testNum == 8){
			printf("Patient %d is leaving PharmacyClerk %d\n", myToken, lineIndex);
		}
		clerkLock[lineIndex]->Release();
	}
	
	//Leave hospital
	printf("Patient %d is leaving the Hospital\n", myToken);
	completedPatientThreads++;
	// printf("Total Patients: %d \n",completedPatientThreads);
}

void
Receptionist(int index){
	while(1){
		recLineLock->Acquire(); //Acquire line lock
		recState[index]=0; //Set self to not busy
		if(recLineCount[index] > 0) { //Check to see if anyone in line
			if(testNum == 8){
				printf("Receptionist %d has signaled a Patient. \n",index);
			}
			recLineCV[index]->Signal(recLineLock); //Signal first person in line
			recState[index]=1; //Set self to busy
		}
		recLock[index]->Acquire(); //Acquire receptionist lock
		recLineLock->Release(); //Release line lock
		recCV[index]->Wait(recLock[index]); //Wait for patient to arrive
		tokenLock->Acquire(); //Acquire token lock
		recTokens[index]=nextToken; //Provide token to patient
		if(testNum == 8){
			printf("Receptionist %d gives Token %d to a Patient. \n",index,nextToken);
		}
		nextToken++; //Increment token count
		tokenLock->Release(); //Release token lock
		recCV[index]->Signal(recLock[index]); //Signal patient that token ready
		recCV[index]->Wait(recLock[index]); //Wait for patient to take token
		recLock[index]->Release(); //Release lock on receptionist
		
		//Take break check
		recLineLock->Acquire();
		if(recLineCount[index] == 0){ //If noone in line
			if(testNum == 8){
				printf("Receptionist %d is going on break. \n",index);
			}
			recState[index] = 2; //Set to on-break
			receptionistBreakLock->Acquire();
			recLineLock->Release();
			receptionistBreakCV[index]->Wait(receptionistBreakLock); //Set condition for manager to callback
			if(testNum == 8){
				printf("Receptionist %d is coming off break. \n",index);
			}
			receptionistBreakLock->Release();
		}
		else{
			recLineLock->Release();
		}
	}
}

void
Door_Boy(int index){
	while(true){
		// printf("---DoorBoy %d trying for linelock\n", index);
		doorBoyLineLock->Acquire();
		// printf("---DoorBoy %d got linelock\n", index);
		doorBoyState[index] = 0; //Set self to free
			if(doorBoyLineCount > 0){ //Someone waiting in line
				doorBoyLineLock->Release();

				// printf("---DoorBoy %d trying for dbblock\n", index);
				dbbLock->Acquire();
				// printf("---DoorBoy %d got dbblock\n", index);

				if(doctorDoorBoyCount > 0){ //If doctor waiting
					doctorDoorBoyCount--; //Decrease count of doctors waiting on doorboys

					// printf("---DoorBoy %d trying for docreadylock\n", index);
					docReadyLock->Acquire();
					// printf("---DoorBoy %d got docreadylock\n", index);
					
					int docNum = 0;
					for(int i = 0; i < docCount; i++){
						if(docState[i] == 9){
							docState[i] = 0;
							docNum = i;
							break;
						}
					}
					docReadyLock->Release();
					doctorDoorBoyCV[docNum]->Signal(dbbLock); //Wake doctor to treat
				}
				else{ //Wait for doctor
					if(testNum == 1 || testNum == 8){
						printf("DoorBoy %d is waiting for a Doctor\n", index);
					}
					doorBoyDoctorCount++;
					
					doorBoyLineLock->Acquire();
					doorBoyState[index] = 9;
					doorBoyLineLock->Release();
					// printf("---Doorboy %d wait on dbblock\n", index);
					doorBoyDoctorCV[index]->Wait(dbbLock); //Wait for doctor to call upon doorboy

				}
				dbbLock->Release();
				// printf("---DoorBoy %d released dbblock\n", index);
				
				docReadyLock->Acquire();
				int docIndex = 0;
			for(int i = 0; i < docCount; i++){ //Goes through each doctor
				if(docState[i] == 0){ //Finds first one ready
					if(testNum == 1 || testNum == 8){
						printf("DoorBoy %d has been told by Doctor %d to bring a Patient.\n", index, docIndex);
					}
					docState[i] = 3; //Claims doctor as own
					docIndex = i;
					break;
				}
			}
			
			doorBoyLineLock->Acquire();
			docReadyLock->Release();
			
			doorBoyState[index] = 3; //Set self to waiting
			doorBoyLineCount--; //Decrement line count
			doorBoyLineCV->Signal(doorBoyLineLock); //Signal patient to start
			
			doorBoyPatientLock->Acquire();
			doorBoyLineLock->Release();
			if(testNum == 1 || testNum == 8){			
				printf("DoorBoy %d has signaled a Patient.\n",index);
			}
			doorBoyPatientCV[index]->Wait(doorBoyPatientLock); //Wait for patient to arrive
			
			doorBoyTokenLock->Acquire();
			int token = doorBoyToken[index]; //Get token from patient
			doorBoyTokenLock->Release();
			if(testNum == 1 || testNum == 8){
				printf("DoorBoy %d has received Token %d from Patient %d\n", index, token, token);
			}
			doorBoyPatientRoomLock->Acquire();
			doorBoyPatientRoom[index] = docIndex; //Tell patient which room
			doorBoyPatientRoomLock->Release();
			
			doorBoyPatientCV[index]->Signal(doorBoyPatientLock); //Wake up patient to take room
			if(testNum == 1 || testNum == 8){
				printf("DoorBoy %d has told Patient %d to go to Examining Room %d \n", index, token, docIndex);
			}
			doorBoyPatientCV[index]->Wait(doorBoyPatientLock); //Wait for patient to get room
			doorBoyPatientLock->Release();
			
		}
		else{ //Noone in line
			doorBoyState[index] = 2;
			if(testNum == 1 || testNum == 2 || testNum == 6 || testNum == 8){
				printf("DoorBoy %d is going on break because there are no Patients. \n",index);
			}
			doorBoyLineLock->Release();
			doorBoyBreakLock->Acquire();
			doorBoyBreakCV[index]->Wait(doorBoyBreakLock); //Set condition for manager to callback
			if(testNum == 1 || testNum == 2 || testNum == 6 || testNum == 8){
				printf("DoorBoy %d is coming off break. \n",index);
			}
			
			doorBoyBreakLock->Release();
		}
	}
}

void
Doctor(int index){
	while(true){
		docReadyLock->Acquire();
		docState[index] = 0;
		docLock[index]->Acquire();
		
		docReadyLock->Release();
		dbbLock->Acquire();

		// printf("~~~~~Doctor %d acquired dbblock\n", index);

		if(doorBoyDoctorCount > 0){
			doorBoyDoctorCount--;
			// printf("~~~~~Doctor %d trying to acquire doorBoyLineLock\n", index);
			doorBoyLineLock->Acquire();
			// printf("~~~~~Doctor %d acquired doorBoyLineLock\n", index);
			int dbNum = 0;
			for(int i = 0; i<doorBoyCount; i++){
				if(doorBoyState[i] == 9){
					doorBoyState[i] = 0; //Find doorboy waiting and set to open
					dbNum = i;
					break;
				}
			}
			doorBoyLineLock->Release();
			doorBoyDoctorCV[dbNum]->Signal(dbbLock);	
		}
		else{
			// printf("~~~~~Doctor %d found no doorboys.\n", index);
			doctorDoorBoyCount++;
			// printf("~~~~~Doctor %d trying to acquire docReadyLock.\n", index);
			docReadyLock->Acquire();
			// printf("~~~~~Doctor %d acquired docReadyLock.\n", index);
			docState[index] = 9;
			docReadyLock->Release();
			// printf("~~~~~Doctor %d wait on dbblock\n", index);
			doctorDoorBoyCV[index]->Wait(dbbLock);
			//printf("~~~~~Doctor %d has awakened!\n", index);
		}
		if(testNum == 1 || testNum == 8){
			printf("Doctor %d has told a DoorBoy to bring a Patient to Examining Room %d \n",index, index);
		}
		dbbLock->Release();
		// printf("~~~~~Doctor %d trying to acquire doclock\n", index);
		//printf("~~~~~Doctor %d acquired doclock\n", index);
		
		// printf("~~~~~Doctor %d going to wait!\n", index);
		docCV[index]->Wait(docLock[index]);
		docTokenLock->Acquire();
		int token = docToken[index]; //Get patient's token number
		docTokenLock->Release();
		if(testNum == 1 || testNum == 8){
			printf("Doctor %d is examining a Patient with Token %d \n",index, token);
		}
		int yieldCount = rand()%11+10; //Generate yield times between 10 and 20
		for(int i = 0; i < yieldCount; i++){ //Check patient for that long
			currentThread->Yield(); //Yield thread to simulate time spent
		}
		int sickTest = rand()%5; //Generate if patient is sick
		/* 0 not sick
		1-4 sick */
		if (testNum == 7){
			sickTest = 1; //Making sickness static for test 8
		}
		if(testNum == 8){
			if(sickTest == 0){
				printf("Doctor %d has determined that the Patient with Token %d is not sick\n", index, token);
			}
			else{
				printf("Doctor %d has determined that the Patient with Token %d is sick with disease type %d \n", index, token, sickTest);
			}
		}
		docPrescriptionLock->Acquire();
		docPrescription[index] = sickTest; //Tells patient illness and prescription
		if(testNum == 8){
			printf("Doctor %d is prescribing medicine type %d to the Patient with Token %d \n", index, sickTest, token);
		}
		docPrescriptionLock->Release();
		
		docCV[index]->Signal(docLock[index]); //Tells patient to take prescription
		docCV[index]->Wait(docLock[index]); //Waits for patient to take prescription
		
		//Doctor tells cashiers price of consultation
		consultLock->Acquire();
		consultationFee[token] = sickTest*20+20;
		consultLock->Release();
		if(testNum == 1 || testNum == 5 || testNum == 7 ||testNum == 8){		
			printf("Doctor %d tells Patient with Token %d they can leave \n", index, token);
		}
		docCV[index]->Signal(docLock[index]);//Tell patient ok to go
		docLock[index]->Release();
		
		int breakVal = rand()%100; //Generate break value
		if(breakVal > 30 || (testNum == 5)){ //Take break for random time
			if (testNum == 5 || testNum == 8){
				printf("Doctor %d tells a DoorBoy he is going on break \n", index);
			}
			docReadyLock->Acquire(); //Acquire doctor ready lock
			docState[index] = 2; //Sets own state to ready
			docReadyLock->Release();
			int breakTimeVal = rand()%11+5; //Random between 5 and 15
			if (testNum == 5){
				printf("Doctor %d is going on break for %d milliseconds. \n",index, breakTimeVal);
			}
			for(int i = 0; i < breakTimeVal; i++){
				currentThread->Yield();
			}
			if (testNum == 5 || testNum == 8){
				printf("Doctor %d tells a DoorBoy he is coming off break \n", index);
			}
		}
		if (testNum == 5){
			printf("To simulate test 5, Doctor %d is quitting. \n",index);
			break;
		}
	}
}

void
Cashier(int index){
	while(1){
		cashierLineLock->Acquire(); //Acquire line lock
		cashierState[index]=0; //Set self to not busy
		
		if(cashierLineCount[index] > 0) { //Check to see if anyone in line
			cashierLineCV[index]->Signal(cashierLineLock); //Signal first person in line
			if (testNum == 3 || testNum == 8){
				printf("Cashier %d has signaled a Patient \n", index);
			}
			cashierState[index]=1; //Set self to busy
		}
		
		cashierLock[index]->Acquire(); //Acquire cashier lock
		cashierLineLock->Release(); //Release line lock

		cashierCV[index]->Wait(cashierLock[index]); //Wait for patient to arrive
		
		cashierTokenLock->Acquire();
		int token = cashierToken[index]; //Get token from patient
		if (testNum == 3 || testNum == 8){

			printf("Cashier %d gets Token %d from a Patient \n", index, token);
		}
		cashierTokenLock->Release();
		
		consultLock->Acquire();
		int fee = consultationFee[token]; //Look up consultation fee
		consultLock->Release();
		
		cashierFeeLock->Acquire();
		cashierFee[index] = fee;  //Set fee for patient to look at
		cashierFeeLock->Release();
		
		cashierCV[index]->Signal(cashierLock[index]); //Tell patient fee
		if (testNum == 3 || testNum == 8){
			printf("Cashier %d tells Patient with Token %d they owe %d \n", index, token, fee);
		}
		cashierCV[index]->Wait(cashierLock[index]); //Wait for patient to give money
		if (testNum == 3 || testNum == 8){

			printf("Cashier %d receives fees from Patient with Token %d \n", index, token);
		}		
		totalFeeLock->Acquire();
		totalConsultationFee += fee; //Add consultation fee to total count
		totalFeeLock->Release();

		cashierLock[index]->Release(); //Acquire cashier lock

		//Take break check
		cashierLineLock->Acquire();	
		if(cashierLineCount[index] == 0){ //If noone in line
			if (testNum == 6 || testNum == 8){
				printf("Cashier %d is going on break \n", index);
			}
			cashierState[index] = 2; //Set to on-break
			cashierBreakLock->Acquire();
			cashierLineLock->Release();
			cashierBreakCV[index]->Wait(cashierBreakLock); //Set condition for manager to callback
			if (testNum == 6 || testNum == 8){
				printf("Cashier %d is coming off break \n", index);
			}
			cashierBreakLock->Release();
		}
		else{
			cashierLineLock->Release();
		}
	}
}

void
Clerk(int index){
	while(1){
		clerkLineLock->Acquire(); //Acquire line lock
		clerkState[index]=0; //Set self to not busy
		
		if(clerkLineCount[index] > 0) { //Check to see if anyone in line
			if (testNum == 3 || testNum == 8){
				printf("PharmacyClerk %d has signaled a Patient. \n",index);
			}
			clerkLineCV[index]->Signal(clerkLineLock); //Signal first person in line
			clerkState[index] = 1; //Set self to busy
		}
		
		clerkLock[index]->Acquire(); //Acquire clerk lock
		clerkLineLock->Release(); //Release line lock
		clerkCV[index]->Wait(clerkLock[index]); //Wait for patient to arrive
		
		clerkTokenLock->Acquire();
		int token = clerkToken[index];
		clerkTokenLock->Release();
		
		clerkPrescriptionLock->Acquire();
		int prescription = clerkPrescription[index]; //Get prescription from patient
		if (testNum == 3 || testNum == 8){
			printf("PharmacyClerk %d gets Prescription %d from Patient with Token %d \n", index, prescription, token);
		}
		clerkPrescriptionLock->Release();

		
		int fee = prescription*25; //Calculate fee of medicine
		
		medicineFeeLock->Acquire();
		medicineFee[index] = fee; //Tell patient fee
		medicineFeeLock->Release();
		if (testNum == 3 || testNum == 8){
			printf("PharmacyClerk %d gives Prescription %d from Patient with Token %d \n", index, prescription, token);
			printf("PharmacyClerk %d tells Patient with Token %d they owe %d \n", index, token, fee);
		}
		clerkCV[index]->Signal(clerkLock[index]);
		clerkCV[index]->Wait(clerkLock[index]); //Wait for patient to give money and take prescription
		if (testNum == 3 || testNum == 8){
			printf("Pharmacyclerk %d gets money from Patient with Token %d \n", index, token);
		}
		totalMedicineLock->Acquire();
		totalMedicineCost += fee; //Add medicine fee to total count
		totalMedicineLock->Release();
		
		clerkLock[index]->Release(); //Release clerk lock
		
		//Take break check
		clerkLineLock->Acquire();
		if(clerkLineCount[index] == 0){ //If noone in line
			clerkState[index] = 2; //Set to on-break
			clerkLineLock->Release();
			if (testNum == 6 || testNum == 8){
				printf("PharmacyClerk %d is going on break. \n",index);
			}
			clerkBreakLock->Acquire();
			clerkBreakCV[index]->Wait(clerkBreakLock); //Set condition for manager to callback
			clerkBreakLock->Release();
			if (testNum == 6 || testNum == 8){
				printf("PharmacyClerk %d is coming off break. \n",index);
			}
		}
		else{
			clerkLineLock->Release();
		}
	}
}

void
Manager(){
	while(true){
		//Checks hospital is running randomly
		//Randomly generate number and yield for other threads
		int yield = rand()%50+50;
		if (testNum == 7){
			yield = rand()%50+5000;
		}
		for(int i = 0; i<yield; i++){
			currentThread->Yield();
		}
		
		//Wakes up receptionist
		//Check if there are at least 2 people waiting in receptionist line
		recLineLock->Acquire();
		int lineCnt = 0;
		for(int i = 0; i < recCount; i++){
			lineCnt+= recLineCount[i];
		}
		if(lineCnt > 1){
			for(int i = 0; i < recCount; i++){
				if(recState[i] == 2 ){
					receptionistBreakLock->Acquire();
					//Set receptionist to off break
					if (testNum == 6 || testNum == 8){
						printf("HospitalManager signaled a Receptionist to come off break\n");
					}
					receptionistBreakCV[i]->Signal(receptionistBreakLock);
					receptionistBreakLock->Release();
				}
			}
		}
		recLineLock->Release();

		
		//Wakes up door boy
		if (testNum != 2){
			doorBoyLineLock->Acquire();
		if(doorBoyLineCount > 0){ //Check if any patient in line
			for(int i = 0; i < doorBoyCount; i++){
				if(doorBoyState[i] == 2){
					doorBoyBreakLock->Acquire();
					//Set door boy to off break
					if (testNum == 6 || testNum == 8){
						printf("HospitalManager signaled a DoorBoy to come off break\n");
					}
					doorBoyBreakCV[i]->Signal(doorBoyBreakLock);
					doorBoyBreakLock->Release();
				}
			}
		}
		doorBoyLineLock->Release();
	}
		//Wakes up cashier
	cashierLineLock->Acquire();
	for( int i = 0; i < cashierCount; i++){
			if(cashierLineCount[i] > 0 && cashierState[i] == 2){ //Check if any patient in line
				cashierBreakLock->Acquire();
				if (testNum == 6 || testNum == 8){
					printf("HospitalManager signaled a Cashier to come off break\n");
				}
				cashierBreakCV[i]->Signal(cashierBreakLock); //Set cashier to off break
				cashierBreakLock->Release();
				//Get total consultation fee
			}
		}
		cashierLineLock->Release();
		
		

		//Wakes up clerk
		clerkLineLock->Acquire();
		for(int i = 0; i<clerkCount; i++){
			if(clerkLineCount[i] > 0 && clerkState[i] == 2){
				clerkBreakLock->Acquire();
				if (testNum == 6 || testNum == 8){
					printf("HospitalManager signaled a PharmacyClerk to come off break\n");
				}
				clerkBreakCV[i]->Signal(clerkBreakLock);
				clerkBreakLock->Release();
			}
		}
		clerkLineLock->Release();


		/* 
		* COMMENT OUT BETWEEN TO REMOVE SPAM FROM HOSPITAL MANAGER 
		*/
		if (testNum == 7){
			totalFeeLock->Acquire();
			int myConsultFee = totalConsultationFee;
			printf("HospitalManager reports that total consultancy fees are %d\n", myConsultFee);
			totalFeeLock->Release();

			totalMedicineLock->Acquire();
			int myMedicineFee = totalMedicineCost;
			printf("HospitalManager reports total sales in pharmacy are %d\n", myMedicineFee);
			totalMedicineLock->Release();
		}
		/* 
		* COMMENT OUT BETWEEM TO REMOVE SPAM FROM HOSPITAL MANAGER 
		*/
		if (testNum == 2 || testNum == 4 || testNum == 5 || testNum == 6){
			break;
		}

		if (completedPatientThreads == numPatients){
			// printf("No patients left, Manager leaving \n");
			break;
		}
		
	}
}

void
Setup(){
	char *name;
	//Instantiating receptionist variables
	for(int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"recLock%d",i);
		recLock[i] = new Lock(name);
	}
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"recLineCV%d",i);
		recLineCV[i] = new Condition(name);
	}
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"recCV%d",i);
		recCV[i] = new Condition(name);
	}
	
	//Instantiating doctor variables
	for(int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"docLock%d",i);
		docLock[i] = new Lock(name);
	}
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"docCV%d",i);
		docCV[i] = new Condition(name);
	}
	
	//Doorboy
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"docReadyCV%d",i);
		docReadyCV[i] = new Condition(name);
	}
	
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"doorBoyPatientCV%d",i);
		doorBoyPatientCV[i] = new Condition(name);
	}

	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"recLineCV%d",i);
		doctorDoorBoyCV[i] = new Condition(name);
	}

	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"recLineCV%d",i);
		doorBoyDoctorCV[i] = new Condition(name);
	}
	
	//Cashier
	//Map?
	for(int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"cashierLock%d",i);
		cashierLock[i] = new Lock(name);
	}
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"cashierCV%d",i);
		cashierCV[i] = new Condition(name);
	}
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"cashierLineCV%d",i);
		cashierLineCV[i] = new Condition(name);
	}
	
	//Clerk
	for(int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"clerkLock%d",i);
		clerkLock[i] = new Lock(name);
	}
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"clerkCV%d",i);
		clerkCV[i] = new Condition(name);
	}
	
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"clerkLineCV%d",i);
		clerkLineCV[i] = new Condition(name);
	}
	
	//Manager
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"receptionistBreakCV%d",i);
		receptionistBreakCV[i] = new Condition(name);
	}
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"doorBoyBreakCV%d",i);
		doorBoyBreakCV[i] = new Condition(name);
	}
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"clerkBreakCV%d",i);
		clerkBreakCV[i] = new Condition(name);
	}
	for (int i = 0; i < 5; i++){
		name = new char [20];
		sprintf(name,"cashierBreakCV%d",i);
		cashierBreakCV[i] = new Condition(name);
	}
}


//Initializing the threads for part 2 of the first assignment
void
InitializeThreads(){
	Thread *t;
	char* name;
	printf("\n");
	printf("Number of Receptionists = %d \n",recCount);
	printf("Number of Doctors = %d \n",docCount);
	printf("Number of DoorBoys = %d \n",doorBoyCount);
	printf("Number of Cashiers = %d \n",cashierCount);
	printf("Number of PharmacyClerks = %d \n",clerkCount);
	printf("Number of Patients = %d \n",numPatients);
	printf("\n");

	for (int i = 0; i < docCount; i++){
		name = new char [20];
		sprintf(name,"Doctor %d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr) Doctor,i);
	}

	for (int i = 0; i < recCount; i++){
		name = new char [20];
		sprintf(name,"Receptionist %d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr) Receptionist,i);
	}

	for (int i = 0; i < doorBoyCount; i++){
		name = new char [20];
		sprintf(name,"Door Boy %d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr) Door_Boy,i);
	}

	for (int i = 0; i < cashierCount; i++){
		name = new char [20];
		sprintf(name,"Cashier %d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr) Cashier,i);
	}

	for (int i = 0; i < clerkCount; i++){
		name = new char [20];
		sprintf(name,"Pharmacy Clerk %d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr) Clerk,i);
	}

	t = new Thread("Manager");
	t->Fork((VoidFunctionPtr) Manager, 1);

	for (int i = 0; i < numPatients; i++){
		name = new char [20];
		sprintf(name,"Patient %d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr) Patient,i);
	}

}

//Tests and test threads for part 2 of the first assignment
void
Problem2() {
	Setup();
	
	int menu = 0;
	printf("Problem 2 Start \n");
	printf("Please choose an option to test: \n");
	printf("1: Patient-DoorBoy-Doctor Interaction Test\n");
	printf("2: DoorBoy Break Test \n");
	printf("3: Patient Line-Choosing Test\n");
	printf("4: Patient Line-Waiting Test\n");
	printf("5: Doctor Break Test\n");
	printf("6: Job Break and Hospital Manager Test\n");
	printf("7: Hospital Fee\n");	
	printf("8: Custom Simulation \n");
	scanf("%d",&menu);

	recCount = 2;
	docCount = 2;
	doorBoyCount = 2;
	cashierCount = 2;
	clerkCount = 2;
	numPatients = 5;

	switch(menu){
		case 1:
		{	
			testNum = 1;
			docCount = 1;
			doorBoyCount = 1;
			numPatients = 2;
			printf("-----------------------------------------------------------------------\n");
			printf("Test 1\n");
			printf("Patients only gets in to see a Doctor when the DoorBoy asks them to. \n");
			printf("Only one Patient get to see one Doctor at any given instance of time, in a room\n \n");
			InitializeThreads();
			break;
		}
		case 2:
		{
			testNum = 2;
			docCount = 1;
			doorBoyCount = 3;
			numPatients = 5;
			printf("-----------------------------------------------------------------------\n");
			printf("Test 2\n");
			printf("If all the Doorboys are on a break no patient gets in to see Doctors. \n \n");
			InitializeThreads();
			break;
		}
		case 3:
		{
			testNum = 3;
			recCount = 1;
			docCount = 2;
			doorBoyCount = 2;	
			cashierCount = 1;
			clerkCount = 1;
			numPatients = 5;
			printf("-----------------------------------------------------------------------\n");
			printf("Test 3\n");
			printf("Patients always choose the shortest line with the Cashier, PharmacyClerk and Receptionist. \n \n");		
			InitializeThreads();
			break;
		}
		case 4:
		{	
			recCount = 0;
			cashierCount = 0;
			clerkCount = 0;
			printf("-----------------------------------------------------------------------\n");
			printf("Test 4\n");
			printf("If there is no Cashier/PharmacyClerk/Receptionist the patient must wait in line. \n \n");		
			testNum = 4;
			InitializeThreads();
			break;
		}
		case 5:
		{
			printf("-----------------------------------------------------------------------\n");
			printf("Test 5\n");
			printf("Doctors go on break at random intervals. \nWhen the Doctor is at break no patient gets in to that examining room. \nIf all Doctors are on break, all Patients wait.\n");
			testNum = 5;
			numPatients = 10;
			InitializeThreads();
			break;
		}
		case 6:
		{
			testNum = 6;
			printf("-----------------------------------------------------------------------\n");
			printf("Test 6\n");
			printf("DoorBoy/Cashier/PharmacyClerk gett signaled by hospital manager when patients are waiting.\n");			
			printf("DoorBoy/PharmacyClerk/Receptionist/Cashier go on break if their line is empty. \n");
			InitializeThreads();
			break;
		}
		case 7:
		{
			printf("-----------------------------------------------------------------------\n");
			printf("Test 7\n");
			printf("The total sales of medicines and the total consultation fees never get affected by race conditions. \n");
			testNum = 7;
			InitializeThreads();
			break;
		}
		case 8:
		{
			testNum = 8;
			printf("Enter how many receptionists to have in the office (between 2 and 5): ");
			scanf("%d",&recCount);
			printf("Enter how many patients to have in the office (between 5 and 20): ");
			scanf("%d",&numPatients);
			printf("Enter how many doctors to have in the office (between 2 and 5):");
			scanf("%d",&docCount);
			printf("Enter how many door boys to have in the office (between 2 and 5):");
			scanf("%d",&doorBoyCount);
			printf("Enter how many cashiers to have in the office (between 2 and 5):");
			scanf("%d",&cashierCount);
			printf("Enter how many clerks to have in the office (between 2 and 5):");
			scanf("%d",&clerkCount);
			InitializeThreads();
			break;
		}
		default:
		{
			printf("Invalid option entered. Exiting the program. \n");
			break;
		}
	}
}


#endif
