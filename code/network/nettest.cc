/*nettest.cc */
/*	Test out message delivery between two "Nachos" machines,*/
/*	using the Post Office to coordinate delivery.*/
/**/
/*	Two caveats:*/
/*	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:*/
/*		./nachos -m 0 -o 1 &*/
/*		./nachos -m 1 -o 0 &*/
/**/
/*	  2. You need an implementation of condition variables,*/
/*	     which is *not* provided as part of the baseline threads */
/*	     implementation.  The Post Office won't work without*/
/*	     a correct implementation of condition variables.*/
/**/
/*Copyright (c) 1992-1993 The Regents of the University of California.*/
/*All rights reserved.  See copyright.h for copyright notice and limitation */
/*of liability and disclaimer of warranty provisions.*/

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"

#ifdef CHANGED
#include <sstream>
#include <string>
#include <iostream>

using namespace std;

extern const int MAX_CVS;
extern const int MAX_LOCKS;
extern KernelLock* kLocks[];
extern KernelCV* kCV[]; 
extern KernelMV* MVArray[500];
extern int mailboxID;
#endif

ServerLock* sLocks[500];
ServerCV* sCV[500];

/*Test out message delivery, by doing the following:*/
/*	1. send a message to the machine with ID "farAddr", at mail box #0*/
/*	2. wait for the other machine's message to arrive (in our mailbox #0)*/
/*	3. send an acknowledgment for the other machine's message*/
/*	4. wait for an acknowledgement from the other machine to our */
/*	    original message*/

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data = "Hello there!";
    char *ack = "Got it!";
    char buffer[MaxMailSize];

/*    construct packet, mail header for original message*/
/*    To: destination machine, mailbox 0*/
/*    From: our machine, reply to: mailbox 1*/
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

/*    Send the first message*/
    bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
  }

/*    Wait for the first message from the other machine*/
  postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
  printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
  fflush(stdout);

/*    Send acknowledgement to the other machine (using "reply to" mailbox*/
/*    in the message that just arrived*/
  outPktHdr.to = inPktHdr.from;
  outMailHdr.to = inMailHdr.from;
  outMailHdr.length = strlen(ack) + 1;
  success = postOffice->Send(outPktHdr, outMailHdr, ack); 

  if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
  }

/*    Wait for the ack from the other machine to the first message we sent.*/
  postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
  printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
  fflush(stdout);

/*    Then we're done!*/
  interrupt->Halt();
}


