/* Test hospital file
 * 
*/

#include "syscall.h"

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
int docLock[5]; //Lock for doctor and patient meeting
int docCV[5]; //CV for doctor and patient meeting
int docState[5] = {1,1,1,1,1}; //0 available, 1 busy, 2 on-break, 3 waiting
int docToken[5] = {-1,-1,-1,-1,-1}; //Used to give doctor token from patients
int docTokenLock = CreateLock(0); //Lock for docTokens use
int docPrescriptionLock = CreateLock(0); //Lock for passing prescriptions
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
int medicineFeeLock = CreateLock(0); //Lock for medicine costs
int medicineFee[5] = {0,0,0,0,0}; //Used to pass costs to patient
int totalMedicineLock = CreateLock(0); //Used to lock total medicine cost
int totalMedicineCost = 0; //Used to keep track of total medicine costs
int clerkCount = 5; // Number of clerks

int clerkLineLock = CreateLock(0); //Lock for clerk lines
int clerkLineCount[5] = {0,0,0,0,0}; //Number of people in each line
int clerkLineCV[5]; //CV for clerk line calling
int clerkState[5] = {1,1,1,1,1}; //0 available, 1 busy, 2 on-break
int clerkPrescription[5] = {0,0,0,0,0}; //Medicine types 1-4
int clerkPrescriptionLock = CreateLock(0); //Lock for prescription passing

int clerkTokenLock = CreateLock(0); //Lock for token passing
int clerkToken[5] = {0,0,0,0,0}; //Used to hold tokens passed to clerk by patient

int clerkLock[5]; //Lock for interaction with clerks
int clerkCV[5]; //CV for interaction with clerks

