/*Global variables*/
int completedPatientThreads = 0;
int numPatients = 0;
int testNum = 0;
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


/*Cashier globals */
int totalFeeLock;
int totalConsultationFee = 0;
int cashierCount = 5;

int cashierLineLock;
int cashierLineCount[5] = {0,0,0,0,0};
int cashierLineCV[5];

int cashierLock[5];
int cashierCV[5];

int cashierState[5] = {1,1,1,1,1};
int cashierToken[5] = {0,0,0,0,0};
int cashierTokenLock;
int cashierFee[5] = {0,0,0,0,0};
int cashierFeeLock;

int cashierIndexLock;
int nextCashierIndex = 0;

/*Clerk globals*/
int medicineFeeLock;
int medicineFee[5] = {0,0,0,0,0};
int totalMedicineLock;
int totalMedicineCost = 0;
int clerkCount = 5;

int clerkLineLock;
int clerkLineCount[5] = {0,0,0,0,0};
int clerkLineCV[5];
int clerkState[5] = {1,1,1,1,1};
int clerkPrescription[5] = {0,0,0,0,0};
int clerkPrescriptionLock;

int clerkTokenLock;
int clerkToken[5] = {0,0,0,0,0};

int clerkLock[5];
int clerkCV[5];

int clerkIndexLock;
int nextClerkIndex = 0;

/*Manager globals*/
int receptionistBreakLock;
int receptionistBreakCV[5];
int doorBoyBreakLock;
int doorBoyBreakCV[5];
int cashierBreakLock;
int cashierBreakCV[5];
int clerkBreakLock;
int clerkBreakCV[5];

void setup(){

}
