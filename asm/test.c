#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef char byte;

int main(int argc, char const* argv[]) {
	uint8_t expectedHeader[8] = {
		0xae, 'A', 'E', 'F', 0x00, 0x00, 0x00, 0x00
	};
	uint8_t actualHeader[8] = {};

	uint16_t expectedEntry = 0x0;
	uint16_t actualEntry;

	uint16_t expectedSize = 12;
	uint16_t actualSize;

	uint8_t expectedInstr[12] = {
		0b00111110, 0b00000001,
		0b00000110, 0b00001011,
		0b00001110, 0b00000101,
		0b00010110, 0b00000010,
		0b00100110, 0b00000000,
		0b10000000,
		0b01110110
	};
	uint8_t actualInstr[12] = {};

	FILE* bin = fopen("add", "rb");

	fread(actualHeader, sizeof(uint8_t), 8, bin);
	fread(&actualEntry, sizeof(uint16_t), 1, bin);
	fread(&actualSize, sizeof(uint16_t), 1, bin);
	fread(actualInstr, sizeof(uint8_t), 12, bin);

	for (int i = 0; i < 8; i++) {
		if (actualHeader[i] != expectedHeader[i]) {
			printf("Actual and expected header differ between actual %x and expected %x from %d!\n", 
					actualHeader[i], expectedHeader[i], i);
			exit(1);
		}
	}

	if (actualEntry != expectedEntry) {
		printf("Actual and expected entries differ between actual %d and expected %d!\n", actualEntry, expectedEntry);
		exit(1);
	}

	if (actualSize != expectedSize) {
		printf("Actual and expected sizes differ between actual %d and expected %d!\n", actualSize, expectedSize);
		exit(1);
	}

	for (int i = 0; i < 12; i++) {
		if (actualInstr[i] != expectedInstr[i]) {
			printf("Actual and expected instructions differ between actual %x and expected %x from %d!\n", 
					actualInstr[i], expectedInstr[i], i);
			exit(1);
		}
	}

	fclose(bin);

	printf("Binary add is as expected!\n");

	return 0;
}