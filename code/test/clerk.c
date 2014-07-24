#include "setup.h"

void
Clerk(){
	int token = 0;
	int fee = 0;
	int prescription = 0;
	int index = 0;

	Acquire(clerkIndexLock);
	index = nextClerkIndex;
	nextClerkIndex++;
	Release(clerkIndexLock);
	
	while(1){
		Acquire(clerkLineLock);
		clerkState[index]=0;
		
		if(clerkLineCount[index] > 0) {
			MyWrite("PharmacyClerk %d has signaled a Patient. \n", sizeof("PharmacyClerk %d has signaled a Patient. \n")-1,index*100, 0);
			Signal(clerkLineCV[index], clerkLineLock);
			clerkState[index] = 1;
		}
		
		Acquire(clerkLock[index]);
		Release(clerkLineLock);
		Wait(clerkCV[index], clerkLock[index]);
		
		//Acquire(clerkTokenLock);
		token = clerkToken[index];
		//Release(clerkTokenLock);
		
		// Acquire(clerkPrescriptionLock);
		prescription = clerkPrescription[index];
		MyWrite("PharmacyClerk %d gets Prescription %d from Patient with Token %d \n", sizeof("PharmacyClerk %d gets Prescription %d from Patient with Token %d \n")-1, index*100+prescription, token*100);
		// Release(clerkPrescriptionLock);
		
		
		fee = prescription*25;
		
		// Acquire(medicineFeeLock);
		medicineFee[index] = fee;
		// Release(medicineFeeLock);
		MyWrite("PharmacyClerk %d gives Prescription %d from Patient with Token %d \n", sizeof("PharmacyClerk %d gives Prescription %d from Patient with Token %d \n")-1, index*100+ prescription, token*100);
		MyWrite("PharmacyClerk %d tells Patient with Token %d they owe %d \n", sizeof("PharmacyClerk %d tells Patient with Token %d they owe %d \n")-1, index*100+ token, fee*100);
		Signal(clerkCV[index], clerkLock[index]);
		Wait(clerkCV[index], clerkLock[index]);
		MyWrite("Pharmacyclerk %d gets money from Patient with Token %d \n", sizeof("Pharmacyclerk %d gets money from Patient with Token %d \n")-1, index*100+ token, 0);
		Acquire(totalMedicineLock);
		totalMedicineCost += fee;
		Release(totalMedicineLock);
		
		Release(clerkLock[index]);
		
		Acquire(clerkLineLock);
		if(clerkLineCount[index] == 0){
			clerkState[index] = 2;
			Release(clerkLineLock);
			MyWrite("PharmacyClerk %d is going on break. \n", sizeof("PharmacyClerk %d is going on break. \n")-1, index*100, 0);
			Acquire(clerkBreakLock);
			Wait(clerkBreakCV[index], clerkBreakLock);
			Release(clerkBreakLock);
			MyWrite("PharmacyClerk %d is coming off break. \n", sizeof("PharmacyClerk %d is coming off break. \n")-1, index*100, 0);
		}
		else{
			Release(clerkLineLock);
		}
	}
	Exit(0);
}