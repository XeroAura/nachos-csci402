/* Test hospital file
 * 
*/

#include "syscall.h"

/*Global variables*/
 int completedPatientThreads = 0;
 int numPatients = 0;
 int testNum = 0;

/*Receptionist globals*/
 int recLineLock = CreateLock(0);;
 int recLineCount[5] = {0,0,0,0,0};
 int recLineCV[5];
 int recState[5] = {1,1,1,1,1};

 int recTokens[5] = {0,0,0,0,0};
 int recLock[5];
 int recCV[5];
 int tokenLock = CreateLock(0); 
 int nextToken = 0; 
 int recCount = 5; 

 int recIndexLock = CreateLock(0);
 int nextRecIndex = 0;


/*Doctor globals*/
 int docLock[5];
 int docCV[5];
 int docState[5] = {1,1,1,1,1}; 
 int docToken[5] = {-1,-1,-1,-1,-1};
 int docTokenLock = CreateLock(0);
 int docPrescriptionLock = CreateLock(0); 
 int docPrescription[5] = {0,0,0,0,0}; 
 int docCount = 5; 
 int nextDoctorIndex = 0;
 int doctorIndexLock = CreateLock(0);

/*Doorboy globals*/
 int doorBoyLineLock = CreateLock(0);
 int doorBoyLineCV = CreateCondition(0);
 int doorBoyLineCount = 0; 

 int doorBoyDoctorCount = 0; 
 int doctorDoorBoyCount = 0; 
 int dbbLock = CreateLock(0);
 int doctorDoorBoyCV[5];
 int doorBoyDoctorCV[5];

 int docReadyLock = CreateLock(0);
 int docReadyCV[5]; 

 int doorBoyCount = 5; 
 int doorBoyState[5] = {1,1,1,1,1}; 

 int doorBoyPatientCV[5]; 
 int doorBoyPatientLock = CreateLock(0);
 int doorBoyToken[5] = {-1,-1,-1,-1,-1}; 

 int doorBoyTokenLock = CreateLock(0);
 int doorBoyPatientRoomLock = CreateLock(0);
 int doorBoyPatientRoom[5] = {-1,-1,-1,-1,-1};

 int doorBoyIndexLock = CreateLock(0);
 int nextDoorBoyIndex = 0;


/*Cashier globals */
 int consultLock = CreateLock(0); 
 std::map<int, int> consultationFee;
 int totalFeeLock = CreateLock(0); 
 int totalConsultationFee = 0; 
 int cashierCount = 5; 

 int cashierLineLock = CreateLock(0);
 int cashierLineCount[5] = {0,0,0,0,0};
 int cashierLineCV[5]; 

 int cashierLock[5]; 
 int cashierCV[5]; 

 int cashierState[5] = {1,1,1,1,1};
 int cashierToken[5] = {0,0,0,0,0};
 int cashierTokenLock = CreateLock(0);
 int cashierFee[5] = {0,0,0,0,0}; 
 int cashierFeeLock = CreateLock(0);

 int cashierIndexLock = CreateLock(0);
 int nextCashierIndex = 0;

/*Clerk globals*/
 int medicineFeeLock = CreateLock(0); 
 int medicineFee[5] = {0,0,0,0,0}; 
 int totalMedicineLock = CreateLock(0); 
 int totalMedicineCost = 0; 
 int clerkCount = 5; 

 int clerkLineLock = CreateLock(0); 
 int clerkLineCount[5] = {0,0,0,0,0}; 
 int clerkLineCV[5]; 
 int clerkState[5] = {1,1,1,1,1};
 int clerkPrescription[5] = {0,0,0,0,0}; 
 int clerkPrescriptionLock = CreateLock(0); 

 int clerkTokenLock = CreateLock(0); 
 int clerkToken[5] = {0,0,0,0,0}; 

 int clerkLock[5]; 
 int clerkCV[5];

 int clerkIndexLock = CreateLock(0);
 int nextClerkIndex = 0;

/*Manager globals*/
 int receptionistBreakLock = CreateLock(0);
 int receptionistBreakCV[5];
 int doorBoyBreakLock = CreateLock(0);
 int doorBoyBreakCV[5];
 int cashierBreakLock = CreateLock(0);
 int cashierBreakCV[5];
 int clerkBreakLock = CreateLock(0);
 int clerkBreakCV[5];