void
Server(){
    int nextLockIndex = 0;
    int nextCVIndex = 0;
    int nextMVIndex = 0;

    mailboxID = 0;
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data = "Hello there!";
    char* buffer;
    char* oSuccess = new char;
    sprintf(oSuccess, "%d", 1);
    char* oFailure = new char;
    sprintf(oFailure, "%d", -1);
    string key;
    string outputString;

    while(1){

/*    construct packet, mail header for original message*/
/*    To: destination machine, mailbox 1*/
/*    From: our machine, reply to: mailbox 0*/
        printf("%d\n",mailboxID);
        buffer = new char[MaxMailSize]; 
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        outPktHdr.to = inPktHdr.from;     
        outMailHdr.to = inMailHdr.from;
        outMailHdr.from = 0;
        outMailHdr.length = strlen(data) + 1;
/*    Wait for the first message from the other machine*/
        fflush(stdout);

/*    parses the buffer to figure out what command is being sent*/
        outputString = string(buffer);
        key = outputString.substr(0,2);
        buffer += 2;

/*    Create Lock request - assumes that the lock name has no spaces*/
        if (key == "CL"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* name = new char[MaxMailSize];
            int addr;
            int machineID;
            char* output = new char[3];
            printf("Create Lock syscall\n");
            ss1 << buffer;
            ss1 >> name >> addr >> machineID;

/*            checking to see if the lock has already been made from another program*/
            for (int i = 0; i < nextLockIndex; i++){
                printf("Comparing %s to %s \n", kLocks[i]->name,name);
                if(strcmp(kLocks[i]->name,name)==0){
                    printf("Names are identical\n");
                    kLocks[i]->requestThreads++;
                    sprintf(output,"%d",i);
                    success = postOffice->Send(outPktHdr,outMailHdr,output);
                    flag = true;
                    break;
                }
            }
            if (!flag){
                KernelLock* tempLock = new KernelLock;
                tempLock->name = name;
                tempLock->as = (AddrSpace*)addr;
                tempLock->isToBeDestroyed = false;
                tempLock->lock = new Lock(name);
                if (nextLockIndex < 500){
                    kLocks[nextLockIndex] = tempLock;
                    kLocks[nextLockIndex]->requestThreads++;
                    sLocks[nextLockIndex] = new ServerLock();

                    nextLockIndex++;
                    sprintf(output, "%d",nextLockIndex-1);
                    success = postOffice->Send(outPktHdr, outMailHdr, output); 

                } else {
                    printf("ERROR: Maximum number of locks reached. Current number of locks is %d. \n", nextLockIndex);
                    success = postOffice->Send(outPktHdr,outMailHdr,oFailure);
                    break;
                }
            }
            flag = false;
        }

/*        Destroy lock syscall*/
        if (key == "DL"){
            stringstream ss1;
            bool success;
            bool flag = false;
            int addr;
            int machineID;
            int index;
            printf("Destroy Lock syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr >> machineID;
            if (index >= 0 && index < MAX_LOCKS){
                if (kLocks[index]->lock == NULL){
                    printf("ERROR: No lock exists here.\n");
                    success = postOffice->Send(outPktHdr,outMailHdr,oFailure);
                } else {
                    kLocks[index]->isToBeDestroyed = true;
                    if (kLocks[index]->isToBeDestroyed && kLocks[index]->lock->getFree()){
                        kLocks[index]->requestThreads--;
                        if (kLocks[index]->requestThreads == 0){
                            delete kLocks[index]->lock;
                            delete kLocks[index];
                            delete sLocks[index];
                            kLocks[index] = NULL;
                            sLocks[index] = NULL;
                            printf("All processes asked for its destruction: destroyed lock.\n");                            
                        }
                    }
                    printf("Lock successfully set for destruction\n");
                    success = postOffice->Send(outPktHdr,outMailHdr,oSuccess);
                }
            } else {
                printf("ERROR: Index is out of bounds.\n");
                success = postOffice->Send(outPktHdr,outMailHdr,oFailure);
            }
        }

/* Acquire lock syscall*/
        if (key == "AL"){
            stringstream ss1;
            bool success;
            bool flag = false;
            int addr;
            int machineID;
            int index;
            char* output = new char[1];
            int machineID = inMailHdr.from;
            printf("Acquire Lock syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr >> machineID;

            if (index >= 0 && index < MAX_LOCKS){
                if (sLocks[index]->state == 0){
                    sLocks[index]->state = 1;
                    sLocks[index]->owner = machineID;
                    kLocks[index]->lock->Acquire();
                } else {
                    sLocks[index]->queue->Append((void*)machineID);
                }
                postOffice->Send(outPktHdr,outMailHdr,oSuccess);
            } else {
                printf("ERROR: Cannot acquire, index exceeds bounds.\n");
            }
            postOffice->Send(outPktHdr,outMailHdr,oFailure);
        }

    //Release a lock
        if (key == "RL"){
            stringstream ss1;
            bool success;
            bool flag = false;
            int addr;
            int machineID;
            int index;
            char* output = new char[1];
            printf("Release Lock syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr >> machineID;
            if (index >= 0 && index < MAX_LOCKS){ //checks if index is valid
                if (kLocks[index]->lock != NULL){ //checks if the lock exists
                    kLocks[index]->lock->Release();
                    if (kLocks[index]->lock->getFree() && kLocks[index]->isToBeDestroyed){
                        kLocks[index]->requestThreads--;
                        if (kLocks[index]->requestThreads == 0){
                            delete kLocks[index]->lock;
                            delete kLocks[index];
                            delete sLocks[index];
                            kLocks[index] = NULL;
                            sLocks[index] = NULL;
                        }
                        success = postOffice->Send(outPktHdr,outMailHdr,oSuccess);
                    }
                } else {
                    printf("ERROR: No lock exists at this index.\n");
                    postOffice->Send(outPktHdr,outMailHdr,oFailure);
                } 
            } else {
                printf("ERROR: Cannot release, index exceeds bounds.\n");
                postOffice->Send(outPktHdr,outMailHdr,oFailure);
            }
        }               



        if (key == "CC"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* name = new char[MaxMailSize];
            int addr;
            int machineID;
            char* output = new char[3];
            printf("Create Lock syscall\n");
            ss1 << buffer;
            ss1 >> name >> addr >> machineID;

/*            checking to see if the lock has already been made from another program*/
            for (int i = 0; i < nextCVIndex; i++){
                printf("Comparing %s to %s \n", kCV[i]->name,name);
                if(strcmp(kCV[i]->name,name)==0){
                    printf("Names are identical\n");
                    kCV[i]->requestThreads++;
                    sprintf(output,"%d",i);
                    success = postOffice->Send(outPktHdr,outMailHdr,output);
                    flag = true;
                    break;
                }
            }
            if (!flag){
                KernelCV* tempCV = new KernelCV;
                tempCV->name = name;
                tempCV->as = (AddrSpace*)addr;
                tempCV->isToBeDestroyed = false;
                tempCV->condition = new Condition(name);
                if (nextCVIndex < 500){
                    kCV[nextCVIndex] = tempCV;
                    kCV[nextCVIndex]->requestThreads++;
                    sCV[nextCVIndex] = new ServerCV();

                    nextCVIndex++;
                    sprintf(output, "%d",nextCVIndex-1);
                    success = postOffice->Send(outPktHdr, outMailHdr, output); 

                } else {
                    printf("ERROR: Maximum number of CVs reached. Current number of CVs is %d. \n", nextLockIndex);
                    success = postOffice->Send(outPktHdr,outMailHdr,oFailure);
                    break;
                }
            }
            flag = false;
        }
        if (key == "DC"){
            stringstream ss1;
            bool success;
            bool flag = false;
            int addr;
            int machineID;
            int index;
            char* output = new char[1];
            printf("Destroy CV syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr >> machineID;
            if (index >= 0 && index < MAX_CVS){
                if (kCV[index]->condition == NULL){
                    printf("ERROR: No CV exists here.\n");
                    success = postOffice->Send(outPktHdr,outMailHdr,oFailure);
                } else {
                    kCV[index]->isToBeDestroyed = true;
                    if (kCV[index]->isToBeDestroyed && kCV[index]->condition->getLock() == NULL){
                        kCV[index]->requestThreads--;
                        if (kCV[index]->requestThreads == 0){
                            delete kCV[index]->condition;
                            delete kCV[index];
                            delete sCV[index];
                            kCV[index] = NULL;
                            sCV[index] = NULL;
                            printf("All processes asked for its destruction: destroyed lock.\n");                            
                        }
                    }
                    printf("Lock successfully set for destruction\n");
                    success = postOffice->Send(outPktHdr,outMailHdr,oSuccess);
                }
            } else {
                printf("ERROR: Index is out of bounds.\n");
                success = postOffice->Send(outPktHdr,outMailHdr,oFailure);
            }

        }

        if (key == "WC"){
            stringstream ss1;
            bool success;
            int addr;
            int machineID;
            int index;
            int lockIndex;
            char* output = new char[3];
            int machineID = inMailHdr.from;
            printf("Wait CV syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr >> machineID;
            if (index >= 0 && index < MAX_LOCKS){
                KernelLock* cvLock = kLocks[lockIndex];
                kCV[index]->condition->Wait(cvLock->lock);
            } else {
                printf("\nERROR: Cannot wait, index exceeds bounds.\n\n");
                sprintf(output, "%d",0);
                success = postOffice->Send(outPktHdr,outMailHdr,output);
            }
        }
        if (key == "SC"){
            stringstream ss1;
            bool success;
            bool flag = false;
            int addr;
            int machineID;
            int index;
            int lockIndex;
            int machineID = inMailHdr.from;
            printf("Signal CV syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr >> machineID;
            if (index >= 0 && index < MAX_LOCKS){
                KernelLock* cvLock = kLocks[lockIndex];
                if (cvLock->lock != NULL){
                    kCV[index]->condition->Signal(cvLock->lock);
                }
            } else {
                printf("\nERROR: Cannot signal, index exceeds bounds.\n\n");
            }
        }
        if (key == "BC"){
            stringstream ss1;
            bool success;
            bool flag = false;
            int addr;
            int machineID;
            int index;
            int lockIndex;
            int machineID = inMailHdr.from;
            printf("Broadcast CV syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr >> machineID;
            if (index >= 0 && index < MAX_LOCKS){
                KernelLock* cvLock = kLocks[lockIndex];
                kCV[index]->condition->Broadcast(cvLock->lock);
            } else {
                printf("\nERROR: Cannot broadcast, index exceeds bounds.\n\n");
            }

        }
        if (key == "CM"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* name = new char[MaxMailSize];
            int addr;
            int machineID;
            char* output = new char[3];
            printf("Create Lock syscall\n");
            int arrayLen;
            ss1 << buffer;
            ss1 >> name >> arrayLen >> addr >> machineID;

/*checking to see if the MV has already been made from another program*/
            for (int i = 0; i < nextMVIndex; i++){
                printf("Comparing %s to %s \n", MVArray[i]->name,name);
                if(strcmp(MVArray[i]->name,name)==0){
                    printf("Names are identical\n");
                    MVArray[i]->requestThreads++;
                    sprintf(output,"%d",i);
                    success = postOffice->Send(outPktHdr,outMailHdr,output);
                    flag = true;
                    break;
                }
            }
            if (!flag){
                KernelMV* tempMV = new KernelMV;
                tempMV->name = name;
                tempMV->maxValue = arrayLen;
                tempMV->values = new int[arrayLen];
                tempMV->as = (AddrSpace*)addr;
                tempMV->isToBeDestroyed = false;
                if (nextMVIndex < 500){
                    MVArray[nextMVIndex] = tempMV;
                    MVArray[nextMVIndex]->requestThreads++;
                    nextMVIndex++;
                    sprintf(output, "%d",nextMVIndex-1);
                    printf("Sending success message\n");
                    success = postOffice->Send(outPktHdr, outMailHdr, output); 

                } else {
                    printf("ERROR: Maximum number of MVs reached. Current number of MVs is %d. \n", nextMVIndex);
                    sprintf(output,"%d",-1);
                    success = postOffice->Send(outPktHdr,outMailHdr,output);
                    break;
                }
            }
            flag = false;

        }

        if (key == "DM"){
            stringstream ss1;
            bool success;
            bool flag = false;
            int addr;
            int machineID;
            int index;
            char* output = new char[1];
            printf("Destroy CV syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr >> machineID;
            if (index >= 0 && index < 500){
                if (MVArray[index]->values == NULL){
                    printf("ERROR: No MV exists here.\n");
                    sprintf(output, "%d",0);
                    success = postOffice->Send(outPktHdr,outMailHdr,output);
                } else {
                    MVArray[index]->isToBeDestroyed = true;
                    MVArray[index]->requestThreads--;
                    if (MVArray[index]->requestThreads == 0){
                        delete MVArray[index];
                        MVArray[index] = NULL;
                        printf("All processes asked for its destruction: destroyed lock.\n");                            
                    }
                    printf("MV successfully set for destruction\n");
                    sprintf(output, "%d",1);
                    success = postOffice->Send(outPktHdr,outMailHdr,output);
                }
            } else {
                printf("ERROR: Index is out of bounds.\n");
                sprintf(output, "%d",0);
                success = postOffice->Send(outPktHdr,outMailHdr,output);
            }


        }
        if (key == "GM"){
            stringstream ss1;
            bool success;
            bool flag = false;
            int arrayIndex;
            int addr;
            int machineID;
            int index;
            int outputInt;
            char* output = new char[5];
            printf("Get MV syscall\n");
            ss1 << buffer;
            ss1 >> index >> arrayIndex >> addr >> machineID;
            if (index >= 0 && index < 500){
                if (MVArray[index]->values != NULL){
                    outputInt = MVArray[index]->values[arrayIndex];
                    sprintf(output, "%d", outputInt);
                    postOffice->Send(outPktHdr,outMailHdr,output);
                } else {
                    sprintf(output, "%d", 0);
                    postOffice->Send(outPktHdr,outMailHdr,output);
                }
            } else {
                sprintf(output, "%d", 0);
                postOffice->Send(outPktHdr,outMailHdr,output);
            }

        }
        if (key == "SM"){
            stringstream ss1;
            bool success;
            bool flag = false;
            int newValue;
            int addr;
            int machineID;
            int index;
            int arrayIndex;
            char* output;
            printf("Set MV syscall\n");
            ss1 << buffer;
            ss1 >> index >> arrayIndex >> newValue >> addr >> machineID;
            if(index >= 0 && index < 500){
                MVArray[index]->values[arrayIndex] = newValue;
                postOffice->Send(outPktHdr,outMailHdr,output);
            } else {
                printf("ERROR: Index is out of bounds.\n");
                sprintf(output, "%d", 0);
                postOffice->Send(outPktHdr,outMailHdr,output);
            }

        }
    }
}

