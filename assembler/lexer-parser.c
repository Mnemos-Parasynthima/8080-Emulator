#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <strings.h>

#include <stdio.h>

#include "lexer.h"
#include "parser.h"
#include "utils.h"
#include "Error.h"


const char* VALID_INS[65] = {
	"inr", "dcr", "cma", "nop",
	"mov", "stax", "ldax",
	"add", "adc", "sub", "ana", "xra",
	"ora", "cmp", // rotate instructions ignored
	"push", "pop",
	"dad", "inx", "dcx", "xchg", "xthl", "sphl",
	"mvi", "adi", "aci", "sui", "sbi", "ani", "xri",
	"ori", "cpi",
	"sta", "lda", "shld", "lhld",
	"pchl", 
	"jmp", "jc", "jnc", "jz", "jnz", "jm", "jp", "jpe", "jpo",
	"call", "cc", "cnc", "cz", "cnz", "cm", "cp", "cpe", "cpo",
	"ret", "rc", "rnc", "rz", "rnz", "rm", "rp", "rpe", "rpo", 
	"rst",
	// "ei", "di", 
	"hlt"
};

const char* VALID_PSEUDO[4] = {
	"org", "equ", "set", "end"
};

const char* VALID_DIRCTVS[3] = {
	"db", "dw", "ds"
};

const char* VALID_REGS[10] = {
	"b", "c", "d", "e", "h", "l", "m", "a", "sp", "psw" // note that "m" technically is a register, using h and l
};


static src_obj_list_t* initSrcObjsList(int size) {
	src_obj_list_t* srcObjs = (src_obj_list_t*) malloc(sizeof(src_obj_list_t));
	if (!srcObjs) handleError(ERR_MEM, FATAL, "Could not allocate space for source objects list!\n");

	srcObjs->arr = (src_obj_t**) calloc(size, sizeof(src_obj_t*));
	if (!srcObjs->arr) handleError(ERR_MEM, FATAL, "Could not allocate space for source objects list array!\n");

	srcObjs->cap = size;
	srcObjs->count = 0;

	return srcObjs;
}

/**
 * Creates and initializes a source object with the given label, instruction, and operands array.
 * Proper combinations are: 
 * - only label
 * - label and instruction
 * - label, instruction, operands
 * - instruction and operands
 * 
 * Also note that operands must be a null-terminated array of strings.
 * The strings/array are shallowed copied. Freeing the source in which the given data originates
 * affects the contents. Thus, data freeing should only happen in the delete source object function.
 * @param label The label, if any
 * @param instr The instruction, if any
 * @param operands Null-terminated array of operands, if any
 * @return Source object
 */
static src_obj_t* initSrcObj(char* label, char* instr, char** operands) {
	src_obj_t* srcObj = (src_obj_t*) malloc(sizeof(src_obj_t));
	if (!srcObj) handleError(ERR_MEM, FATAL, "Could not allocate space for source object!\n");

	srcObj->label = label;
	srcObj->instr = instr;
	srcObj->operands = operands;

	return srcObj;
}

static void deleteSrcObj(src_obj_t* srcObj) {
	free(srcObj->label);
	free(srcObj->instr);

	// The operands array MUST be null terminated
	while (*srcObj->operands) {
		free(*srcObj->operands);
		*(srcObj->operands) = NULL;
		srcObj->operands++;
	}

	free(srcObj);
}

void addSrcObj(src_obj_list_t* srcObjList, src_obj_t* srcObj) {
	if (srcObjList->count == srcObjList->cap) {
		src_obj_t** temp = realloc(srcObjList->arr, (srcObjList->cap * 2) * sizeof(src_obj_t*));
		if (!temp) handleError(ERR_MEM, FATAL, "Could not reallocate source object list array!\n");

		srcObjList->arr = temp;
		srcObjList->cap *= 2;
	}

	int emptyIdx = srcObjList->count;
	if (srcObjList->arr[emptyIdx] == NULL) {
		srcObjList->arr[emptyIdx] = srcObj;
		srcObjList->count++;
		return;
	}
}

