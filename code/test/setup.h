/*Global variables*/
int completedPatientThreads = 0;
int numPatients = 20; // 20 Patients for final test

int patientIndexLock;
int nextPatientIndex = 0;

/*Receptionist globals*/
int recLineLock;
int recLineCount[5] = {0,0,0,0,0};
int recLineCV[5];
int recState[5] = {1,1,1,1,1};

int recTokens[5] = {0,0,0,0,0};
int recLock[5];
int recCV[5];
int tokenLock;
int nextToken = 0;
int recCount = 5;

int recIndexLock;
int nextRecIndex = 0;

/*Doctor globals*/
int docLock[5];
int docCV[5];
int docState[5] = {1,1,1,1,1};
int docToken[5] = {-1,-1,-1,-1,-1};
int docTokenLock;
int docPrescriptionLock;
int docPrescription[5] = {0,0,0,0,0};
int docCount = 5;
int nextDoctorIndex = 0;
int doctorIndexLock ;

/*Doorboy globals*/
int doorBoyLineLock;
int doorBoyLineCV;
int doorBoyLineCount = 0;

int doorBoyDoctorCount = 0;
int doctorDoorBoyCount = 0;
int dbbLock;
int doctorDoorBoyCV[5];
int doorBoyDoctorCV[5];

int docReadyLock;
int docReadyCV[5];

int doorBoyCount = 5;
int doorBoyState[5] = {1,1,1,1,1};

int doorBoyPatientCV[5];
int doorBoyPatientLock;
int doorBoyToken[5] = {-1,-1,-1,-1,-1};

int doorBoyTokenLock;
int doorBoyPatientRoomLock;
int doorBoyPatientRoom[5] = {-1,-1,-1,-1,-1};

int doorBoyIndexLock;
int nextDoorBoyIndex = 0;

/* ----------------------------------------------------------------- */

/*Cashier globals */
int totalFeeLock; //Lock for total fee charged to patients
int cashierCount = 5; //5 cashiers
int cashierLineLock; //Lock for cashier ilnes
int cashierLineCV[5]; //Line CV for cashiers
int cashierLock[5]; //Lock for cashier interactions
int cashierCV[5]; //CV for cashier interactions
int cashierIndexLock; //Lock for counting cashiers at startup

//Extra locks
int cashierTokenLock;
int cashierFeeLock;

//Monitor Variables
int nextCashierIndex = 0; //Used for giving each cashier an unique index
int totalConsultationFee = 0; //Total consultation fee
int cashierLineCount[5] = {0,0,0,0,0}; //Number of people in each cashier line
int cashierState[5] = {1,1,1,1,1}; //State of cashier 1 - busy, 0 - free
int cashierToken[5] = {0,0,0,0,0}; //Token number passed to cashier by patient
int cashierFee[5] = {0,0,0,0,0}; //Fee charged by cashier to patient

/* ----------------------------------------------------------------- */

/*Clerk globals*/
int totalMedicineLock; //Total medicine fees Lock
int clerkCount = 5; //5 clerks

int clerkLineLock;
int clerkLineCV[5];

int clerkLock[5];
int clerkCV[5];
int clerkIndexLock; //Lock for counting clerks

//Extra Locks
int clerkTokenLock;
int clerkPrescriptionLock;
int medicineFeeLock;

//Monitor Variables
int clerkState[5] = {1,1,1,1,1};
int clerkLineCount[5] = {0,0,0,0,0};
int clerkToken[5] = {0,0,0,0,0};
int clerkPrescription[5] = {0,0,0,0,0};
int medicineFee[5] = {0,0,0,0,0};
int nextClerkIndex = 0;
int totalMedicineCost = 0;

/* ----------------------------------------------------------------- */

/*Manager globals*/
int receptionistBreakLock;
int receptionistBreakCV[5];
int doorBoyBreakLock;
int doorBoyBreakCV[5];
int cashierBreakLock;
int cashierBreakCV[5];
int clerkBreakLock;
int clerkBreakCV[5];

/* ----------------------------------------------------------------- */

void setup(){

	//Patient stuff
	

	//Receptionist stuff


	//Doorboy stuff


	//Doctor stuff


	//Cashier stuff
	for(i = 0; i < 5; i++){
		cashierLock[i] = CreateLock("cashierLock"+i, sizeof("cashierLock")+1);
	}
	for (i = 0; i < 5; i++){
		cashierCV[i] = CreateCondition("cashierCV"+i, sizeof("cashierCV")+1);
	}
	for (i = 0; i < 5; i++){
		cashierLineCV[i] = CreateCondition("cashierLineCV"+i, sizeof("cashierLineCV")+1);
	}
	totalFeeLock = CreateLock("totalFeeLock", sizeof("totalFeeLock"));
	cashierLineLock= CreateLock("cashierLineLock", sizeof("cashierLineLock"));
	cashierTokenLock= CreateLock("cashierTokenLock", sizeof("cashierTokenLock"));
	cashierFeeLock = CreateLock("cashierFeeLock", sizeof("cashierFeeLock"));
	cashierIndexLock = CreateLock("cashierIndexLock", sizeof("cashierIndexLock"));
	
	//Clerk stuff
	for(i = 0; i < 5; i++){
		clerkLock[i] = CreateLock("clerkLock"+i, sizeof("clerklock")+1);
	}
	for (i = 0; i < 5; i++){
		clerkCV[i] = CreateCondition("clerkCV"+i, sizeof("clerkCV")+1);
	}
	
	for (i = 0; i < 5; i++){
		clerkLineCV[i] = CreateCondition("clerkLineCV"+i, sizeof("clerkLineCV")+1);
	}
	
	medicineFeeLock= CreateLock("medicineFeeLock", sizeof("medicineFeeLock"));
	totalMedicineLock = CreateLock("totalMedicineLock", sizeof("totalMedicineCost"));
	clerkLineLock = CreateLock("clerkLineLock", sizeof("clerkLineLock"));
	clerkPrescriptionLock = CreateLock("clerkPrescriptionLock", sizeof("clerkPrescriptionLock"));
	clerkTokenLock = CreateLock("clerkTokenLock", sizeof("clerkTokenLock"));
	clerkIndexLock= CreateLock("clerkIndexLock", sizeof("clerkIndexLock"));
	
	//Manager stuff
	receptionistBreakLock = CreateLock("receptionistBreakLock", sizeof("receptionistBreakLock"));
	doorBoyBreakLock = CreateLock("doorBoyBreakLock", sizeof("doorBoyBreakLock"));
	cashierBreakLock = CreateLock("cashierBreakLock", sizeof("cashierBreakLock"));
	clerkBreakLock  = CreateLock("clerkBreakLock", sizeof("clerkBreakLock"));
	for (i = 0; i < 5; i++){
		receptionistBreakCV[i] = CreateCondition("receptionistBreakCV"+i, sizeof("receptionistBreakCV")+1);
	}
	for (i = 0; i < 5; i++){
		doorBoyBreakCV[i] = CreateCondition("doorBoyBreakCV"+i, sizeof("doorBoyBreakCV")+1);
	}
	for (i = 0; i < 5; i++){
		clerkBreakCV[i] = CreateCondition("clerkBreakCV"+i, sizeof("clerkBreakCV")+1);
	}
	for (i = 0; i < 5; i++){
		cashierBreakCV[i] = CreateCondition("cashierBreakCV"+i, sizeof("cashierBreakCV")+1);
	}

}
