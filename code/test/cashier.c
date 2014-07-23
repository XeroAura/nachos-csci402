#include "setup.h"

void
Cashier(){
	int fee = 0;
	int token = 0;
	int index;
	Acquire(cashierIndexLock);
	index = nextCashierIndex;
	nextCashierIndex++;
	Release(cashierIndexLock);
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
		token = cashierToken[index];
		if (testNum == 3 || testNum == 8){
			MyWrite("Cashier %d gets Token %d from a Patient \n", sizeof("Cashier %d gets Token %d from a Patient \n")-1, index*100+token, 0);
		}
		Release(cashierTokenLock);
		
		
		Acquire(cashierFeeLock);
		cashierFee[index] = fee;
		Release(cashierFeeLock);
		
		Signal(cashierCV[index], cashierLock[index]);
		if (testNum == 3 || testNum == 8){
			MyWrite("Cashier %d tells Patient with Token %d they owe %d \n", sizeof("Cashier %d tells Patient with Token %d they owe %d \n")-1, index*100+token, fee*100);
		}
		Wait(cashierCV[index], cashierLock[index]);
		if (testNum == 3 || testNum == 8){
			MyWrite("Cashier %d receives fees from Patient with Token %d \n", sizeof("Cashier %d receives fees from Patient with Token %d \n")-1, index*100+token, 0);
		}
		Acquire(totalFeeLock);
		totalConsultationFee += 25;
		Release(totalFeeLock);
		
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
	Exit(0);
}