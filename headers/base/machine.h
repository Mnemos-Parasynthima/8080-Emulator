#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <stdint.h>

#include "mem.h"
#include "instr.h"

typedef struct proc {
	uint8_t gpr[6];
	uint8_t alureg[3];
	uint8_t tempreg[2];
	uint16_t PC;
	uint16_t SP;
	uint8_t eflags;



	stat_t status;
} proc_t;


typedef struct machine {
	char* name;
	proc_t* proc;
	mem_t* mem;
} machine_t;


void initMachine();


#endif