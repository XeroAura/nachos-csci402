#include "setup.h"

void
Patient(){
	int i;
	int myPrescription;
	int myMedicineFee;
	int index;
	int shortest = recLineCount[0];
	int lineIndex = 0;
	int myDoorBoy = 0;
	int docIndex = 0;
	int myToken = 0;
	int myFee = 0;
	
	Acquire(patientIndexLock);
	index = nextPatientIndex;
	nextPatientIndex++;
	Release(patientIndexLock);
	
	MyWrite("Patient %d has arrived at the Hospital. \n", sizeof("Patient %d has arrived at the Hospital. \n")-1, index*100, 0);
	
	/*
	* Receptionist
	*/
	Acquire(recLineLock);
	for(i=0; i<recCount; i++){
		if(recLineCount[i] < shortest){
			lineIndex = i;
			shortest = recLineCount[i];
		}
		if(recState[i] == 0){
			recState[i] = 1;
			lineIndex = i;
			shortest = -1;
			break;
		}
	}
	if (testNum == 3 || testNum == 4 || testNum == 8){
		MyWrite("Patient %d is waiting on Receptionist %d. \n", sizeof("Patient %d is waiting on Receptionist %d. \n")-1, index*100+lineIndex,0);
	}
	if(shortest > -1 && (recState[lineIndex] == 1 || recState[lineIndex] == 2)){
		if (testNum == 3){
			MyWrite("The line Patient %d is entering is for receptionist %d and is currently %d people long. \n",sizeof("The line Patient %d is entering is for receptionist %d and is currently %d people long. \n")-1,
				index*100+lineIndex, recLineCount[lineIndex]*100);
		}
		recLineCount[lineIndex]++;
		Wait(recLineCV[lineIndex],recLineLock);
		recLineCount[lineIndex]--;
	}
	Release(recLineLock);
	Acquire(recLock[lineIndex]);
	Signal(recCV[lineIndex],recLock[lineIndex]);
	Wait(recCV[lineIndex],recLock[lineIndex]);
	myToken = recTokens[lineIndex];
	if ( testNum == 3  || testNum == 8){
		MyWrite("Patient %d has recieved Token %d from Receptionist %d \n", sizeof("Patient %d has recieved Token %d from Receptionist %d \n")-1, index*100+myToken,lineIndex*100);
	}
	Signal(recCV[lineIndex],recLock[lineIndex]);
	
	Release(recLock[lineIndex]);
	Acquire(doorBoyLineLock);
	
	
	/*
	* Doorboy
	*/
	doorBoyLineCount++;
	if (testNum == 1 || testNum == 2 || testNum == 8){
		MyWrite("Patient %d is waiting on a DoorBoy \n", sizeof("Patient %d is waiting on a DoorBoy \n")-1, index*100, 0);
	}
	Wait(doorBoyLineCV,doorBoyLineLock);
	if (testNum == 1 || testNum == 2 || testNum == 8){
		MyWrite("Patient %d was signaled by a DoorBoy\n", sizeof("Patient %d was signaled by a DoorBoy\n")-1,index*100, 0);
	}
	
	for(i = 0; i < doorBoyCount; i++){
		if(doorBoyState[i] == 3){
			doorBoyState[i] = 1;
			myDoorBoy = i;
			break;
		}
	}
	Acquire(doorBoyPatientLock);
	Release(doorBoyLineLock);
	
	Acquire(doorBoyTokenLock);
	doorBoyToken[myDoorBoy] = myToken;
	Release(doorBoyTokenLock);
	
	Signal(doorBoyPatientCV[myDoorBoy],doorBoyPatientLock);
	Wait(doorBoyPatientCV[myDoorBoy],doorBoyPatientLock);
	
	Acquire(doorBoyPatientRoomLock);
	docIndex = doorBoyPatientRoom[myDoorBoy];
	Release(doorBoyPatientRoomLock);
	if (testNum == 1 || testNum == 2 || testNum == 8){
		MyWrite("Patient %d has been told by DoorBoy %d to go to Examining Room %d \n", sizeof("Patient %d has been told by DoorBoy %d to go to Examining Room %d \n"), myToken*100+myDoorBoy, docIndex*100);
	}
	
	Signal(doorBoyPatientCV[myDoorBoy],doorBoyPatientLock);
	
	Acquire(docReadyLock);
	Release(doorBoyPatientLock);
	
	/*
	* Doctor
	*/
	if (testNum == 1 || testNum == 2 || testNum == 8){
		MyWrite("Patient %d is going to Examining Room %d \n", sizeof("Patient %d is going to Examining Room %d \n")-1, myToken*100+docIndex, 0);
	}
	docState[docIndex] = 1;
	
	Release(docReadyLock);
	
	Acquire(docLock[docIndex]);
	
	Acquire(docTokenLock);
	docToken[docIndex] = myToken;
	Release(docTokenLock);
	
	if (testNum == 1 || testNum == 2 || testNum == 5 || testNum == 8){
		MyWrite("Patient %d is waiting to be examined by the Doctor in ExaminingRoom %d \n", sizeof("Patient %d is waiting to be examined by the Doctor in ExaminingRoom %d \n") -1, myToken*100 + docIndex, 0);
	}
	
	Signal(docCV[docIndex], docLock[docIndex]);
	Wait(docCV[docIndex], docLock[docIndex]);
	
	Acquire(docPrescriptionLock);
	myPrescription = docPrescription[docIndex];
	if (testNum == 8){
		if(myPrescription == 0){
			MyWrite("Patient %d is not sick in Examining Room %d \n", sizeof("Patient %d is not sick in Examining Room %d \n")-1, myToken*100+docIndex, 0);
		}
		else{
			MyWrite("Patient %d is sick with disease %d in Examining Room %d \n", sizeof("Patient %d is sick with disease %d in Examining Room %d \n")-1, myToken*100+ myPrescription, docIndex*100);
			MyWrite("Patient %d has been prescribed medicine %d \n", sizeof("Patient %d has been prescribed medicine %d \n")-1, myToken*100+ myPrescription, 0);
		}
	}
	Release(docPrescriptionLock);
	Signal(docCV[docIndex], docLock[docIndex]);
	if (testNum == 8){
		MyWrite("Patient %d in Examining Room %d is waiting for the Doctor to come back from the Cashier \n", sizeof("Patient %d in Examining Room %d is waiting for the Doctor to come back from the Cashier \n")-1, myToken*100+ docIndex, 0);
		}
	Wait(docCV[docIndex], docLock[docIndex]);
	if (testNum == 5 ||testNum == 7 ||testNum == 8){
		MyWrite("Patient %d is leaving Examining Room %d\n", sizeof("Patient %d is leaving Examining Room %d\n")-1, myToken*100+ docIndex, 0);
	}
	Release(docLock[docIndex]);
	Acquire(cashierLineLock);
	
	/*
	* Cashier
	*/
	shortest = cashierLineCount[0];
	lineIndex = 0;
	for(i=0; i<cashierCount; i++){
		if(cashierLineCount[i] < shortest){
			lineIndex = i;
			shortest = cashierLineCount[i];
		}
		if(cashierState[i] == 0){
			cashierState[i] = 1;
			lineIndex = i;
			shortest = -1;
			break;
		}
	}
	if(shortest > -1 && (cashierState[lineIndex] == 1 || cashierState[lineIndex] == 2)){
		if (testNum == 3){
			
			MyWrite("The line Patient %d is entering belongs to Cashier %d and is currently %d people long. \n",
			sizeof("The line Patient %d is entering belongs to Cashier %d and is currently %d people long. \n")-1,
			index*100+lineIndex,cashierLineCount[lineIndex]*100);
		}
		cashierLineCount[lineIndex]++;
		Wait(cashierLineCV[lineIndex],cashierLineLock);
		cashierLineCount[lineIndex]--;
	}
	Acquire(cashierLock[lineIndex]);
	Release(cashierLineLock);
	if ( testNum == 3 || testNum == 8){
		MyWrite("Patient %d is waiting to see Cashier %d\n", sizeof("Patient %d is waiting to see Cashier %d\n")-1,myToken*100+lineIndex);
	}
	Acquire(cashierTokenLock);
	cashierToken[lineIndex] = myToken;
	Release(cashierTokenLock);
	Signal(cashierCV[lineIndex],cashierLock[lineIndex]);
	Wait(cashierCV[lineIndex],cashierLock[lineIndex]);
	
	Acquire(cashierFeeLock);
	myFee = cashierFee[lineIndex];
	Release(cashierFeeLock);
	if (testNum == 8){
		MyWrite("Patient %d is paying their consultancy fees of %d\n", sizeof("Patient %d is paying their consultancy fees of %d\n")-1, myToken*100+myFee);
	}
	Signal(cashierCV[lineIndex],cashierLock[lineIndex]);
	if (testNum == 8){
		MyWrite("Patient %d is leaving Cashier %d\n", sizeof("Patient %d is leaving Cashier %d\n")-1, myToken*100+lineIndex);
	}
	Release(cashierLock[lineIndex]);
	
	/*
	* Pharmacy Clerk
	*/
	if(myPrescription != 0){
		Acquire(clerkLineLock);
		shortest = clerkLineCount[0];
		lineIndex = 0;
		for(i=0; i<clerkCount; i++){
			if(clerkLineCount[i] < shortest){
				lineIndex = i;
				shortest = clerkLineCount[i];
			}
			if(clerkState[i] == 0){
				clerkState[i] = 1;
				lineIndex = i;
				shortest = -1;
				break;
			}
		}
		if(shortest > -1 && (clerkState[lineIndex] == 1|| clerkState[lineIndex] == 2)){
			if (testNum == 3){
				MyWrite("The line Patient %d is entering is for Pharmacy Clerk %d and is currently %d people long. \n", sizeof("The line Patient %d is entering is for Pharmacy Clerk %d and is currently %d people long. \n") -1,index*100+ lineIndex, clerkLineCount[lineIndex]*100);
				}
			clerkLineCount[lineIndex]++;
			Wait(clerkLineCV[lineIndex], clerkLineLock);
			clerkLineCount[lineIndex]--;
		}
		
		Release(clerkLineLock);
		Acquire(clerkLock[lineIndex]);
		
		Acquire(clerkTokenLock);
		clerkToken[lineIndex] = myToken;
		Release(clerkTokenLock);
		
		Acquire(clerkPrescriptionLock);
		clerkPrescription[lineIndex] = myPrescription;
		Release(clerkPrescriptionLock);
		
		Signal(clerkCV[lineIndex], clerkLock[lineIndex]);
		if(testNum == 3 || testNum == 8){
			MyWrite("Patient %d is waiting to see PharmacyClerk %d\n", sizeof("Patient %d is waiting to see PharmacyClerk %d\n")-1, myToken*100+ lineIndex, 0);
		}
		Wait(clerkCV[lineIndex], clerkLock[lineIndex]);
		
		Acquire(medicineFeeLock);
		myMedicineFee = medicineFee[lineIndex];
		if(testNum == 8){
			MyWrite("Patient %d is paying their prescription fees of %d\n",sizeof("Patient %d is paying their prescription fees of %d\n")-1, myToken*100+ myMedicineFee,0);
		}
		Release(medicineFeeLock);
		
		Signal(clerkCV[lineIndex], clerkLock[lineIndex]);
		if(testNum == 3 || testNum == 8){
			MyWrite("Patient %d is leaving PharmacyClerk %d\n", sizeof("Patient %d is leaving PharmacyClerk %d\not")-1, myToken*100 +lineIndex, 0);
		}
		Release(clerkLock[lineIndex]);
	}
	
	MyWrite("Patient %d is leaving the Hospital\n", sizeof("Patient %d is leaving the Hospital\n")-1, myToken*100, 0);
	completedPatientThreads++;
	Exit(0);
}