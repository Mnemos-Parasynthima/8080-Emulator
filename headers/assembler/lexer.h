#ifndef _LEXER_H_
#define _LEXER_H_

#include <ctype.h>

typedef struct SourceObject {
	char* label; // The label token, if any (equ and set must have it)
	char* instr; // The instruction, directive, or psuedo-instruction token
	char** operands; // The null-terminated array of operands tokens, once separated by commas
} src_obj_t;

typedef struct SourceObjectList {
	src_obj_t** arr; // The array of source objects
	int count;
	int cap;
} src_obj_list_t;


#define TOLOWER(temp) for (; *temp; ++temp) *temp = tolower(*temp);
#define TOUPPER(temp) for (; *temp; ++temp) *temp = toupper(*temp);


void deleteSrcObjsList(src_obj_list_t* srcObjs);

/**
 * Performs lexical analysis on the given array of raw source lines, returning
 * an array of source objects containing lexical token groupings.
 * Note that the given raw source strings and the strings in the source objects are deep copied.
 * Meaning that the data in each line of `rawSource` is in a different location that the data in each `srcObj`.
 * @param rawSource The array of raw source strings
 * @param size Length of the array
 * @return 
 */
src_obj_list_t* lexicalize(char** rawSource, int size);

#endif