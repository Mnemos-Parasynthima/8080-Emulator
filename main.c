#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "machine.h"
#include "aef-loadrun.h"


machine_t guest;


int main(int argc, char const* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: emu filename\n");
		exit(-1);
	}

	// Add assembly files are in asm/
	// Append it

	size_t len = strlen(argv[1]);
	char* filename = (char*) malloc(sizeof(char) * (len + 4 + 1));
	sprintf(filename, "asm/%s", argv[1]);

	initMachine();

	printf("Welcome to %s, ", guest.name);
	printf("8080 Intel Processor\n");
	printf("Loaded up with 64KB RAM\n");

	uint16_t entry = loadAEF(filename);
	int ret = runAEF(entry);

	return ret;
}