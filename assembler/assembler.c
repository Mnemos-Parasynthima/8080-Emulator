#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "preprocessor.h"
#include "utils.h"
#include "translator-generator.h"


/**
 * Write the given binary image to a binary executable file with the given name.
 * @param outname The executable file name
 * @param binImg The binary image
 */
static void outputExec(char* outname, aef_bin_img* binImg) {
	// All assembled code file is to be in ../asm/

	char* file = (char*) malloc(8 + strlen(outname));
	if (!file) handleError(ERR_MEM, FATAL, "Could not allocate space for outgoing file name!\n");

	sprintf(file, "../asm/%s", outname);
	FILE* out = fopen(file, "wb");

	fwrite(&binImg->header.ident, sizeof(char), 8, out);
	fwrite(&binImg->header.entry, sizeof(uint16_t), 1, out);
	fwrite(&binImg->size, sizeof(uint16_t), 1, out);
	fwrite(&binImg->mem, sizeof(byte), binImg->size, out);

	fclose(out);

	printf("Executable %s created!\n", outname);
}

/**
 * Initializes the predefined labels for the register, setting them as unable to be redefined.
 * @param symTable The symbol table
 */
static void createPredefinedLabels(SymbolTable* symTable) {
	int len = sizeof(VALID_REGS) / sizeof(VALID_REGS[0]);

	// This will iterate through the entire array
	// According to the 8080 asm data sheet, SP and PSW are not placed as labels but it's going to be done anyway
	// In a later retrospect, placing SP and PSW as labels was a good decision, maybe
	for (int i = 0; i < len; i++) {
		size_t strLen = strlen(VALID_REGS[i]);
		char* label = (char*) malloc(strLen + 1);
		if (!label) handleError(ERR_MEM, FATAL, "Could not allocate space for predefined label!\n");

		strcpy(label, VALID_REGS[i]);
		// char* temp = label;
		// TOLOWER(temp)

		sym_entry_t* entry = initEntry(label, i, false);
		addEntry(symTable, entry);
	}
}

/**
 * Generates and fills the symbol tables based on the source objects.
 * Does any resolving if needed. That is, if a symbol uses an expression (may be just another symbol) including a symbol, 
 * if that symbol is in the table, it evaluates that expression. 
 * For example, `LAB equ 3`, `LAB2 equ LAB + 2`, `LAB` gets placed in the table, and then `LAB2` is evaluated since it knows `LAB`, resulting
 * in `entry.data` for `LAB2` to be `5`.
 * 
 * For labels that cannot be resolved due to using an expression (`LAB1	equ LAB0 + LAB2`) using unknown labels (so far), they are
 * marked as incomplete entries, which will then be needed to be evaluated then marked as complete.
 * Complete "true" resolving is done at code translation and generation. That is,
 * up until code translation and generation, the data set for the labels are not filled in elsewhere, making each `sourceObject`
 * maintain any used labels, especially for instructions.
 * @param srcObjs 
 * @return The filled symbol table
 */
static SymbolTable* generateSymTable(src_obj_list_t* srcObjs) {
	// printf("Generating symbol table for %d lines\n", srcObjs->count);

	/**
	 * Register names are technically labels, as per 8080 ASM data sheet
	 * Automatically add those labels to the symbol table as if `A equ 7`
	 * Meaning it cannot be redefined, disallowing the programmer to use register names as labels
	 * This could be checked during parsing but it is more of a symbol redefinition type of issue
	 */
	SymbolTable* symTable = initTable(5, 5);

	createPredefinedLabels(symTable);

	// Go through the entire list
	// For the lines that only have its label set, it labels the following line (address)
	// set/equ are easy enough*, just set its data to SourceObject.operands (LAB equ 1)
	// These will be labels that will be used to resolve elsewhere
	// For label instructions, it labels the address
	// For operand labels (labels is found in SourceObject.operands[]), they are to be resolved later on with the help of the sym table
	// do nothing for the table
	// For incomplete entries, they occur when they use an expression
	src_obj_t* srcObj = NULL;

	sym_entry_t* entry;
	inc_sym_entry_t* incEntry;
	for (int i = 0; i < srcObjs->count; i++) {
		srcObj = srcObjs->arr[i];

		entry = NULL;
		incEntry = NULL;

		// Skip this iteration if no label present
		if (!srcObj->label) continue;

		if (!srcObj->instr) {
			// When it is only a label, note that by proxy, srcObj.operands[0] should also be null
			// Additionally, only label lines mark an instruction, no redefinition by nature
			entry = initEntry(srcObj->label, i + 1, false);
		} else if (strcmp(srcObj->instr, VALID_PSEUDO[1]) == 0 || strcmp(srcObj->instr, VALID_PSEUDO[2]) == 0) {
			// When the label is for set or equ
			// Chances are that it might need some evaluating
			// Just do a syntax/eval tree thing
			// It is safe to do operands[0] since set and equ only take in an expression as an operand
			bool evalDone = eval(srcObj->operands[0], symTable);

			bool canRedefine;
			if (strcmp(srcObj->instr, VALID_PSEUDO[1]) == 0) canRedefine = false; // equ
			else canRedefine = true; // set

			if (evalDone) {
				// printf("Expression eval is complete\n");
				// The operand was evaluated to a single number
				// Could be from an all number expression or labels have been defined already
				// Add it to the table now
				// Not that the data is still in string format (srcObj->operands[0])
				entry = initEntry(srcObj->label, atoi(srcObj->operands[0]), canRedefine);
			} else {
				// printf("Expression eval could not be completed\n");
				// Could not eval because it references a label not yet defined
				// Mark it as incomplete
				incEntry = initIncEntry(srcObj->label, srcObj->operands[0], canRedefine);
			}		
		} else if (srcObj->instr) {
			// When the label is for the rest of instructions and directives, the label is inline
			// By nature, no redefinition
			entry = initEntry(srcObj->label, i, false);
		}

		if (entry) addEntry(symTable, entry);
		else if (incEntry) addIncEntry(symTable, incEntry);
	}

	return symTable;
}

