#include "setup.h"


	/*
	Instatiate all global variables using CreateMV. Same with Locks and CV's and stuff.
	Any time you want to change a global variable, use SetMV(index in MV array, the index of the array inside the MV, new value to set)
	Any time you want to get the value of a global variable, use GetMV(index in MV Array, index of the variable in the array inside MV)
	Destroy stuff at the end if necessary
	*/

void
Cashier(){
	setup();
	int fee = 0;
	int token = 0;

	int index = 0;
	Acquire(cashierIndexLock);
	index = nextCashierIndex;
	nextCashierIndex++;
	Release(cashierIndexLock);

	while(1){
		Acquire(cashierLineLock);
		cashierState[index]=0;
		
		if(cashierLineCount[index] > 0) {
			Signal(cashierLineCV[index],cashierLineLock);
			MyWrite("Cashier %d has signaled a Patient \n", sizeof("Cashier %d has signaled a Patient \n")-1, index*100, 0);
			cashierState[index]=1;
		}
		
		Acquire(cashierLock[index]);
		Release(cashierLineLock);
		
		Wait(cashierCV[index],cashierLock[index]);
		
		// Acquire(cashierTokenLock);
		token = cashierToken[index];
		MyWrite("Cashier %d gets Token %d from a Patient \n", sizeof("Cashier %d gets Token %d from a Patient \n")-1, index*100+token, 0);
		// Release(cashierTokenLock);
		
		
		// Acquire(cashierFeeLock);
		cashierFee[index] = fee;
		// Release(cashierFeeLock);
		
		Signal(cashierCV[index], cashierLock[index]);
		MyWrite("Cashier %d tells Patient with Token %d they owe %d \n", sizeof("Cashier %d tells Patient with Token %d they owe %d \n")-1, index*100+token, fee*100);
		Wait(cashierCV[index], cashierLock[index]);
		MyWrite("Cashier %d receives fees from Patient with Token %d \n", sizeof("Cashier %d receives fees from Patient with Token %d \n")-1, index*100+token, 0);
		Acquire(totalFeeLock);
		totalConsultationFee += 25;
		Release(totalFeeLock);
		
		Release(cashierLock[index]);
		
		Acquire(cashierLineLock);
		
		if(cashierLineCount[index] == 0){
			MyWrite("Cashier %d is going on break \n", sizeof("Cashier %d is going on break \n")-1, index*100, 0);
			cashierState[index] = 2;
			Acquire(cashierBreakLock);
			Release(cashierLineLock);
			Wait(cashierBreakCV[index],cashierBreakLock);
			MyWrite("Cashier %d is coming off break \n", sizeof("Cashier %d is coming off break \n")-1, index*100, 0);
			Release(cashierBreakLock);
		}
		else{
			Release(cashierLineLock);
		}
	}
	Exit(0);
}