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


    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data = "Hello there!";
    char* buffer;
    string key;
    string outputString;

    while(1){

/*    construct packet, mail header for original message*/
/*    To: destination machine, mailbox 1*/
/*    From: our machine, reply to: mailbox 0*/
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
        printf("%s \n", buffer);

/*    Create Lock request - assumes that the lock name has no spaces*/
        if (key == "CL"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* name = new char[MaxMailSize];
            char* addr = new char[MaxMailSize];
            char* output = new char[3];
            printf("Create Lock syscall\n");
            ss1 << buffer;
            ss1 >> name >> addr;

            output[0] = 'C';
            output[1] = 'L';
            postOffice->Send(outPktHdr,outMailHdr,output);
/*            checking to see if the lock has already been made from another program*/
            // for (int i = 0; i < nextLockIndex; i++){
            //     printf("Comparing %s to %s \n", kLocks[i]->name,name);
            //     if(strcmp(kLocks[i]->name,name)==0){
            //         printf("Names are identical\n");
            //         kLocks[i]->requestThreads++;
            //         sprintf(output,"%d",i);
            //         success = postOffice->Send(outPktHdr,outMailHdr,output);
            //         flag = true;
            //         break;
            //     }
            // }
            // if (!flag){
            //     KernelLock* tempLock = new KernelLock;
            //     tempLock->name = name;
            //     tempLock->as = (AddrSpace*)addr;
            //     tempLock->isToBeDestroyed = false;
            //     tempLock->lock = new Lock(name);
            //     if (nextLockIndex < 500){
            //         kLocks[nextLockIndex] = tempLock;
            //         kLocks[nextLockIndex]->requestThreads++;
            //         sLocks[nextLockIndex] = new ServerLock();

            //         nextLockIndex++;
            //         sprintf(output, "%d",nextLockIndex-1);
            //         printf("Sending success message");
            //         success = postOffice->Send(outPktHdr, outMailHdr, output); 

            //     } else {
            //         printf("ERROR: Maximum number of locks reached. Current number of locks is %d. \n", nextLockIndex);
            //         sprintf(output,"%d",-1);
            //         success = postOffice->Send(outPktHdr,outMailHdr,output);
            //         break;
            //     }
//            }
//            flag = false;
        }

/*        Destroy lock syscall*/
        if (key == "DL"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* addr = new char[MaxMailSize];
            int index;
            printf("Destroy Lock syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr;
        }

/* Acquire lock syscall*/
        if (key == "AL"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* addr = new char[MaxMailSize];
            int index;
            int machineID = inMailHdr.from;
            printf("Acquire Lock syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr;
        }

    //Release a lock
        if (key == "RL"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* addr = new char[MaxMailSize];
            int index;
            printf("Release Lock syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr;

        }


        if (key == "CC"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* name = new char[MaxMailSize];
            char* addr = new char[MaxMailSize];
            char* output = new char[3];
            printf("Create Lock syscall\n");
            ss1 << buffer;
            ss1 >> name >> addr;

        }
        if (key == "DC"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* addr = new char[MaxMailSize];
            int index;
            printf("Destroy CV syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr;

        }

        if (key == "WC"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* addr = new char[MaxMailSize];
            int index;
            int lockIndex;
            int machineID = inMailHdr.from;
            printf("Wait CV syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr;

        }
        if (key == "SC"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* addr = new char[MaxMailSize];
            int index;
            int lockIndex;
            int machineID = inMailHdr.from;
            printf("Signal CV syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr;

        }
        if (key == "BC"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* addr = new char[MaxMailSize];
            int index;
            int lockIndex;
            int machineID = inMailHdr.from;
            printf("Broadcast CV syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr;

        }
        if (key == "CM"){
            stringstream ss1;
            bool success;
            bool flag = false;
            int arrayLen;
            char* name = new char[MaxMailSize];
            char* addr = new char[MaxMailSize];
            char* output = new char[3];
            printf("Create MV syscall\n");
            ss1 << buffer;
            ss1 >> name >> arrayLen >> addr;

        }

        if (key == "DM"){
            stringstream ss1;
            bool success;
            bool flag = false;
            char* addr = new char[MaxMailSize];
            int index;
            printf("Destroy MV syscall\n");
            ss1 << buffer;
            ss1 >> index >> addr;


        }
        if (key == "GM"){
            stringstream ss1;
            bool success;
            bool flag = false;
            int arrayIndex;
            char* addr = new char[MaxMailSize];
            int index;
            printf("Get MV syscall\n");
            ss1 << buffer;
            ss1 >> index >> arrayIndex >> addr;

        }
        if (key == "SM"){
            stringstream ss1;
            bool success;
            bool flag = false;
            int newValue;
            char* addr = new char[MaxMailSize];
            int index;
            printf("Set MV syscall\n");
            ss1 << buffer;
            ss1 >> index >> newValue >> addr;

        }
    }
}