/**
 * Evaluates any and all expressions found in set/equ or instruction operands. Uses the symbol table in case an 
 * expression refers to a complete symbol. Note that in such case where a label is set to an expression involving a complete label/symbol
 * found in the symbol table, it only evaluates it. It does not add that entry to the symbol table. Also, these incomplete labels are found
 * in `symTable.incEntries` where each incomplete entry has an `expr` field.
 * 
 * Note that the evaluation is done in place. That is, if an operand has `"3 * 2 + 1"`, then it will be changed to `"7"` with an early
 * null terminator, thus leaving extra space. The same applies for the incomplete table entries. `incEntry.expr` is evaluated, replacing the
 * result with the expression.
 * @param symTable The symbol table
 * @param srcObjs 
 */
static void evaluateExpressions(SymbolTable* symTable, src_obj_list_t* srcObjs) {
	// First go through the incomplete entries
	// Now that all complete entries are in the table, chances are that some incomplete entries
	// that use these complete entries can be eval'd

	for (int i = 0; i < symTable->incEntriesSize; i++) {
		bool evalDone = eval(symTable->incEntries[i]->expr, symTable);

		if (evalDone) {

		} else {

		}
	}

	// Now go through source objects
	// The token of importance are the operands for the normal instructions
	// Note "normal", since equ and set do use operands but their operands were taken care of in generateSymTable

	for (int i = 0; i < srcObjs->count; i++) {
		src_obj_t* srcObj = srcObjs->arr[i];

		if (!srcObj->instr) continue;
		if (strcmp(srcObj->instr, VALID_PSEUDO[1]) == 0 || strcmp(srcObj->instr, VALID_PSEUDO[1]) == 0) continue;
		// Only deal with normal instr

		char** operands = srcObj->operands;

		while (*operands) {
			bool evalDone = eval(*operands, symTable);
			if (evalDone) {

			} else {

			}

			operands++;
		}
	}	
}

/**
 * Resolves any symbols found in symTable. This is the case in which a label that once used label(s) in an expression
 * (`LAB1 equ LAB0 + LAB3`) which then was evaluated by `evaluateExpressions` leading to a number be set (`LAB1 equ 10`).
 * This would then require the incomplete entry to be converted to a complete entry, using the evaluated expression as its data.
 * Technically, since the evaluations are done in place, `incEntry.expr` holds a number (as a string) so it can be interpreted as a complete entry.
 * However, due to the semantics of `incEntry` representing an incomplete entry, it must be changed to an complete entry.
 * @param symTable The symbol table
 */
static void resolveSymbols(SymbolTable* symTable) {
	// Resolving meaning going through the incompletes and converting them to complete entries
	for (int i = 0; i < symTable->incEntriesSize; i++) {
		inc_sym_entry_t* entry = symTable->incEntries[i];

		completeAndAdd(symTable, entry);

		// The incomplete entry is deleted/freed
		// Need to null it out
		symTable->incEntries[i] = NULL;
	}

	symTable->incEntriesSize = 0;
}

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
		fprintf(stderr, "No assembly source file given.\n");
		exit(-1);
	}

	// printf("Using %s as input file and %s as output file\n", infile, outbin);

	// All assembly code is in ../asm/ so instead of the user needing to put the path
	// just make it include automatically

	char* file = (char*) malloc(8 + strlen(infile));
	if (!file) handleError(ERR_MEM, FATAL, "Could not allocate space for file name!\n");

	sprintf(file, "../asm/%s", infile);

	FILE* source = fopen(file, "r");

	int size = 0;
	printf("Will preprocess\n");
	char** sourceLines = preprocess(source, &size);
	printf("Preprocessed!\n\nWill now lexicalize\n");
	src_obj_list_t* sourceObjs = lexicalize(sourceLines, size);
	printf("Lexicalized!\n\nWill now parse and check\n");
	parseCheck(sourceObjs);
	printf("Parsed!\n\nWill now generate symbol table\n");
	SymbolTable* symTable = generateSymTable(sourceObjs);
	printf("Symbol table generated!\n\nWill now evaluate any and all expressions\n");
	evaluateExpressions(symTable, sourceObjs);
	printf("Evaluated all expressions!\n\nWill now resolve symbols\n");
	resolveSymbols(symTable);
	printf("All symbols resolved!\n\nWill now translate and generate\n");
	aef_bin_img* binImg = translateGenerate(symTable, sourceObjs);
	printf("Translated and generated!\n\nWill now write exec\n");
	outputExec(outbin, binImg);

	deleteTable(symTable);
	deleteSrcObjsList(sourceObjs);
	// deleteAEF(binImg);
	fclose(source);
	for (int i = 0; i < size; i++) {
		free(sourceLines[i]);
	}
	free(sourceLines);

	return 0;
}