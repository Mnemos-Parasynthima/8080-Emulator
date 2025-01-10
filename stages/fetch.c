#include <stdlib.h>
#include <stdio.h>

#include "instr-stages.h"
#include "machine.h"
#include "hardware.h"

extern machine_t guest;

void fetch() {
	State.statusSigs._WO = true;
	State.statusSigs.M1 = true;
	State.statusSigs.MEMR = true;

	State.ctrSigs._WR = true;

	// T1
	AddrBus = guest.proc->PC;
	sendStatusToData();

	// printf("AddrBus: 0x%x; DataBus: 0x%x\n", AddrBus, DataBus);

	// T2
	State.ctrSigs.DBIN = true;
	latchStatus();

	// printf("CtrlBus: 0x%x\n", CtrlBus);

	if (State.statusSigs.HLTA) {
		// Enter halt state
		// For now, it means subsequent functions are not ran
		State.ctrSigs.INTE = true;
		State.ctrSigs.WAIT = true;
		return;
	}

	// Processor entering TW state
	mem();

	// T3
	State.intdatabus = DataBus;
	guest.proc->IR = State.intdatabus;
}