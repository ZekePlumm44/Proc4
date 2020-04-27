/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"
#include "filesys.h"
#include "directory.h"
#include "filesys.h"


void SysHalt()
{
  kernel->interrupt->Halt();
}

void Start() {
	kernel->currentThread->space->Execute();
}
int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

SpaceId SysExec(char *name) {
	Thread *thread = new Thread(name);
	AddrSpace *addr = new AddrSpace();
	SpaceId id = (SpaceId) addr;
	thread->space = addr;
	if(thread->space->Load(name)) {
		thread->Fork((VoidFunctionPtr) Start, 0);
	}
	return id;
}
void SysExit() {
	kernel->currentThread->Finish();
}
int SysWrite(char *buffer, int size, OpenFileId id) {
	//Get OpenFile from OpenFileID
	OpenFile* File = (OpenFile*) id;
	//Writes buffer to File and returns the number of bytes written
	int bytesWrit = File->Write(buffer,size);
	return bytesWrit;
}
OpenFileId SysOpen(char *name) {
	int id;
	printf("Gona open now\n");
	printf("%s\n", name);
	id = OpenForReadWrite(name,FALSE);	
	if(id < 0) {
		id = OpenForWrite(name);
		Close(id);
	}
	return id;
}

#endif /* ! __USERPROG_KSYSCALL_H__ */
