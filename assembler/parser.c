#include <stdlib.h>
#include <stdio.h>
#include <elf.h>

#include "parser.h"
#include "Error.h"
#include "ResourceTable.h"


extern ResourceTable* rsrcTable;

const char* VALID_INS[] = {
	"inr", "dcr", "cma", "nop",
	"mov", "stax", "ldax",
	"add", "adc", "sub", "ana", "xra",
	"ora", "cmp", // rotate instructions ignored
	"push", "pop",
	"dad", "inx", "dcx", "xchg", "xthl", "sphl",
	"mvi", "adi", "aci", "sui", "sbi", "ani", "xri",
	"ori", "cpi",
	"sta", "lda", "shld", "lhld",
	"pchl", "jmp", "jc", "jnc", "jz", "jnz", "jm",
	"jp", "jpe", "jpo",
	"call", "cc", "cnc", "cz", "cnz", "cm",
	"cp", "cpe", "cpo",
	"ret", "rc", "rnc", "rz", "rnz", "rm",
	"rp", "rpe", "rpo", "rst",
	"ei", "di", "hlt"
};

const char* VALID_PSEUDO[] = {
	"org", "equ", "set", "end", "if", "endif"
};

const char* VALID_DIRCTVS[] = {
	"db", "dw", "ds"
};

void parse(char** source, int size) {
	rsrcTable = initTable(5);

	for (int i = 0; i < size; i++) {
		printf("Parsing %s\n", source[i]);

		char* tok = strtok(source[i], " ");
	}
}