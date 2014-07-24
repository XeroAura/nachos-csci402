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
int nextCashierIndex; //Used for giving each cashier an unique index
int totalConsultationFee; //Total consultation fee
int cashierLineCount; //Number of people in each cashier line
int cashierState; //State of cashier 1 - busy, 0 - free
int cashierToken; //Token number passed to cashier by patient
int cashierFee; //Fee charged by cashier to patient

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
int clerkState;
int clerkLineCount;
int clerkToken;
int clerkPrescription;
int medicineFee;
int nextClerkIndex;
int totalMedicineCost;

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
	nextCashierIndex = CreateMV("nextCashierIndex", sizeof("nextCashierIndex"), 1);
	SetMV(nextCashierIndex, 0, 0);
	totalConsultationFee = CreateMV("totalConsultationFee", sizeof("totalConsultationFee"), 1);
	SetMV(totalConsultationFee, 0, 0);
	cashierLineCount = CreateMV("cashierLineCount", sizeof("cashierLineCount"), 5); //{0,0,0,0,0}; //Number of people in each cashier line
	for(int i = 0; i < 5; i++){
		SetMV(cashierLineCount, i, 0);
	}
	cashierState = CreateMV("cashierState", sizeof("cashierState"), 5); //{1,1,1,1,1}; //State of cashier 1 - busy, 0 - free
	for(int i = 0; i < 5; i++){
		SetMV(cashierState, i, 1);
	}
	cashierToken = CreateMV("cashierToken", sizeof("cashierToken"), 5);  //{0,0,0,0,0}; //Token number passed to cashier by patient
	for(int i = 0; i < 5; i++){
		SetMV(cashierToken, i, 0);
	}
	cashierFee = CreateMV("cashierFee", sizeof("cashierFee"), 5);  //{0,0,0,0,0}; //Fee charged by cashier to patient
	for(int i = 0; i < 5; i++){
		SetMV(cashierFee, i, 0);
	}

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
	// cashierTokenLock= CreateLock("cashierTokenLock", sizeof("cashierTokenLock"));
	// cashierFeeLock = CreateLock("cashierFeeLock", sizeof("cashierFeeLock"));
	cashierIndexLock = CreateLock("cashierIndexLock", sizeof("cashierIndexLock"));
	
//Clerk stuff
	clerkState = CreateMV("clerkState", sizeof("clerkState"), 5); //{1,1,1,1,1};
	for(int i = 0; i < 5; i++){
		SetMV(clerkState, i, 1);
	}
	clerkLineCount = CreateMV("clerkLineCount", sizeof("clerkLineCount"), 5); //{0,0,0,0,0};
	for(int i = 0; i < 5; i++){
		SetMV(clerkLineCount, i, 0);
	}
	clerkToken = CreateMV("clerkToken", sizeof("clerkToken"), 5); //{0,0,0,0,0};
	for(int i = 0; i < 5; i++){
		SetMV(clerkToken, i, 0);
	}
	clerkPrescription = CreateMV("clerkPrescription", sizeof("clerkPrescription"), 5); // {0,0,0,0,0};
	for(int i = 0; i < 5; i++){
		SetMV(clerkPrescription, i, 0);
	}
	medicineFee = CreateMV("medicineFee", sizeof("medicineFee"), 5); //{0,0,0,0,0};
	for(int i = 0; i < 5; i++){
		SetMV(medicineFee, i, 0);
	}
	nextClerkIndex = CreateMV("nextClerkIndex", sizeof("nextClerkIndex"), 1);
	SetMV(nextClerkIndex, 0, 0);
	totalMedicineCost = CreateMV("totalMedicineCost", sizeof("totalMedicineCost"), 1);
	SetMV(totalMedicineCost, 0, 0);

	for(i = 0; i < 5; i++){
		clerkLock[i] = CreateLock("clerkLock"+i, sizeof("clerklock")+1);
	}
	for (i = 0; i < 5; i++){
		clerkCV[i] = CreateCondition("clerkCV"+i, sizeof("clerkCV")+1);
	}
	
	for (i = 0; i < 5; i++){
		clerkLineCV[i] = CreateCondition("clerkLineCV"+i, sizeof("clerkLineCV")+1);
	}
	
	// medicineFeeLock= CreateLock("medicineFeeLock", sizeof("medicineFeeLock"));
	totalMedicineLock = CreateLock("totalMedicineLock", sizeof("totalMedicineCost"));
	clerkLineLock = CreateLock("clerkLineLock", sizeof("clerkLineLock"));
	// clerkPrescriptionLock = CreateLock("clerkPrescriptionLock", sizeof("clerkPrescriptionLock"));
	// clerkTokenLock = CreateLock("clerkTokenLock", sizeof("clerkTokenLock"));
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
