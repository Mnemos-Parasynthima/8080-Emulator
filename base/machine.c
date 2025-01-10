#include <stdlib.h>
#include <stdio.h>

#include "machine.h"

extern machine_t guest;

static uint16_t segStarts[] = {
	0x0000, // text-data starts at 0
	0x7800, // no access starts at 0x0+30KB
	0x8800, // stack starts at 0x0+34KB
};

void initMachine() {
	guest.name = "m80";

	guest.proc = (proc_t*) malloc(sizeof(proc_t));
	
	for (int i = 0; i < 6; i++) {
		guest.proc->gpr[i] = 0x00;
	}
	for (int i = 0; i < 3; i++) {
		guest.proc->alureg[i] = 0x00;
	}
	for (int i = 0; i < 2; i++) {
		guest.proc->tempreg[i] = 0x00;
	}

	guest.proc->IR = 0x0;
	guest.proc->PC = 0x00;
	guest.proc->SP = 0x00;
	
	AddrBus = 0x00;
	DataBus = 0x0;
	CtrlBus = 0x0;

	guest.proc->eflags = PACK_EFLAGS(0,0,0,0,0);

	State.intdatabus = 0x0;

	guest.mem = (mem_t*) malloc(sizeof(mem_t));
	guest.mem->maxAddr = MAX_ADDR;
	guest.mem->wordSize = WORD_SIZE;
	for (int i = 0; i <= STACK_SEG; i++) {
		guest.mem->segStart[i] = segStarts[i];
	}

	// Add stack canary
	guest.mem->ram[guest.mem->segStart[STACK_SEG] - 1] = 0xFE;
	guest.mem->ram[guest.mem->segStart[STACK_SEG] - 2] = 0xED;
	guest.mem->ram[guest.mem->segStart[STACK_SEG] - 3] = 0xFA;
	guest.mem->ram[guest.mem->segStart[STACK_SEG] - 4] = 0xED;
}