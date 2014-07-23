#include "setup.h"

void
Manager(){
	int i = 0;
	int yield = 70;
	int lineCnt = 0;
	int myConsultFee = 0;
	int myMedicineFee = 0;
	while(1){
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
					MyWrite("HospitalManager signaled a Receptionist to come off break\n", sizeof("HospitalManager signaled a Receptionist to come off break\n")-1, 0, 0);
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
						MyWrite("HospitalManager signaled a DoorBoy to come off break\n", sizeof("HospitalManager signaled a DoorBoy to come off break\n")-1,0 ,0);
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
				MyWrite("HospitalManager signaled a Cashier to come off break\n", sizeof("HospitalManager signaled a Cashier to come off break\n")-1, 0, 0);
				Signal(cashierBreakCV[i], cashierBreakLock);
				Release(cashierBreakLock);
			}
		}
		Release(cashierLineLock);
		
		Acquire(clerkLineLock);
		for(i = 0; i<clerkCount; i++){
			if(clerkLineCount[i] > 0 && clerkState[i] == 2){
				Acquire(clerkBreakLock);
				MyWrite("HospitalManager signaled a PharmacyClerk to come off break\n", sizeof("HospitalManager signaled a PharmacyClerk to come off break\n")-1, 0, 0);
				Signal(clerkBreakCV[i], clerkBreakLock);
				Release(clerkBreakLock);
			}
		}
		Release(clerkLineLock);
		
		
		/*
		* COMMENT OUT BETWEEN TO REMOVE SPAM FROM HOSPITAL MANAGER
		
		Acquire(totalFeeLock);
		myConsultFee = totalConsultationFee;
		MyWrite("HospitalManager reports that total consultancy fees are %d\n", sizeof("HospitalManager reports that total consultancy fees are %d\n")-1, myConsultFee*100, 0);
		Release(totalFeeLock);

		Acquire(totalMedicineLock);
		myMedicineFee = totalMedicineCost;
		MyWrite("HospitalManager reports total sales in pharmacy are %d\n", sizeof("HospitalManager reports total sales in pharmacy are %d\n")-1, myMedicineFee*100, 0);
		Release(totalMedicineLock);
		
		* COMMENT OUT BETWEEN TO REMOVE SPAM FROM HOSPITAL MANAGER
		*/
		
		if (completedPatientThreads == numPatients){
			break;
		}
		
	}
	Exit(0);
}