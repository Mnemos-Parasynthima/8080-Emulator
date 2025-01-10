#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "aef-loadrun.h"
#include "mem.h"
#include "machine.h"
#include "aef.h"
#include "Error.h"

extern machine_t guest;

static bool isAEF(aef_hdr* header) {
	uint8_t aefMagic[4] = { AEF_MAGIC0, AEF_MAGIC1, AEF_MAGIC2, AEF_MAGIC3 };

	for (int i = AI_MAGIC0; i <= AI_MAGIC3; i++) {
		if (header->ident[i] != aefMagic[i]) return false;
	}

	return true;
}

uint16_t loadAEF(const char* filename) {
	printf("Loading AEF executable\n");

	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
		perror(NULL);
		exit(-1);
	}

	struct stat statbuff;
	int rc = fstat(fd, &statbuff);
	if (rc != 0) {
		perror(NULL);
		exit(-1);
	}

	void* ptr = mmap(0, statbuff.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (ptr == MAP_FAILED) {
		perror(NULL);
		exit(-1);
	}

	aef_hdr* header = (aef_hdr*) ptr;
	if (!isAEF(header)) {
		handleError(ERR_AEF, FATAL, "File is not AEF executable!\n");
	}
	u_int16_t entry = header->entry;

	uint8_t* data = ((uint8_t*) header) + 10;
	// Size is at offset 10 from beginning
	uint16_t size = *data;
	// The beginning of the program
	data += 2;

	for (int i = 0; i < size; i++) {
		uint8_t byte = data[i];
		// printf("Loading byte 0x%x at 0x%x\n", byte, i);
		guest.mem->ram[i] = byte;
	}

	return entry;
}

int runAEF(const uint16_t entry) {
	printf("Running AEF executable\n");
	
	guest.proc->PC = entry;
	guest.proc->SP = guest.mem->segStart[STACK_SEG] + STACK_SIZE;
	
	// printf("PC: 0x%x\n", guest.proc->PC);

	State.ctrSigs.DBIN = false;
	State.ctrSigs.HLDA = false;
	State.ctrSigs.INTE = false;
	State.ctrSigs.WAIT = false;
	State.ctrSigs._WR = false;

	// State.statusSigs.

	do {
		fetch();

		// printf("Fetched instruction: 0x%x\n", guest.proc->IR);

		// decode();
		// execute();
		break;
	} while (!State.ctrSigs.WAIT);
		
	return 0;
}