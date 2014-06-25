/* Test hospital file
 * 
*/

#include "syscall.h"

/*Global variables*/
int completedPatientThreads = 0;
int numPatients = 0;
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

/*Doctor globals*/
int docLock[5];
int docCV[5];
int docState[5] = {1,1,1,1,1}; 
int docToken[5] = {-1,-1,-1,-1,-1};
int docTokenLock = CreateLock(0);
int docPrescriptionLock = CreateLock(0); 
int docPrescription[5] = {0,0,0,0,0}; 
int docCount = 5; 

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
Patient(int index){
	MyWrite("Patient %d has arrived at the Hospital. \n", sizeof("Patient %d has arrived at the Hospital. \n")-1, index*100, 0);
	
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
	int myPrescription = docPrescription[docIndex]; 
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
	if(myPrescription != 0){ 
		Acquire(clerkLineLock); 
		shortest = clerkLineCount[0]; 
		lineIndex = 0; 
		for(int i=0; i<clerkCount; i++){ 
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
		int myMedicineFee = medicineFee[lineIndex]; 
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
	int dbNum = 0;
	int i = 0;
	int token = 0;
	int sickTest =0;
	int yieldCount = 10;
	int breakVal = 0;
	int breakTimeVal = 10;
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
	int token = 0;
	int fee = 0;
	int prescription = 0;
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
	Thread *t;
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

