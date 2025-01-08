#ifndef _TRANSL_GEN_H_
#define _TRANSL_GEN_H_

#include "aef.h"
#include "SymbolTable.h"
#include "parser.h"

typedef char byte;

#define KB 1024
#define MEM_SIZE 64 * KB // Total memory the 8080 can address

typedef struct {
	aef_hdr header;
	uint16_t size; // Size of actual program, can be up to 64 KB
	byte mem[MEM_SIZE]; // In theory, code can take up 64 KB (entire memory space)
} aef_bin_img;


/**
 * Translates the instructions and operands, generating its binary data. It uses the symbol table
 * to resolve all labels before translating. That is, `srcObjs` may maintain its labels and only when
 * the translator gets to a `srcObj` using a label, it refers to the table. In other words, it may evaluate for the last time
 * in the cases any expressions where skipped during symbol creation and expression evaluation.
 * 
 * For the operands that have already been evaluated, they are assumed to be of the proper size. That is, if the operand 
 * is for `org`, it must have been checked that it is a 16-bit number. If not, it should have given an error beforehand.
 * 
 * Returns a binary image ready to be written as an executable.
 * @param symTable The symbol table
 * @param srcObjs 
 * @return Complete binary image
 */
aef_bin_img* translateGenerate(SymbolTable* symTable, src_obj_list_t* srcObjs);

#endif