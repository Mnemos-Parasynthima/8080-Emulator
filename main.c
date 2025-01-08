#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>



int main(int argc, char const* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: emu filename\n");
		exit(-1);
	}

	char* filename = argv[1];

	initMachine();
	initITable();

	uint16_t entry = loadAEF(filename);
	int ret = runAEF(entry);

	return ret;
}