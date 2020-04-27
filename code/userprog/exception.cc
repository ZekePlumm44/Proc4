// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    //declare variables
    int type = kernel->machine->ReadRegister(2);
    SpaceId idExec;
    char * fileName;
    char * buffer;
    int * location;
    int size;
    int bytesWritten;
    OpenFileId id;
    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
    case SyscallException:
      switch(type) {
      case SC_Halt:
	DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

	SysHalt();
	
	ASSERTNOTREACHED();
	break;
      case SC_Exit:
	//Calls helper to exit thread
	//If somethings wrong tells debug
	DEBUG(dbgSys, "Exitng cur thread\n");
	if(kernel->machine->ReadRegister(4) != 0) 
		DEBUG(dbgSys, "Something went wrong...\n");
	SysExit();

        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
        }
	return;
	ASSERTNOTREACHED();
	break;
      case SC_Exec:
	DEBUG(dbgSys, "Creating new thread for file\n");
	kernel->machine->ReadMem(kernel->machine->ReadRegister(4),1,location);
	fileName = (char*)location;
	id = SysExec(fileName);
	kernel->machine->WriteRegister(2, id);

        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
        }
	return;
	ASSERTNOTREACHED();
	break;
//      case SC_Join:
      case SC_Open:
	//Grabs file name from reg 4 and creates variables
	kernel->machine->ReadMem(kernel->machine->ReadRegister(4),1,location);
	fileName = (char*)location;
	DEBUG(dbgSys, "Opening file " << fileName << "\n");
	id = SysOpen(fileName);
	kernel->machine->WriteRegister(2, (int)id);

        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
        }
	return;
	ASSERTNOTREACHED();
	break;
      case SC_Write:
	size = kernel->machine->ReadRegister(5);
	buffer = new char[size];
        DEBUG(dbgSys, "Writing " << size << " bytes to ");
	for(int iterW = 0; iterW < size;iterW++) {
		//Iterates across the size of the memory location repeadly to read buffer; 
		kernel->machine->ReadMem((kernel->machine->ReadRegister(4)+iterW),1,location);
		buffer[iterW] = (char)location[0];
	}
        if(kernel->machine->ReadRegister(6)== 1) {
		DEBUG(dbgSys, "terminal\n");
		//If OpenFileID param = ConsoleOutput prints to terminal
		for(int writeIter = 0; writeIter < size; writeIter++) {
			printf("%c", buffer[writeIter]);
		}
		//In this case returns full size as everything is written
		bytesWritten = size;
	}
	else {
		//Uses helper method to write buffer to File and returns bytes written
		DEBUG(dbgSys, "file with id " << kernel->machine->ReadRegister(6));
		bytesWritten = SysWrite(buffer,size,kernel->machine->ReadRegister(6));
	}
	kernel->machine->WriteRegister(2, bytesWritten);

        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
        }
	return;
        ASSERTNOTREACHED();
        break;
      case SC_Close:
	//Checks to see if the open file matches id given
	//Closes if yes returns error if not
	DEBUG(dbgSys,"Closing file id " << kernel->machine->ReadRegister(4) << "\n");
	id = Close(kernel->machine->ReadRegister(4));
	

	kernel->machine->WriteRegister(2, id);

        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
        }
	return;
	ASSERTNOTREACHED();
	break;
      case SC_Add:
	DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
	
	/* Process SysAdd Systemcall*/
	int result;
	result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
			/* int op2 */(int)kernel->machine->ReadRegister(5));

	DEBUG(dbgSys, "Add returning with " << result << "\n");
	/* Prepare Result */
	kernel->machine->WriteRegister(2, (int)result);
	
	/* Modify return point */
	{
	  /* set previous programm counter (debugging only)*/
	  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  /* set next programm counter for brach execution */
	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
	}

	return;
	
	ASSERTNOTREACHED();

	break;
      default:
	cerr << "Unexpected system call " << type << "\n";
	break;
      }
      break;
}}
//	"Halt".
//
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
