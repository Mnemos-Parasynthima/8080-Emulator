#ifndef _PARSER_H_
#define _PARSER_H_

#include "lexer.h"

extern const char* VALID_INS[65];
extern const char* VALID_PSEUDO[4];
extern const char* VALID_DIRCTVS[3];
extern const char* VALID_REGS[10];

/**
 * Checks whether the given string array contains the given string value.
 * @param arr The string array
 * @param val The string to check
 * @return True if it contains, false otherwise
 */
bool _contains(const char* arr[], int len, char* val);

#define contains(arr, val) _contains(arr, sizeof(arr) / sizeof((arr)[0]), val)

/**
 * Parses through the list, checking that each source object is appropriate.
 * It also does necessary conversions or changes, such as lowercasing.
 * It mostly conforms to the 8080 assembly data sheet.
 * @param srcObjs 
 */
void parseCheck(src_obj_list_t* srcObjs);

#endif