/* Hospital members*/
 void
 Patient(){
 	int i;
 	int myPrescription;
 	int myMedicineFee;
 	int index;
 	int shortest = recLineCount[0];
 	int lineIndex = 0;
 	int myDoorBoy = 0;
 	int docIndex;

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
	int myToken = recTokens[lineIndex]; 
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
	
	for(int i = 0; i < doorBoyCount; i++){
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
	int myFee = cashierFee[lineIndex]; 
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
}

void
Receptionist(int index){
	while(1){
		int index;
		Acquire(recIndexLock);
		index = nextRecIndex;
		nextRecIndex++;
		Release(recIndexLock);
		Acquire(recLineLock);

		recState[index]=0; 
		if(recLineCount[index] > 0) { 
			if(testNum == 8){
				MyWrite("Receptionist %d has signaled a Patient. \n", sizeof("Receptionist %d has signaled a Patient. \n")-1, index*100, 0);
			}
			Signal(recLineCV[index],recLineLock);
			recState[index]=1; 
		}
		Acquire(recLock[index]);
		Release(recLineLock);
		Wait(recCV[index],recLock[index]);
		Acquire(tokenLock);
		recTokens[index]=nextToken; 
		if(testNum == 8){
			MyWrite("Receptionist %d gives Token %d to a Patient. \n", sizeof("Receptionist %d gives Token %d to a Patient. \n")-1, index*100+nextToken);
		}
		nextToken++; 
		Release(tokenLock);
		Signal(recCV[index],recLock[index]);
		Wait(recCV[index],recLock[index]);
		Release(recLock[index]);
		

		Acquire(recLineLock);
		if(recLineCount[index] == 0){ 
			if(testNum == 8){
				MyWrite("Receptionist %d is going on break. \n", sizeof("Receptionist %d is going on break. \n")-1, index*100,0);
			}
			recState[index] = 2; 
			Acquire(receptionistBreakLock);
			Release(recLineLock);
			Wait(receptionistBreakCV[index],receptionistBreakLock);
			if(testNum == 8){
				MyWrite("Receptionist %d is coming off break. \n", sizeof("Receptionist %d is coming off break. \n")-1, index*100, 0);
			}
			Release(receptionistBreakLock);
		}
		else{
			Release(recLineLock);
		}
	}
}

void
Door_Boy(int index){
	int i;
	while(true){
		Acquire(doorBoyLineLock);
		doorBoyState[index] = 0; 
			if(doorBoyLineCount > 0){ 
				Release(doorBoyLineLock);
				Acquire(dbbLock);

				if(doctorDoorBoyCount > 0){ 
					doctorDoorBoyCount--; 

					Acquire(docReadyLock);
					
					int docNum = 0;
					for(i = 0; i < docCount; i++){
						if(docState[i] == 9){
							docState[i] = 0;
							docNum = i;
							break;
						}
					}
					Release(docReadyLock);
					Signal(doctorDoorBoyCV[docNum],dbbLock);
				}
				else{ 
					if(testNum == 1 || testNum == 8){
						MyWrite("DoorBoy %d is waiting for a Doctor\n", sizeof("DoorBoy %d is waiting for a Doctor\n")-1, index*100, 0);
					}
					doorBoyDoctorCount++;
					
					Acquire(doorBoyLineLock);
					doorBoyState[index] = 9;
					Release(doorBoyLineLock);
					Wait(doorBoyDoctorCV[index],dbbLock);

				}
				Release(dbbLock);
				
				Acquire(docReadyLock);
				int docIndex = 0;
			for{i = 0; i < docCount; i++){
				if(docState[i] == 0){ 
					if(testNum == 1 || testNum == 8){
						MyWrite("DoorBoy %d has been told by Doctor %d to bring a Patient.\n", sizeof("DoorBoy %d has been told by Doctor %d to bring a Patient.\n")-1, index*100+docIndex,0);
					}
					docState[i] = 3; 
					docIndex = i;
					break;
				}
			}
			
			Acquire(doorBoyLineLock);
			Release(docReadyLock);
			
			doorBoyState[index] = 3; 
			doorBoyLineCount--; 
			Signal(doorBoyLineCV,doorBoyLineLock);
			
			Acquire(doorBoyPatientLock);
			Release(doorBoyLineLock);
			if(testNum == 1 || testNum == 8){			
				MyWrite("DoorBoy %d has signaled a Patient.\n", sizeof("DoorBoy %d has signaled a Patient.\n")-1, index*100, 0);
			}
			Wait(doorBoyPatientCV[index],doorBoyPatientLock);
			
			Acquire(doorBoyTokenLock);
			int token = doorBoyToken[index]; 
			Release(doorBoyTokenLock);
			if(testNum == 1 || testNum == 8){
				MyWrite("DoorBoy %d has received Token %d from Patient %d\n", sizeof("DoorBoy %d has received Token %d from Patient %d\n")-1, index*100+token, token*100);
			}
			Acquire(doorBoyPatientRoomLock);
			doorBoyPatientRoom[index] = docIndex;
			Release(doorBoyPatientRoomLock);
			
			Signal(doorBoyPatientCV[index],doorBoyPatientLock);
			if(testNum == 1 || testNum == 8){
				MyWrite("DoorBoy %d has told Patient %d to go to Examining Room %d \n", sizeof("DoorBoy %d has told Patient %d to go to Examining Room %d \n")-1, index*100+token, docIndex*100);
			}
			Wait(doorBoyPatientCV[index],doorBoyPatientLock);
			Release(doorBoyPatientLock);
			
		}
		else{
			doorBoyState[index] = 2;
			if(testNum == 1 || testNum == 2 || testNum == 6 || testNum == 8){
				MyWrite("DoorBoy %d is going on break because there are no Patients. \n", sizeof("DoorBoy %d is going on break because there are no Patients. \n")-1, index*100, 0);
			}
			Acquire(doorBoyBreakLock);
			Release(doorBoyLineLock);
			Wait(doorBoyBreakCV[index],doorBoyBreakLock);
			if(testNum == 1 || testNum == 2 || testNum == 6 || testNum == 8){
				MyWrite("DoorBoy %d is coming off break. \n", sizeof("DoorBoy %d is coming off break. \n")-1, index*100, 0);
			}
			Release(doorBoyBreakLock);	

		}
	}
}

void
Doctor(){
	int stickCount = 0;
	int breakValCount = 0;
	int dbNum = 0;
	int i = 0;
	int token = 0;
	int sickTest =0;
	int yieldCount = 10;
	int breakVal = 0;
	int breakTimeVal = 10;
	int index;
	Acquire(doctorIndexLock);
	index = nextDoctorIndex;
	nextDoctorIndex++;
	Release(doctorIndexLock);

	while(true){
		Acquire(docReadyLock);
		docState[index] = 0;
		Acquire(docLock[index]);
		
		Release(docReadyLock);
		Acquire(dbbLock);

		if(doorBoyDoctorCount > 0){
			doorBoyDoctorCount--;
			Acquire(doorBoyLineLock);
			dbNum = 0;
			for( i = 0; i<doorBoyCount; i++){
				if(doorBoyState[i] == 9){
					doorBoyState[i] = 0;
					dbNum = i;
					break;
				}
			}
			Release(doorBoyLineLock);
			Signal(doorBoyDoctorCV[dbNum], dbbLock);	
		}
		else{
			doctorDoorBoyCount++;
			Acquire(docReadyLock);
			docState[index] = 9;
			Release(docReadyLock);
			Wait(doctorDoorBoyCV[index], dbbLock);
		}
		if(testNum == 1 || testNum == 8){
			MyWrite("Doctor %d has told a DoorBoy to bring a Patient to Examining Room %d \n", sizeof("Doctor %d has told a DoorBoy to bring a Patient to Examining Room %d \n")-1 index*100 + index, 0);
		}
		Release(dbbLock);
		Wait(docCV[index], docLock[index]);
		Acquire(docTokenLock);
		token = docToken[index];
		Release(docTokenLock);
		if(testNum == 1 || testNum == 8){
			MyWrite("Doctor %d is examining a Patient with Token %d \n", sizeof("Doctor %d is examining a Patient with Token %d \n")-1,index*100+token,0);
		}
		for(int i = 0; i < yieldCount; i++){ 
			Yield();
		}
		sickCount++;
		sickTest = sickCount%5; 
		/* 0 not sick
		1-4 sick */
		if (testNum == 7){
			sickTest = 1; 
		}
		if(testNum == 8){
			if(sickTest == 0){
				MyWrite("Doctor %d has determined that the Patient with Token %d is not sick\n", sizeof("Doctor %d has determined that the Patient with Token %d is not sick\n")-1, index*100+token, 0);
			}
			else{
				MyWrite("Doctor %d has determined that the Patient with Token %d is sick with disease type %d \n", sizeof("Doctor %d has determined that the Patient with Token %d is sick with disease type %d \n")-1, index*100+token, sickTest*100);
			}
		}
		Acquire(docPrescriptionLock);
		docPrescription[index] = sickTest; 
		if(testNum == 8){
			MyWrite("Doctor %d is prescribing medicine type %d to the Patient with Token %d \n", sizeof("Doctor %d is prescribing medicine type %d to the Patient with Token %d \n")-1, index*100+sickTest, token*100);
		}
		Release(docPrescriptionLock);
		
		Signal(docCV[index], docLock[index]); 
		Wait(docCV[index], docLock[index]); 
		
		Acquire(consultLock);
		consultationFee[token] = sickTest*20+20;
		Release(consultLock);
		if(testNum == 1 || testNum == 5 || testNum == 7 ||testNum == 8){		
			MyWrite("Doctor %d tells Patient with Token %d they can leave \n", sizeof("Doctor %d tells Patient with Token %d they can leave \n")-1, index*100+ token,0);
		}
		Signal(docCV[index], docLock[index]);
		Release(docLock[index]);
		
		breakValCount++;
		breakVal = breakValCount%2; 
		if(breakVal == 1 || (testNum == 5)){ 
			if (testNum == 5 || testNum == 8){
				MyWrite("Doctor %d tells a DoorBoy he is going on break \n", sizeof("Doctor %d tells a DoorBoy he is going on break \n")-1,index*100, 0);
			}
			Acquire(docReadyLock); 
			docState[index] = 2; 
			Release(docReadyLock);
			
			if (testNum == 5){
				MyWrite("Doctor %d is going on break for %d milliseconds. \n", sizeof("Doctor %d is going on break for %d milliseconds. \n") -1, index*100+ breakTimeVal, 0);
			}
			for(int i = 0; i < breakTimeVal; i++){
				Yield();
			}
			if (testNum == 5 || testNum == 8){
				MyWrite("Doctor %d tells a DoorBoy he is coming off break \n", sizeof("Doctor %d tells a DoorBoy he is coming off break \n")-1, index*100, 0);
			}
		}
		if (testNum == 5){
			MyWrite("To simulate test 5, Doctor %d is quitting. \n", sizeof("To simulate test 5, Doctor %d is quitting. \n")-1, index*100,0);
			break;
		}
	}
}

void
Cashier(int index){
	while(1){
		Acquire(cashierLineLock);
		cashierState[index]=0; 
		
		if(cashierLineCount[index] > 0) { 
			Signal(cashierLineCV[index],cashierLineLock);
			if (testNum == 3 || testNum == 8){
				MyWrite("Cashier %d has signaled a Patient \n", sizeof("Cashier %d has signaled a Patient \n")-1, index*100, 0);
			}
			cashierState[index]=1; 
		}

		Acquire(cashierLock[index]);		
		Release(cashierLineLock);

		Wait(cashierCV[index],cashierLock[index]);

		Acquire(cashierTokenLock);	
		int token = cashierToken[index]; 
		if (testNum == 3 || testNum == 8){
			MyWrite("Cashier %d gets Token %d from a Patient \n", sizeof("Cashier %d gets Token %d from a Patient \n")-1, index*100+token, 0);
		}
		Release(cashierTokenLock);
		cashierTokenLock->Release();
		
		Acquire(consultLock);
		int fee = consultationFee[token]; 
		Release(consultLock);
		
		Acquire(cashierFeeLock);
		cashierFee[index] = fee;  
		Release(cashierFeeLock);
		
		cashierCV[index]->Signal(cashierLock[index]); 
		if (testNum == 3 || testNum == 8){
			MyWrite("Cashier %d tells Patient with Token %d they owe %d \n", sizeof("Cashier %d tells Patient with Token %d they owe %d \n")-1, index*100+token, fee*100);
		}
		cashierCV[index]->Wait(cashierLock[index]); 
		if (testNum == 3 || testNum == 8){
			MyWrite("Cashier %d receives fees from Patient with Token %d \n", sizeof("Cashier %d receives fees from Patient with Token %d \n")-1, index*100+token, 0);
		}		
		totalFeeLock->Acquire();
		totalConsultationFee += fee; 
		totalFeeLock->Release();

		Release(cashierLock[index]);


		Acquire(cashierLineLock);

		if(cashierLineCount[index] == 0){ 
			if (testNum == 6 || testNum == 8){
				MyWrite("Cashier %d is going on break \n", sizeof("Cashier %d is going on break \n")-1, index*100, 0);
			}
			cashierState[index] = 2;
			Acquire(cashierBreakLock);
			Release(cashierLineLock);
			Wait(cashierBreakCV[index],cashierBreakLock);
			if (testNum == 6 || testNum == 8){
				MyWrite("Cashier %d is coming off break \n", sizeof("Cashier %d is coming off break \n")-1, index*100, 0);
			}
			Release(cashierBreakLock);
		}
		else{
			Release(cashierLineLock);
		}
	}
}

void
Clerk(){
	int token = 0;
	int fee = 0;
	int prescription = 0;	
	int index;
	Acquire(clerkIndexLock);
	index = nextClerkIndex;
	nextClerkIndex++;
	Release(clerkIndexLock);

	while(1){
		Acquire(clerkLineLock); 
		clerkState[index]=0; 
		
		if(clerkLineCount[index] > 0) { 
			if (testNum == 3 || testNum == 8){
				MyWrite("PharmacyClerk %d has signaled a Patient. \n", sizeof("PharmacyClerk %d has signaled a Patient. \n")-1,index*100, 0);
			}
			Signal(clerkLineCV[index], clerkLineLock); 
			clerkState[index] = 1; 
		}
		
		Acquire(clerkLock[index]);
		Release(clerkLineLock); 
		Wait(clerkCV[index], clerkLock[index]); 
		
		Acquire(clerkTokenLock);
		token = clerkToken[index];
		Release(clerkTokenLock);
		
		Acquire(clerkPrescriptionLock);
		prescription = clerkPrescription[index]; 
		if (testNum == 3 || testNum == 8){
			MyWrite("PharmacyClerk %d gets Prescription %d from Patient with Token %d \n", sizeof("PharmacyClerk %d gets Prescription %d from Patient with Token %d \n")-1, index*100+prescription, token*100);
		}
		Release(clerkPrescriptionLock);

		
		fee = prescription*25; 
		
		Acquire(medicineFeeLock);
		medicineFee[index] = fee; 
		Release(medicineFeeLock);
		if (testNum == 3 || testNum == 8){
			MyWrite("PharmacyClerk %d gives Prescription %d from Patient with Token %d \n", sizeof("PharmacyClerk %d gives Prescription %d from Patient with Token %d \n")-1, index*100+ prescription, token*100);
			MyWrite("PharmacyClerk %d tells Patient with Token %d they owe %d \n", sizeof("PharmacyClerk %d tells Patient with Token %d they owe %d \n")-1, index*100+ token, fee*100);
		}
		Signal(clerkCV[index], clerkLock[index]);
		Wait(clerkCV[index], clerkLock[index]); 
		if (testNum == 3 || testNum == 8){
			MyWrite("Pharmacyclerk %d gets money from Patient with Token %d \n", sizeof("Pharmacyclerk %d gets money from Patient with Token %d \n")-1, index*100+ token, 0);
		}
		Acquire(totalMedicineLock);
		totalMedicineCost += fee; 
		Release(totalMedicineLock);
		
		Release(clerkLock[index]);
		
		Acquire(clerkLineLock);
		if(clerkLineCount[index] == 0){
			clerkState[index] = 2; 
			Release(clerkLineLock);
			if (testNum == 6 || testNum == 8){
				MyWrite("PharmacyClerk %d is going on break. \n", sizeof("PharmacyClerk %d is going on break. \n")-1, index*100, 0);
			}
			Acquire(clerkBreakLock);
			Wait(clerkBreakCV[index], clerkBreakLock);
			Release(clerkBreakLock);
			if (testNum == 6 || testNum == 8){
				MyWrite("PharmacyClerk %d is coming off break. \n", sizeof("PharmacyClerk %d is coming off break. \n")-1, index*100, 0);
			}
		}
		else{
			Release(clerkLineLock);
		}
	}
}

void
Manager(){
	int i = 0;
	int yield = 70;
	int lineCnt = 0;
	int myConsultFee = 0;
	int myMedicineFee = 0;
	while(true){
		if (testNum == 7){
			yield = 5020;
		}
		for(i = 0; i<yield; i++){
			Yield();
		}
		
		Acquire(recLineLock);
		lineCnt = 0;
		for(i = 0; i < recCount; i++){
			lineCnt+= recLineCount[i];
		}
		if(lineCnt > 1){
			for(i = 0; i < recCount; i++){
				if(recState[i] == 2 ){
					Acquire(receptionistBreakLock);
					if (testNum == 6 || testNum == 8){
						MyWrite("HospitalManager signaled a Receptionist to come off break\n", sizeof("HospitalManager signaled a Receptionist to come off break\n")-1, 0, 0);
					}
					Signal(receptionistBreakCV[i], receptionistBreakLock);
					Release(receptionistBreakLock);
				}
			}
		}
		Release(recLineLock);

		
		if (testNum != 2){
			Acquire(doorBoyLineLock);
			if(doorBoyLineCount > 0){ 
				for(i = 0; i < doorBoyCount; i++){
					if(doorBoyState[i] == 2){
						Acquire(doorBoyBreakLock);
						if (testNum == 6 || testNum == 8){
							MyWrite("HospitalManager signaled a DoorBoy to come off break\n", sizeof("HospitalManager signaled a DoorBoy to come off break\n")-1,0 ,0);
						}
						Signal(doorBoyBreakCV[i], doorBoyBreakLock);
						Release(doorBoyBreakLock);
					}
				}
			}
			Release(doorBoyLineLock);
		}

		Acquire(cashierLineLock);
		for(i = 0; i < cashierCount; i++){
			if(cashierLineCount[i] > 0 && cashierState[i] == 2){
				Acquire(cashierBreakLock);
				if (testNum == 6 || testNum == 8){
					MyWrite("HospitalManager signaled a Cashier to come off break\n", sizeof("HospitalManager signaled a Cashier to come off break\n")-1, 0, 0);
				}
				Signal(cashierBreakCV[i], cashierBreakLock); 
				Release(cashierBreakLock);
			}
		}
		Release(cashierLineLock);
		
		Acquire(clerkLineLock);
		for(i = 0; i<clerkCount; i++){
			if(clerkLineCount[i] > 0 && clerkState[i] == 2){
				Acquire(clerkBreakLock);
				if (testNum == 6 || testNum == 8){
					MyWrite("HospitalManager signaled a PharmacyClerk to come off break\n", sizeof("HospitalManager signaled a PharmacyClerk to come off break\n")-1, 0, 0);
				}
				Signal(clerkBreakCV[i], clerkBreakLock);
				Release(clerkBreakLock);
			}
		}
		Release(clerkLineLock);


		/* 
		* COMMENT OUT BETWEEN TO REMOVE SPAM FROM HOSPITAL MANAGER 
		*/
		if (testNum == 7){
			Acquire(totalFeeLock);
			myConsultFee = totalConsultationFee;
			MyWrite("HospitalManager reports that total consultancy fees are %d\n", sizeof("HospitalManager reports that total consultancy fees are %d\n")-1, myConsultFee*100, 0);
			Release(totalFeeLock);

			Acquire(totalMedicineLock);
			myMedicineFee = totalMedicineCost;
			MyWrite("HospitalManager reports total sales in pharmacy are %d\n", sizeof("HospitalManager reports total sales in pharmacy are %d\n")-1, myMedicineFee*100, 0);
			Release(totalMedicineLock);
		}
		/* 
		* COMMENT OUT BETWEEN TO REMOVE SPAM FROM HOSPITAL MANAGER 
		*/
		if (testNum == 2 || testNum == 4 || testNum == 5 || testNum == 6){
			break;
		}

		if (completedPatientThreads == numPatients){
			break;
		}
		
	}
}

void
Setup(){
	char *name;
	int i = 0;
	for(i = 0; i < 5; i++){
		recLock[i] = CreateLock(0);
	}
	for (i = 0; i < 5; i++){
		recLineCV[i] = CreateCondition(0);
	}
	for (i = 0; i < 5; i++){
		recCV[i] = CreateCondition(0);
	}
	
	for(i = 0; i < 5; i++){
		docLock[i] = CreateLock(0);
	}
	for (i = 0; i < 5; i++){
		docCV[i] = CreateCondition(0);
	}
	
	for (i = 0; i < 5; i++){
		docReadyCV[i] = CreateCondition(0);
	}
	
	for (i = 0; i < 5; i++){
		doorBoyPatientCV[i] = CreateCondition(0);
	}

	for (i = 0; i < 5; i++){
		doctorDoorBoyCV[i] = CreateCondition(0);
	}

	for (i = 0; i < 5; i++){
		doorBoyDoctorCV[i] = CreateCondition(0);
	}
	
	for(i = 0; i < 5; i++){
		cashierLock[i] = CreateLock(0);
	}
	for (i = 0; i < 5; i++){
		cashierCV[i] = CreateCondition(0);
	}
	for (i = 0; i < 5; i++){
		cashierLineCV[i] = CreateCondition(0);
	}
	
	for(i = 0; i < 5; i++){
		clerkLock[i] = CreateLock(0);
	}
	for (i = 0; i < 5; i++){
		clerkCV[i] = CreateCondition(0);
	}
	
	for (i = 0; i < 5; i++){
		clerkLineCV[i] = CreateCondition(0);
	}
	
	for (i = 0; i < 5; i++){
		receptionistBreakCV[i] = CreateCondition(0);
	}
	for (i = 0; i < 5; i++){
		doorBoyBreakCV[i] = CreateCondition(0);
	}
	for (i = 0; i < 5; i++){
		clerkBreakCV[i] = CreateCondition(0);
	}
	for (i = 0; i < 5; i++){
		cashierBreakCV[i] = CreateCondition(0);
	}
}


void
InitializeThreads(){
	char* name;
	int i;

	MyWrite("Number of Receptionists = %d \n", sizeof()-1, recCount*100, 0);
	MyWrite("Number of Doctors = %d \n", sizeof("Number of Doctors = %d \n")-1, docCount*100, 0);
	MyWrite("Number of DoorBoys = %d \n", sizeof("Number of DoorBoys = %d \n")-1, doorBoyCount*100, 0);
	MyWrite("Number of Cashiers = %d \n", sizeof("Number of Cashiers = %d \n")-1, cashierCount*100, 0);
	MyWrite("Number of PharmacyClerks = %d \n", sizeof("Number of PharmacyClerks = %d \n")-1, clerkCount*100, 0);
	MyWrite("Number of Patients = %d \n", sizeof("Number of Patients = %d \n")-1, numPatients*100, 0);

	for (i = 0; i < docCount; i++){
		Fork(Doctor);
	}

	for (i = 0; i < recCount; i++){
		Fork(Receptionist);
	}

	for (i = 0; i < doorBoyCount; i++){
		Fork(Door_Boy);
	}

	for (i = 0; i < cashierCount; i++){
		Fork(Cashier);
	}

	for (i = 0; i < clerkCount; i++){
		Fork(Clerk);
	}

	Fork(Manager);

	for (i = 0; i < numPatients; i++){
		Fork(Patient);
	}

}

int main() {
	
}

