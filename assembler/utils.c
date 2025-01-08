#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"
#include "Error.h"

typedef char byte;

static uint32_t _atox(const char* str) {
	printf("Incoming numstr: 0x%s\n", str);

	uint32_t num = 0;

	const char* temp = str;

	int i = 0;
	while (*temp) {
		byte offset;

		if (isalpha(*temp)) offset = 0x37;
		else if (isdigit(*temp)) offset = 0x30;

		byte digit = *temp - offset;

		num |= (digit << (i * 4));

		temp++;
		i++;
	}

	printf("Outgoing num: %d/0x%x\n", num, num);

	return num;
}

uint32_t atoib(const char* str, int radix) {
	if (radix == 10) return (uint32_t) atoi(str);

	if (radix == 16) return _atox(str);

	// Todo later: octal and binary

	return 0;
}

void strrev(char* str) {
	size_t len = strlen(str);

	char* start = str;
	char* end = str + len - 1;

	while (start < end) {
		char temp = *end;
		*end = *start;
		*start = temp;

		start++;
		end--;
	}
}

char* itoa(int num, char* buff) {
	char* temp = buff;

	if (num == 0) {
		*temp++ = '0';
		*temp = '\n';

		return buff;
	}

	int rem = 0;
	while (num != 0) {
		rem = num % 10;
		num /= 10;
		*temp++ = rem + 0x30;
	}
	*temp = '\0';

	strrev(buff);

	return buff;
}


#define STACK_DEPTH 5 // Recursive max depth
typedef struct {
	const char* labels[STACK_DEPTH];
	int sp;
} resolution_t;

static void push(resolution_t* rstack, const char* label) {
	rstack->labels[++rstack->sp] = label;
}

static void pop(resolution_t* rstack) {
	if (rstack->sp >= 0) rstack->sp--;
}


typedef enum {
	LABEL_T,
	NUMBER_T,
	OP_T
} token_type_t;
typedef struct {
	token_type_t type; // The type of token
	char* str; // The token itself
	int value; // The value of the token, if it is a number (decimal or hex)
} token_t;

typedef struct {
	char* operator;
	int precedence;
	int assoc;
} operator_t;

operator_t operators[] = {
	{"*", 5, 0},
	{"/", 5, 0},
	{"MOD", 5, 0},
	{"SHL", 5, 0},
	{"SHR", 5, 0},
	{"+", 4, 0},
	{"-", 4, 0},
	{"NOT", 3, 1},
	{"AND", 2, 0},
	{"OR", 1, 0},
	{"XOR", 1, 0},
	{NULL, 0, 0}
};