void deleteSrcObjsList(src_obj_list_t* srcObjs) {
	if (!srcObjs) return;

	for (int i = 0; i < srcObjs->count; i++) {
		deleteSrcObj(srcObjs->arr[i]);
	}
	free(srcObjs->arr);
	free(srcObjs);
}


/**
 * Checks that the given character is a valid character to start with.
 * @param c Character to check
 * @return True if valid starting character, false otherwise
 */
static bool validChar(char c) {
	return c == '?' || c == '@' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/**
 * Trims the given label to at most 5 characters, excluding ':' if it is used.
 * At the point in which this function is called, the absence or not of ':' is confirmed to be valid.
 * The trimming is simply placing the null character earlier after the relocated ':'.
 * @param label The label to trim
 */
static void trim(char* label) {
	// len might or might not include ':'
	size_t len = strlen(label);

	if (*(label + len - 1) == ':' && len-1 > 5) {
		// for normal labels, len includes ':'
		*(label + 5) = ':';
		*(label + 6) = '\0';
	} else if (len - 1 > 5) {
		// for directive labels, len does not include ':'
		*(label + 5) = '\0';
	} // do nothing if the label is within length bounds (1<=len<=5)

	// Make sure it is max 6 (may include ':')
	assert(strlen(label) <= 6);
}

/**
 * Trims a token of its leading and trailing whitespace.
 * @param str 
 * @return 
 */
static char* trimspace(char* str) {
	// printf("Trimming %s\n", str);

	char* start = str;
	while (isblank(*start)) start++;

	size_t len = strlen(str);

	char* end = str + len - 1;
	while (isblank(*end)) end--;

	size_t newlen = end - start + 1;
	char* ret = (char*) malloc(newlen + 1);
	if (!ret) handleError(ERR_MEM, FATAL, "Could not allocate space for trimmed string!\n");

	strcpy(ret, start);
	*(ret + newlen) = '\0';

	// printf("Trimmed to %s\n", ret);

	// free(str);
	return ret;
}

/**
 * Checks whether the given label is the same as an instruction, pseudo-instruction, or directive.
 * @param label The label to check
 * @return True if is a reserved keyword, false otherwise
 */
static bool usesReserved(char* label) {
	bool isReserved = false;

	if (contains(VALID_INS, label) || contains(VALID_PSEUDO, label) || contains(VALID_DIRCTVS, label)) isReserved = true;

	return isReserved;
}


static void tokenize(const char* rawSrcLine, char* tokens[3]) {
	// printf("Tokenizing %s\n", rawSrcLine);

	char* temp = strdup(rawSrcLine);
	char* saveptr = NULL;
	char* token = strtok_r(temp, " ", &saveptr);

	char* operandsToken = NULL;

	// For detecting normal and set/equ labels
	// Assume labels are normal, check whether the first token has ":"
	// If normal, keep tokenizing
	// If not, it may be a label for set/equ, so "peek" next token to check if it's set/equ, if yes, mark it as a label
	// HOWEVER, this may not always be accurate since if the programmer used set/equ as an operand label (should not do that anyway)
	// for a single operand instruction with no label (ie jmp SET)
	// Then it will detect jmp to be the label and SET to be the pseudo-instruction and NULL to be operands
	// However however, this will be flagged in synax check
	// It should also be fine if the syntax is switched, ie LAB0: equ 3; LAB1 jmp ADDR
	// Both label fields will be marked as existing but it will be flagged in syntax check that LAB0: is not allowed for equ (and set)

	if (strchr(token, ':')) {
		tokens[0] = strdup(token);

		char* tok = strtok_r(NULL, " ", &saveptr);
		tokens[1] = (!tok) ? NULL : strdup(tok);
	} else {
		// Either it is a label for set/equ or an instruction
		char* next = strtok_r(NULL, " ", &saveptr);
		if (next && ((strcasecmp(next, "equ") == 0) || strcasecmp(next, "set") == 0)) {
			// It is a label for set/equ, at least according to the following token
			// Refer to comment at top
			tokens[0] = strdup(token);
			tokens[1] = strdup(next);
		} else {
			// It is an "instruction"
			tokens[1] = strdup(token);

			// next is null when there is only one instruction (like hlt or ret)
			if (next) {
				size_t len = strlen(next);
				*(next + len) = ' ';
				saveptr = next;

				// next refers to the following token, that is the operands
				operandsToken = next;
			}
		}
	}

	// FIXME: This whole thing is a mess, fix it!!!!!!

	// Due to how label tokenizing was handled, saveptr might either be in the end of the entire line (when only label and instr)
	// Be at the start of the operands (when the instr is equ or set)
	// Be at the end of the entire line as well (when no label, meaning `token` was the instruction and `next` the operands)
	// However, there is a case when saveptr is at a space, that being when it is an expression
	
	if (!*saveptr && !operandsToken) {
		char** operands = calloc(1, sizeof(char*));
		if (!operands) handleError(ERR_MEM, FATAL, "Could not allocate space for operands array!\n");
		
		operands[0] = NULL;
		tokens[2] = (char*) operands;
		goto END;
	}

	// When saveptr is not at the end of the line (instr is equ or set)
	// Or when saveptr is at the end of the line but operandsToken point to the start of the operands
	if (*saveptr || operandsToken) {
		char* ops = (*saveptr) ? saveptr : operandsToken;

		// the size of the operands array must be the exact number of operands
		// meaning extra allocation cannot be afforded
		// meaning allocation and reallocation happen increments by 1
		// very not time efficient but whatever
		char** operands = calloc(1, sizeof(char*));
		if (!operands) handleError(ERR_MEM, FATAL, "Could not allocate space for operands array!\n");

		int i = 0;

		// char** _temp = operands;

		char* operand = strtok_r(ops, ",", &saveptr);

		while (operand) {
			operands[i] = trimspace(operand);
			// printf("Adding %s to operands array\n", operands[i]);
			operand = strtok_r(NULL, ",", &saveptr);
			
			char** otemp = realloc(operands, (i + 2) * sizeof(char*));
			if (!otemp) handleError(ERR_MEM, FATAL, "Could not reallocate space for operands array!\n");

			operands = otemp;
			i++;
		}
		operands[i] = NULL;

		tokens[2] = (char*) operands;
		// printf("Operand length for %s: %d\n\n", tokens[1], i);
	}

	END:
	free(temp);
}

bool _contains(const char* arr[], int len, char* val) {
	// printf("Checking if contains for %s\n", val);

	if (!val) return false;

	// int len = sizeof(arr) / sizeof(arr[0]);

	// printf("arr len: %d\n", len);

	for (int i = 0; i < len; i++) {
		if (strcasecmp(arr[i], val) == 0) return true;
	}

	return false;
}


src_obj_list_t* lexicalize(char** rawSource, int size) {
	// printf("Lexicalizing for %d raw source lines\n", size);

	src_obj_list_t* srcObjs = initSrcObjsList(10);

	// Go through the array, working with each string
	for (int i = 0; i < size; i++) {
		char* tokens[3] = { NULL };
		tokenize(rawSource[i], tokens);

		char* label = tokens[0];
		char* instr = tokens[1];
		char** operands = (char**) tokens[2];

		src_obj_t* srcObj = initSrcObj(label, instr, operands);
		addSrcObj(srcObjs, srcObj);
	}

	return srcObjs;
}

void parseCheck(src_obj_list_t* srcObjs) {
	// printf("Parsing and checking for %d lines\n", srcObjs->count);

	for (int i = 0; i < srcObjs->count; i++) {
		// Note that srcObj->instr must have only one of these as true
		bool isInstr = false, isPsuedo = false, isDirv = false;

		src_obj_t* srcObj = srcObjs->arr[i];

		// printf("%p\n", srcObj);

		char* label = srcObj->label;
		char* instr = srcObj->instr;
		char** operands = srcObj->operands;

		// printf("label: %s; instr: %s; s[0]: %s\n", label, instr, operands[0]);

		// validateLabel(srcObj->label);
		// validateInstr();
		// validateOperands();

		/**
		 * Labels:
		 * Should a label be present (need not for normal instructions but mandatory for equ and set)--
		 * Must trim to 1-5 chars long
		 * First char must be letter, @, or ?
		 * For a normal label, must end with : ()
		 * Not have same name as an instr, pseudo, directive, or reg
		 */
		/**
		 * Instructions/Pseudo/Directives:
		 * Must be a valid one
		 * Allowed to be all uppercase or lowercase
		 */
		/**
		 * Operands (if any):
		 * Are to be delimited by commas
		 * For hex, must begin by number and end with "h", otherwise, treat as a label
		 * For decimal, must begin by number, or can end with "d"
		 * No support for octal
		 * No support for binary
		 * No support for current program counter (for now)
		 * Semi support for ascii constants
		 * Using numbers as reg representation
		 * Labels
		 * Arithmetic expression (expressions may be marked as a single token due to how it is delimited)
		 * No support for using encoding of an instruction as a number
		 * No support for evaluating expression into a reg (a register is to be either its name or a single number)
		 * For instructions that use a reg pair, the reg must always be its letter repr (sp,psw including)
		 * No support for direct address (doing jmp 2bch instead of jmp LABEl)
		 */

		// Check for section configuration
		if (label && !instr) {
			// It's fine if only a NORMAL label
			// That is, check that it ends in :
			char* temp = label;
			while (*temp) temp++;

			// temp should point at null character, check char right before
			if (*(temp - 1) != ':') handleError(ERR_INVALID, FATAL, "Single label %s should end with a colon!\n", label);

			goto LABEL_CHECK;
		}
		// Note that for the following if, label is not checked since labels "can be" optional
		// except for equ/set, although that will be checked later on
		// V just checks that the operands array needs to exist, if an instr does not use it, the array must only have the null terminator
		// meaning if this is raised, it's an issue of the tokenizer
		else if (instr && !operands) handleError(ERR_MISSING_TOKEN, FATAL, "Missing potential operands array!\n");

		// Validate instruction/pseudo/directive
		// Saved instructions are to be all lowercase
		char* temp = instr;
		if (temp) TOLOWER(temp)

		if (contains(VALID_INS, instr)) isInstr = true;
		else if (contains(VALID_PSEUDO, instr)) isPsuedo = true;
		else if (contains(VALID_DIRCTVS, instr)) isDirv = true;

		// printf("%d, %d, %d\n", isInstr, isPsuedo, isDirv);

		// if (!label && instr)
		if (!isInstr && !isPsuedo && !isDirv) handleError(ERR_INVALID, FATAL, "%s is not an instruction, pseudo-instruction, or directive!\n", instr);


		// Validate label and trim if necessary
		if (strcmp(instr, VALID_PSEUDO[1]) == 0 || strcmp(instr, VALID_PSEUDO[2]) == 0) {
			// For directives equ and set, the label must be present
			if (!label) handleError(ERR_MISSING_TOKEN, FATAL, "No label found for directive %s!\n", instr);

			temp = label;
			while (*temp) temp++;

			// temp should point at null character, check char right before
			if (*(temp - 1) == ':') handleError(ERR_INVALID, FATAL, "Directive label %s should not end with a colon!\n", label);
		} else if (label) {
			// For normal labels, check ending to be :
			temp = label;
			while (*temp) temp++;

			// temp should point at null character, check char right before
			if (*(temp - 1) != ':') handleError(ERR_INVALID, FATAL, "Label %s should end with a colon!\n", label);

		} // No need to check when no label present

		LABEL_CHECK:
		if (label) {
			// For all labels (normal or directive)
			if (!validChar(*label)) handleError(ERR_INVALID, FATAL, "Label %s includes disallowed initial character of %c!\n", label, *label);

			// Trim
			// Choice of either allocing new memory to fit just the trimmed label plus : (if used)
			// cost: more time spent in allocation
			// Or to use the same memory and null terminate early, including : (if used)
			// cost: space going to waste
			// Going with simply terminating early
			trim(label);

			if (usesReserved(label)) {
				handleError(ERR_INVALID, FATAL, "Label %s uses a reserved keyword!\n", label);
			}
		}


		// Validate operands
		if (operands) {
			while (*operands) {
				// printf("Operand: %s\n", *operands);

				if (contains(VALID_REGS, *operands)) {
					char* temp = *operands;
					TOLOWER(temp)
				}

				operands++;
			}
		}
	}
}