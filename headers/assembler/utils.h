#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>

#include "SymbolTable.h"


/**
 * Converts the given string to an integer, given the radix base. 1 for binary, 8 for octal, 10 for decimal, 16 for hexadecimal.
 * @param str The string, stripped off of its base prefix/suffix
 * @param radix The base
 * @return The integer
 */
uint32_t atoib(const char* str, int radix);

/**
 * Evaluates the given string representing an expression `expr`. Uses the given symbol table for any labels/symbols used in the expression.
 * If it contains any symbols not yet defined, it stops evaluating. If a symbol A is incomplete (from incEntries[]), it evaluates A's `expr`
 * recursively (if that A's expression also uses an incomplete symbol B, etc) until A's `expr` is evaluated. If at any point, it encounters
 * an undefined symbol, it stops evaluating as well.
 * Basically, when a symbol is encountered, it uses symTable.entries[], then symTable.incEntries[] (recursively if needed).
 * If neither, then it stops.
 * It assume the expression has proper syntax.
 * 
 * Note that evaluation happens in place. That is, it modifies the given string.
 * It may seem that the original allocated space would not be sufficient for the end result,
 * but a percentage is used up by spaces, ops, and parenthesis, and labels. The end result is likely to end up
 * using less. Note that this may go wrong under some conditions but for now, assume true.
 * @param expr The expression to evaluate 
 * @param symTable The symbol table
 * @return True if complete evaluation, false otherwise
 */
bool eval(char* expr, SymbolTable* symTable);

#endif