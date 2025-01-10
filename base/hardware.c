#include <stdint.h>

#include "hardware.h"
#include "machine.h"
#include "mem.h"

extern machine_t guest;

void regarray(bool wr, uint8_t src, uint8_t dst) {
	if (wr) {
		guest.proc->gpr[dst] = guest.proc->bus.databus;
	} else guest.proc->bus.databus = guest.proc->gpr[src];
}

void alu(alu_op_t aluop, bool seteflags) {
	uint8_t	res = 0;

	uint8_t a = guest.proc->alureg[ACC_LATCH];
	uint8_t b = guest.proc->alureg[TEMP];

	switch (aluop)	{
		case PLUS_OP:
			res = a + b;
			break;
		case MINUS_OP:
			res = a - b;
		case OR_OP:
			res = a | b;
		case XOR_OP:
			res = a ^ b;
		case AND_OP:
			res = a & b;
		default:
			break;
	}

	guest.proc->bus.databus = res;

	if (seteflags) {

	}
}

void sendStatusToData() {
	uint8_t d0 = State.statusSigs.INTA;
	uint8_t d1 = State.statusSigs._WO;
	uint8_t d2 = State.statusSigs.STACK;
	uint8_t d3 = State.statusSigs.HLTA;
	uint8_t d4 = State.statusSigs.OUT;
	uint8_t d5 = State.statusSigs.M1;
	uint8_t d6 = State.statusSigs.INP;
	uint8_t d7 = State.statusSigs.MEMR;

	uint8_t statusWord = ((d7<<7)|(d6<<6)|(d5<<5)|(d4<<4)|(d3<<3)|(d2<<2)|(d1<<1)|(d0<<0));

	Bus.databus = statusWord;
}

void latchStatus() {
	static uint8_t statusLatch = 0x0;

	statusLatch = DataBus;
	// printf("Status latch: 0x%x\n", statusLatch);
	DataBus = 0x0;

	bool INTA = (statusLatch>>0) & 0x1;
	bool _WO = (statusLatch>>1) & 0x1;
	bool STACK = (statusLatch>>2) & 0x1;
	bool HLTA = (statusLatch>>3) & 0x1;
	bool OUT = (statusLatch>>4) & 0x1;
	bool M1 = (statusLatch>>5) & 0x1;
	bool INP = (statusLatch>>6) & 0x1;
	bool MEMR = (statusLatch>>7) & 0x1;


	// Set INTA
	if (INTA && State.ctrSigs.DBIN) {
		// printf("If INTA and DBIN --> INTA\n");
		Bus.ctrlbus |= (1<<0);
	}
	if (MEMR && State.ctrSigs.DBIN) {
		// printf("If MEMR && DBIN --> MEMR\n");
		Bus.ctrlbus |= (1<<1);
	}
	if (!OUT && !State.ctrSigs._WR) {
		// printf("If ~OUT && ~_WR --> MEMW\n");
		Bus.ctrlbus |= (1<<2);
	}
}

void mem() {
	State.ctrSigs.WAIT = true;

	// MEMR
	if ((((Bus.ctrlbus >> 1) & 0x1) == 0x1) && State.ctrSigs.DBIN) memRead();
	if (((Bus.ctrlbus >> 0) & 0x1) == 0x1) memWrite();


	State.ctrSigs.WAIT = false;
}