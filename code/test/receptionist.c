void
Receptionist(){
	int index;
	int nextRecIndex = CreateMV("nextRecIndex",sizeof("nextRecIndex"),1); //name, size, size of array  (since MV is an array of integers)
	int recIndexLock = CreateLock("recIndexLock",sizeof("recIndexLock"));
	int recState = CreateMV("recState",sizeof("recState"),5);
	recLineCV[];
	recLineLock;
	recLineCount;
	tokenLock
	recTokens[]
	nextToken
	recCV[]
	recLock[]
	receptionistBreakCV
	receptionistBreakLock


	/*
	Instatiate all global variables using CreateMV. Same with Locks and CV's and stuff.
	Any time you want to change a global variable, use SetMV(index in MV array, the index of the array inside the MV, new value to set)
	Any time you want to get the value of a global variable, use GetMV(index in MV Array, index of the variable in the array inside MV)
	Destroy stuff at the end if necessary
	*/





	Acquire(recIndexLock);
	index = nextRecIndex;
	nextRecIndex++;
	Release(recIndexLock);

	while(1){		
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
	Exit(0);	
}