//Manager globals
//Locks and condition variables used to put and wake up people in breaks
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
			printf("The line Patient %d is entering is for receptionist %d and is currently %d people long. \n",index, lineIndex, recLineCount[lineIndex]);
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
		MyWrite("Patient %d is going to Examining Room %d \n", myToken*100+docIndex, 0);
	}
	docState[docIndex] = 1; //Set doctor to busy
	
	Release(docReadyLock);
	
	Acquire(docLock[docIndex]); //Acquire doctor lock

	Acquire(docTokenLock);
	docToken[docIndex] = myToken; //Give token to doctor
	Release(docTokenLock);

	if (testNum == 1 || testNum == 2 || testNum == 5 || testNum == 8){	
		MyWrite("Patient %d is waiting to be examined by the Doctor in ExaminingRoom %d \n", myToken*100 + docIndex, 0);
	}

	Signal(docCV[docIndex], docLock[docIndex]); //Tell doctor to take token
	Wait(docCV[docIndex], docLock[docIndex]); //Wait for doctor to do checkup
	
	Acquire(docPrescriptionLock);
	int myPrescription = docPrescription[docIndex]; //Takes prescription
	if (testNum == 8){
		if(myPrescription == 0){
			MyWrite("Patient %d is not sick in Examining Room %d \n", myToken*100+docIndex, 0);
		}
		else{
			MyWrite("Patient %d is sick with disease %d in Examining Room %d \n", myToken*100+ myPrescription, docIndex*100);
			MyWrite("Patient %d has been prescribed medicine %d \n", myToken*100+ myPrescription, 0);
		}
	}
	Release(docPrescriptionLock);
	Signal(docCV[docIndex], docLock[docIndex]); //Notifies doctor that patient took prescrip.
	if (testNum == 8){
		MyWrite("Patient %d in Examining Room %d is waiting for the Doctor to come back from the Cashier \n", myToken*100+ docIndex, 0);
	}
	Wait(docCV[docIndex], docLock[docIndex]); //Wait for doctor to return from cashier
	if (testNum == 5 ||testNum == 7 ||testNum == 8){
		MyWrite("Patient %d is leaving Examining Room %d\n", myToken*100+ docIndex, 0);
	}	
	Release(docLock[docIndex]);
	Acquire(cashierLineLock); //Find shortest cashier line
	
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
			printf("The line Patient %d is entering belongs to Cashier %d and is currently %d people long. \n",index, lineIndex,cashierLineCount[lineIndex]);
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
		Acquire(clerkLineLock); //Find shortest clerk line
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
				MyWrite("The line Patient %d is entering is for Pharmacy Clerk %d and is currently %d people long. \n",index*100+ lineIndex, clerkLineCount[lineIndex]*100);
			}
			clerkLineCount[lineIndex]++; //Increment shortest line length
			Wait(clerkLineCV[lineIndex], clerkLineLock); //Wait till called
			clerkLineCount[lineIndex]--; //Decrement after being woken
		}
		
		Release(clerkLineLock); //Release lock on clerk line
		Acquire(clerkLock[lineIndex]); //Acquire lock to clerk
		
		Acquire(clerkTokenLock);
		clerkToken[lineIndex] = myToken; //Give token to cashier
		Release(clerkTokenLock);
		
		Acquire(clerkPrescriptionLock);
		clerkPrescription[lineIndex] = myPrescription; //Give clerk prescription
		Release(clerkPrescriptionLock);
		
		Signal(clerkCV[lineIndex], clerkLock[lineIndex]); //Wake clerk up
		if(testNum == 3 || testNum == 8){
			MyWrite("Patient %d is waiting to see PharmacyClerk %d\n", myToken*100+ lineIndex, 0);
		}
		Wait(clerkCV[lineIndex], clerkLock[lineIndex]);
		
		Acquire(medicineFeeLock);
		int myMedicineFee = medicineFee[lineIndex]; //Receive fee and meds from clerk
		if(testNum == 8){
			MyWrite("Patient %d is paying their prescription fees of %d\n", myToken*100+ myMedicineFee,0);
		}
		Release(medicineFeeLock);
		
		Signal(clerkCV[lineIndex], clerkLock[lineIndex]); //Pay clerk
		if(testNum == 3 || testNum == 8){
			MyWrite("Patient %d is leaving PharmacyClerk %d\n", myToken*100 +lineIndex, 0);
		}
		Release(clerkLock[lineIndex]);
	}
	
	//Leave hospital
	MyWrite("Patient %d is leaving the Hospital\n", myToken*100, 0);
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
	int stickCount = 0;
	int breakValCount = 0;
	while(true){
		Acquire(docReadyLock);
		docState[index] = 0;
		Acquire(docLock[index]);
		
		Release(docReadyLock);
		Acquire(dbbLock);

		if(doorBoyDoctorCount > 0){
			doorBoyDoctorCount--;
			Acquire(doorBoyLineLock);
			int dbNum = 0;
			for(int i = 0; i<doorBoyCount; i++){
				if(doorBoyState[i] == 9){
					doorBoyState[i] = 0; //Find doorboy waiting and set to open
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
			MyWrite("Doctor %d has told a DoorBoy to bring a Patient to Examining Room %d \n", index*100 + index, 0);
		}
		Release(dbbLock);
		Wait(docCV[index], docLock[index]);
		Acquire(docTokenLock);
		int token = docToken[index]; //Get patient's token number
		Release(docTokenLock);
		if(testNum == 1 || testNum == 8){
			MyWrite("Doctor %d is examining a Patient with Token %d \n",index*100+token,0);
		}
		int yieldCount = 10;
		for(int i = 0; i < yieldCount; i++){ //Check patient for that long
			Yield(); //Yield thread to simulate time spent
		}
		sickCount++;
		int sickTest = sickCount%5; //Generate if patient is sick
		/* 0 not sick
		1-4 sick */
		if (testNum == 7){
			sickTest = 1; //Making sickness static for test 8
		}
		if(testNum == 8){
			if(sickTest == 0){
				MyWrite("Doctor %d has determined that the Patient with Token %d is not sick\n", index*100+token, 0);
			}
			else{
				MyWrite("Doctor %d has determined that the Patient with Token %d is sick with disease type %d \n", index*100+token, sickTest*100);
			}
		}
		Acquire(docPrescriptionLock);
		docPrescription[index] = sickTest; //Tells patient illness and prescription
		if(testNum == 8){
			MyWrite("Doctor %d is prescribing medicine type %d to the Patient with Token %d \n", index*100+sickTest, token*100);
		}
		Release(docPrescriptionLock);
		
		Signal(docCV[index], docLock[index]); //Tells patient to take prescription
		Wait(docCV[index], docLock[index]); //Waits for patient to take prescription
		
		//Doctor tells cashiers price of consultation
		Acquire(consultLock);
		consultationFee[token] = sickTest*20+20;
		Release(consultLock);
		if(testNum == 1 || testNum == 5 || testNum == 7 ||testNum == 8){		
			MyWrite("Doctor %d tells Patient with Token %d they can leave \n", index*100+ token,0);
		}
		Signal(docCV[index], docLock[index]);//Tell patient ok to go
		Release(docLock[index]);
		
		breakValCount++;
		int breakVal = breakValCount%2; //Generate break value
		if(breakVal == 1 || (testNum == 5)){ //Take break for random time
			if (testNum == 5 || testNum == 8){
				MyWrite("Doctor %d tells a DoorBoy he is going on break \n", index*100, 0);
			}
			Acquire(docReadyLock); //Acquire doctor ready lock
			docState[index] = 2; //Sets own state to ready
			Release(docReadyLock);
			int breakTimeVal = 10; //Random between 5 and 15
			if (testNum == 5){
				MyWrite("Doctor %d is going on break for %d milliseconds. \n",index*100+ breakTimeVal, 0);
			}
			for(int i = 0; i < breakTimeVal; i++){
				Yield();
			}
			if (testNum == 5 || testNum == 8){
				MyWrite("Doctor %d tells a DoorBoy he is coming off break \n", index*100, 0);
			}
		}
		if (testNum == 5){
			MyWrite("To simulate test 5, Doctor %d is quitting. \n",index*100,0);
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
		Acquire(clerkLineLock); //Acquire line lock
		clerkState[index]=0; //Set self to not busy
		
		if(clerkLineCount[index] > 0) { //Check to see if anyone in line
			if (testNum == 3 || testNum == 8){
				MyWrite("PharmacyClerk %d has signaled a Patient. \n",index*100, 0);
			}
			Signal(clerkLineCV[index], clerkLineLock); //Signal first person in line
			clerkState[index] = 1; //Set self to busy
		}
		
		Acquire(clerkLock[index]); //Acquire clerk lock
		Release(clerkLineLock); //Release line lock
		Wait(clerkCV[index], clerkLock[index]); //Wait for patient to arrive
		
		Acquire(clerkTokenLock);
		int token = clerkToken[index];
		Release(clerkTokenLock);
		
		Acquire(clerkPrescriptionLock);
		int prescription = clerkPrescription[index]; //Get prescription from patient
		if (testNum == 3 || testNum == 8){
			MyWrite("PharmacyClerk %d gets Prescription %d from Patient with Token %d \n", index*100+prescription, token*100);
		}
		Release(clerkPrescriptionLock);

		
		int fee = prescription*25; //Calculate fee of medicine
		
		Acquire(medicineFeeLock);
		medicineFee[index] = fee; //Tell patient fee
		Release(medicineFeeLock);
		if (testNum == 3 || testNum == 8){
			MyWrite("PharmacyClerk %d gives Prescription %d from Patient with Token %d \n", index*100+ prescription, token*100);
			MyWrite("PharmacyClerk %d tells Patient with Token %d they owe %d \n", index*100+ token, fee*100);
		}
		Signal(clerkCV[index], clerkLock[index]);
		Wait(clerkCV[index], clerkLock[index]); //Wait for patient to give money and take prescription
		if (testNum == 3 || testNum == 8){
			MyWrite("Pharmacyclerk %d gets money from Patient with Token %d \n", index*100+ token, 0);
		}
		Acquire(totalMedicineLock);
		totalMedicineCost += fee; //Add medicine fee to total count
		Release(totalMedicineLock);
		
		Release(clerkLock[index]); //Release clerk lock
		
		//Take break check
		Acquire(clerkLineLock);
		if(clerkLineCount[index] == 0){ //If noone in line
			clerkState[index] = 2; //Set to on-break
			Release(clerkLineLock);
			if (testNum == 6 || testNum == 8){
				MyWrite("PharmacyClerk %d is going on break. \n",index*100, 0);
			}
			Acquire(clerkBreakLock);
			Wait(clerkBreakCV[index], clerkBreakLock); //Set condition for manager to callback
			Release(clerkBreakLock);
			if (testNum == 6 || testNum == 8){
				MyWrite("PharmacyClerk %d is coming off break. \n",index*100, 0);
			}
		}
		else{
			Release(clerkLineLock);
		}
	}
}

void
Manager(){
	while(true){
		//Checks hospital is running randomly
		//Randomly generate number and yield for other threads
		int yield = 70;
		if (testNum == 7){
			yield = 5020;
		}
		for(int i = 0; i<yield; i++){
			Yield();
		}
		
		//Wakes up receptionist
		//Check if there are at least 2 people waiting in receptionist line
		Acquire(recLineLock);
		int lineCnt = 0;
		for(int i = 0; i < recCount; i++){
			lineCnt+= recLineCount[i];
		}
		if(lineCnt > 1){
			for(int i = 0; i < recCount; i++){
				if(recState[i] == 2 ){
					Acquire(receptionistBreakLock);
					//Set receptionist to off break
					if (testNum == 6 || testNum == 8){
						MyWrite("HospitalManager signaled a Receptionist to come off break\n", 0, 0);
					}
					Signal(receptionistBreakCV[i], receptionistBreakLock);
					Release(receptionistBreakLock);
				}
			}
		}
		Release(recLineLock);

		
		//Wakes up door boy
		if (testNum != 2){
			Acquire(doorBoyLineLock);
		if(doorBoyLineCount > 0){ //Check if any patient in line
			for(int i = 0; i < doorBoyCount; i++){
				if(doorBoyState[i] == 2){
					Acquire(doorBoyBreakLock);
					//Set door boy to off break
					if (testNum == 6 || testNum == 8){
						MyWrite("HospitalManager signaled a DoorBoy to come off break\n",0 ,0);
					}
					Signal(doorBoyBreakCV[i], doorBoyBreakLock);
					Release(doorBoyBreakLock);
				}
			}
		}
		Release(doorBoyLineLock);
	}
		//Wakes up cashier
	Acquire(cashierLineLock);
	for( int i = 0; i < cashierCount; i++){
			if(cashierLineCount[i] > 0 && cashierState[i] == 2){ //Check if any patient in line
				Acquire(cashierBreakLock);
				if (testNum == 6 || testNum == 8){
					MyWrite("HospitalManager signaled a Cashier to come off break\n", 0, 0);
				}
				Signal(cashierBreakCV[i], cashierBreakLock); //Set cashier to off break
				Release(cashierBreakLock);
				//Get total consultation fee
			}
		}
		Release(cashierLineLock);
		
		

		//Wakes up clerk
		Acquire(clerkLineLock);
		for(int i = 0; i<clerkCount; i++){
			if(clerkLineCount[i] > 0 && clerkState[i] == 2){
				Acquire(clerkBreakLock);
				if (testNum == 6 || testNum == 8){
					MyWrite("HospitalManager signaled a PharmacyClerk to come off break\n", 0, 0);
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
			int myConsultFee = totalConsultationFee;
			MyWrite("HospitalManager reports that total consultancy fees are %d\n", myConsultFee*100, 0);
			Release(totalFeeLock);

			Acquire(totalMedicineLock);
			int myMedicineFee = totalMedicineCost;
			MyWrite("HospitalManager reports total sales in pharmacy are %d\n", myMedicineFee*100, 0);
			Release(totalMedicineLock);
		}
		/* 
		* COMMENT OUT BETWEEM TO REMOVE SPAM FROM HOSPITAL MANAGER 
		*/
		if (testNum == 2 || testNum == 4 || testNum == 5 || testNum == 6){
			break;
		}

		if (completedPatientThreads == numPatients){
			// MyWrite("No patients left, Manager leaving \n");
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
			recCount = 2;
			docCount = 2;
			doorBoyCount = 2;	
			cashierCount = 2;
			clerkCount = 2;
			numPatients = 6;
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

int main() {
	
}