static token_t* tokenize(char* expr, size_t* count) {
	const char* ops[7] = {
		"MOD", "NOT", "AND", "OR", "XOR", "SHR", "SHL"
	};

	size_t cap = 10; // predict avg tokens in avg expr
	token_t* tokens = (token_t*) malloc(cap * sizeof(token_t*));
	if (!tokens) handleError(ERR_MEM, FATAL, "Could not allocate space for tokens in expression eval!\n");


	char* _expr = expr;
	while (*_expr) {
		while (isblank(*_expr)) _expr++;

		if (isdigit(*_expr)) {
			// First char is a number, it could either be decimal or hex

			int len = 0;

			while (isalnum(*_expr)) {
				_expr++;
				len++;
			}

			char temp = *_expr;
			*_expr = '\0';
			char* tokstr = strdup(_expr - len);
			*_expr = temp;

			int val;

			// temp = *(tokstr + len - 1);
			// *(tokstr + len - 1) = '\0';

			// printf("tokstr is %s\n", tokstr);
			if (*(tokstr + len - 1) == 'h' || *(tokstr + len - 1) == 'H') {
				val = strtol(tokstr, NULL, 16);
			} else { // It is decimal
				val = strtol(tokstr, NULL, 0);
			}
			// printf("val is %d\n", val);
			// *(tokstr + len - 1) = temp;

			if (*count >= cap) {
				token_t* toktemp = (token_t*) realloc(tokens, (cap * 2) * sizeof(token_t));
				if (!toktemp) handleError(ERR_MEM, FATAL, "Could not realloc for expression tokens!\n");
				tokens = toktemp;

				cap *= 2;
			}

			tokens[*count].str = tokstr;
			tokens[*count].type = NUMBER_T;
			tokens[*count].value = val;
		} else if (isalpha(*_expr) || *_expr == '@' || *_expr == '?') {
			// First char is a letter, may be an operator or a label

			_expr++;
			int len = 1;

			while (isalnum(*_expr)) {
				_expr++;
				len++;
			}

			char temp = *_expr;
			*_expr = '\0';
			char* tokstr = strdup(_expr - len);
			*_expr = temp;

			bool isOp = false;
			// Check whether the current token is a word-based operator
			// Assumes that it is lowercase
			// Which must have been since parseCheck() would have
			// lowercased everything except labels
			for (int i = 0; i < 7; i++) {
				if (strcmp(tokstr, ops[i]) == 0) {
					isOp = true;
					break;
				}
			}

			if (*count >= cap) {
				token_t* toktemp = (token_t*) realloc(tokens, (cap * 2) * sizeof(token_t));
				if (!toktemp) handleError(ERR_MEM, FATAL, "Could not realloc for expression tokens!\n");
				tokens = toktemp;

				cap *= 2;
			}

			tokens[*count].str = tokstr;
			tokens[*count].value = -1;
			if (isOp) tokens[*count].type = OP_T;
			else tokens[*count].type = LABEL_T;
		} else {
			// Single character operators

			_expr++;
			int len = 1;

			char temp = *_expr;
			*_expr = '\0';
			char* tokstr = strdup(_expr - len);
			*_expr = temp;

			if (*count >= cap) {
				token_t* toktemp = (token_t*) realloc(tokens, (cap * 2) * sizeof(token_t));
				if (!toktemp) handleError(ERR_MEM, FATAL, "Could not realloc for expression tokens!\n");
				tokens = toktemp;

				cap *= 2;
			}

			tokens[*count].str = tokstr;
			tokens[*count].value = -1;
			tokens[*count].type = OP_T;
		}

		(*count)++;
	}

	return tokens;
}

static int getPrecedence(const char* op) {
	for (int i = 0; operators[i].operator; i++) {
		if (strcmp(operators[i].operator, op) == 0) return operators[i].precedence;
	}

	return 0;
}

static int isRightAssoc(const char* op) {
	for (int i = 0; operators[i].operator; i++) {
		if (strcmp(operators[i].operator, op) == 0) return operators[i].assoc;
	}

	return 0;
}

static token_t* toRPN(token_t* tokens, size_t count, size_t* outcount) {
	size_t cap = 10;
	token_t* rpntokens = (token_t*) malloc(cap * sizeof(token_t*));
	if (!rpntokens) handleError(ERR_MEM, FATAL, "Could not allocate space for rpn tokens!\n");

	token_t stack[256];
	int sp = -1;

	for (size_t i = 0; i < count; i++) {
		token_t token = tokens[i];

		if (token.type == NUMBER_T || token.type == LABEL_T) {
			if (*outcount >= cap) {
				token_t* temp = (token_t*) realloc(rpntokens, (cap * 2) * sizeof(token_t));
				if (!temp) handleError(ERR_MEM, FATAL, "Could not reallocate rpn tokens!\n");
				
				rpntokens = temp;
				cap *= 2;
			}

			rpntokens[(*outcount)++] = token;
		} else if (token.type == OP_T) {
			if (*token.str == '(') stack[sp++] = token;
			else if (*token.str == ')') {
				while (sp > 0 && *stack[sp-1].str == '(') {
					if (*outcount >= cap) {
						token_t* temp = (token_t*) realloc(rpntokens, (cap * 2) * sizeof(token_t));
						if (!temp) handleError(ERR_MEM, FATAL, "Could not reallocate rpn tokens!\n");
						
						rpntokens = temp;
						cap *= 2;
					}

					rpntokens[(*outcount)++] = stack[--sp];
				}

				sp--;
			} else {
				while (sp >= 0 && 
						getPrecedence(stack[sp].str) >= getPrecedence(token.str) &&
						!isRightAssoc(token.str)) {

					if (*outcount >= cap) {
						token_t* temp = (token_t*) realloc(rpntokens, (cap * 2) * sizeof(token_t));
						if (!temp) handleError(ERR_MEM, FATAL, "Could not reallocate rpn tokens!\n");
						
						rpntokens = temp;
						cap *= 2;
					}

					rpntokens[(*outcount)++] = stack[sp--];
				}

				stack[++sp] = token;
			}
		}
	}
	
	while (sp >= 0) {
		if (*outcount >= cap) {
			token_t* temp = (token_t*) realloc(rpntokens, (cap * 2) * sizeof(token_t));
			if (!temp) handleError(ERR_MEM, FATAL, "Could not reallocate rpn tokens!\n");
			
			rpntokens = temp;
			cap *= 2;
		}

		rpntokens[(*outcount)++] = stack[sp--];
	}

	return rpntokens;
}

