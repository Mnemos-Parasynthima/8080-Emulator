#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <stdint.h>

#include "mem.h"
#include "instr.h"
#include "instr-stages.h"

// Used to refer to a specific ALU register
typedef enum AluRegs {
	ACC, // The accumulator (register A)
	ACC_LATCH, // The temporary accumulator (feeds in to ALU)
	TEMP // Temporary register (feeds in to ALU)
} aluregs;

// Grouping of the different buses
typedef struct bus {
	uint16_t addrbus; // Address bus
	uint8_t databus; // Data bus, incoming or outgoing data
	uint8_t ctrlbus; // Control bus - b0: inta; b1: memr; b2: memw
} bus_t;


typedef struct proc {
	uint8_t gpr[6]; // General purpose registers
	uint8_t alureg[3]; // Registers used by the ALU
	uint8_t tempreg[2]; // Registers W (0) and Z (1)
	uint8_t IR; // Instruction register, holds the instruction
	uint16_t PC;
	uint16_t SP;
	uint8_t eflags;

	bus_t bus; // The buses

	state_t state; // The state of the processor

	stat_t status; // The status
} proc_t;


#define Bus (guest.proc->bus)
#define AddrBus (guest.proc->bus.addrbus)
#define DataBus (guest.proc->bus.databus)
#define CtrlBus (guest.proc->bus.ctrlbus)

#define State (guest.proc->state)


typedef struct machine {
	char* name;
	proc_t* proc;
	mem_t* mem;
} machine_t;


void initMachine();


#endif