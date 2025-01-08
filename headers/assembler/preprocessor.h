#ifndef _PREPROCESS_H
#define _PREPROCESS_H

#include <stdio.h>

#include "Error.h"

/**
 * Returns an allocated array of trimmed source lines, excluding comment lines.
 * @param sourceFile The source assembly file
 * @param size The size of the returned array, length of instructions and directives
 * @return 
 */
char** preprocess(FILE* sourceFile, int* size);


#endif