static bool isCircularDependent(resolution_t* rstack, const char* label) {
	for (int i = 0; i <= rstack->sp; i++) {
		if (strcmp(rstack->labels[i], label) == 0) return true;
	}

	return false;
}

/**
 * Attempts evaluation of the expression pointed to by the given label.
 * The expression may either be just the number (is in entries[]) or is an actual expression
 * (is in incEntries[]). If so, attempt to do `eval()` on it
 * @param label The label to attempt to evaluate
 * @param symTable The symbol table
 * @param val The evaluated value
 * @param rstack 
 * @return 
 */
static bool evalLabel(const char* label, SymbolTable* symTable, int* val, resolution_t* rstack) {
	if (isCircularDependent(rstack, label)) return false;

	push(rstack, label);

	sym_entry_t* entry = getEntry(symTable, label);

	inc_sym_entry_t* incEntry = NULL;	
	if (!entry) {
		incEntry = getIncEntry(symTable, label);
		if (!incEntry) {
			// This should really not happen
			pop(rstack);
			return false;
		}

		if (!eval(incEntry->expr, symTable)) {
			pop(rstack);
			return false;
		}

		completeAndAdd(symTable, incEntry);

		// The incomplete entry now should have been evaluated
		// and be in the complete entries
		entry = getEntry(symTable, label);
		if (!entry) {
			pop(rstack);
			return false;
		}
	}

	*val = (int) entry->data;

	pop(rstack);

	return true;
}

/**
 * Evaluates the stream of RPN tokens. Final result is placed in the original expression string.
 * @param tokens 
 * @param count 
 * @param symTable 
 * @param rstack 
 * @param expr 
 * @return 
 */
static bool evalRPN(token_t* tokens, size_t count, SymbolTable* symTable, resolution_t* rstack, char* expr) {
	int stack[count];
	size_t sp = 0;

	for (size_t i = 0; i < count; i++) {
		token_t token = tokens[i];

		if (token.type == NUMBER_T) {
			stack[sp++] = token.value;
		} else if (token.type == LABEL_T) {
			int val;

			if (!evalLabel(token.str, symTable, &val, rstack)) return false;

			stack[sp++] = val;
		} else if (token.type == OP_T) {
			if (sp < 2) return false;

			int b = stack[--sp];
			int a = stack[--sp];

			if (*(token.str) == '*') stack[sp++] = a * b;
			else if (*(token.str) == '/') stack[sp++] = a / b;
			else if (strcmp(token.str, "mod") == 0) stack[sp++] = a % b;
			else if (strcmp(token.str, "shl") == 0) stack[sp++] = a >> b;
			else if (strcmp(token.str, "shr") == 0) stack[sp++] = a << b;
			// else if (strcmp()
			// TODO: Handle unary and binary +/-

			else if (*(token.str) == '+') stack[sp++] = a + b;
			else if (*(token.str) == '-') stack[sp++] = a - b;

			else if (strcmp(token.str, "and") == 0) stack[sp++] = a & b;
			else if (strcmp(token.str, "or") == 0) stack[sp++] = a | b;
			else if (strcmp(token.str, "xor") == 0) stack[sp++] = a ^ b;
		}
	}

	if (sp > 1) return false;

	itoa(stack[0], expr);

	return true;
}

bool eval(char* expr, SymbolTable* symTable) {
	// printf("Evaluating expression %s to....\n", expr);

	size_t count = 0;
	token_t* tokens = tokenize(expr, &count);


	if (count == 1 && (*tokens).value >= 0) {
		itoa((*tokens).value, expr);
		// printf("\t%s\n", expr);

		return true;
	}

	size_t outcount = 0;
	token_t* rpnTokens = toRPN(tokens, count, &outcount);
	free(tokens);

	resolution_t stack = {.sp = -1};

	bool ret = evalRPN(rpnTokens, count, symTable, &stack, expr);
	free(rpnTokens);

	// printf("\t%s\n", expr);

	return ret;
}