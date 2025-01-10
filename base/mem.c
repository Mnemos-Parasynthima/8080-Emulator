#include <stdlib.h>
#include <stdio.h>

#include "mem.h"
#include "machine.h"

extern machine_t guest;

void memRead() {
	// printf("Reading memory at 0x%x\n", AddrBus);
	DataBus = guest.mem->ram[AddrBus];
}

void memWrite() {

}