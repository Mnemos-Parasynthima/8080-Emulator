#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include "preprocessor.h"
#include "parser.h"
#include "ResourceTable.h"

ResourceTable* rsrcTable;


int main(int argc, char const* argv[]) {
	char* infile = NULL;
	char* outbin = "a.out";
	int opt;

	while ((opt = getopt(argc, (char* const*) argv, "o:")) != -1) {
		switch (opt) {
			case 'o':
				outbin = optarg;
				break;
			case '?':
				if (optopt == 'o') fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint(optopt)) fprintf(stderr, "Unknown option '-%c'.\n", optopt);
		}
	}

	const char* allowedExts[] = { ".s", ".as", ".asm" };

	for (int i = optind; i < argc; i++) {
		// printf("Non-option argument %s\n", argv[i]);

		// Check that the input file ends in .s, .as, or .asm

		const char* dot = strrchr(argv[i], '.');
		if (!dot || dot == argv[i] || *(dot - 1) == '/') {
			continue;
		}

		for (int j = 0; j < 3; j++) {
			if (strcmp(dot, allowedExts[j]) != 0) continue;

			infile = (char*) argv[i];
			break;
		}
	}

	if (!infile) {
		fprintf(stderr, "No source file to assemble given.\n");
		exit(-1);
	}

	printf("Using %s as input file as %s as output file\n", infile, outbin);

	// All assembly code is in ../asm/ so instead of the user needing to put the path
	// just make it include automatically

	char* file = (char*) malloc(8 + strlen(infile));
	if (!file) handleError(ERR_MEM, FATAL, "Could not allocate space for file name!\n");

	sprintf(file, "../asm/%s", infile);

	FILE* source = fopen(file, "r");

	int size = 0;
	char** sourceLines = preprocess(source, &size);

	parse(sourceLines, size);


	deleteTable(rsrcTable);
	fclose(source);
	for (int i = 0; i < size; i++) {
		free(sourceLines[i]);
	}
	free(sourceLines);

	return 0;
}