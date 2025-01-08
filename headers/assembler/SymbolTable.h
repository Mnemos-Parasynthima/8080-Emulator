#ifndef _RSRC_TABLE_H_
#define _RSRC_TABLE_H_

#include <stdbool.h>

// Complete Symbol entry.
typedef struct {
	char* label; // Symbol label
	long data; // Symbol data (either normal data or an address)
	bool canRedefine; // Can reuse label or not (equ: 0, set: 1, instructions: 0)
} sym_entry_t;

// Incomplete Symbol entry. Requires its expression to be evaluated. 
typedef struct {
	char* label; // Symbol label
	char* expr; // Expression to be evaluated (can either resolve to an address or data)
	bool canRedefine; // Can reuse label or not (equ: 0, set: 1)
} inc_sym_entry_t;


// The symbol table to hold all complete entryes and incomplete entries.
typedef struct {
	sym_entry_t** entries; // The complete symbol entries
	int entriesSize; // The complete symbol entries array size
	int entriesCap; // The complete symbol entries array capacity
	inc_sym_entry_t** incEntries; // The incomplete symbol entries
	int incEntriesSize; // The incomplete symbol entries array size
	int incEntriesCap; // The incomplete symbol entries array capacity
} SymbolTable;


/**
 * Initializes a symbol table with the given size for capacities for the complete and incomplete entry arrays.
 * @param size The capacity to start with for the complete entry array
 * @param incSize The capacity to start with for the incomplete entry array
 * @return An empty symbol table
 */
SymbolTable* initTable(int size, int incSize);
/**
 * Initializes a symbol entry with the given label, data, and whether it can be redefined.
 * @param label The label (will be strcpy'd)
 * @param data The data (either a normal number or a number to be interpreted as an address [instruction labels])
 * @param canRedefine Whether the label/symbol can be redefined. Only true if it is from `set`, false otherwise
 * @return An initialized complete symbol entry
 */
sym_entry_t* initEntry(char* label, long data, bool canRedefine);
/**
 * Initializes an incomplete symbol entry with the given label, expression, and whether it can be redefined.
 * These occur when the label uses an expression (only in the case of using `set` or `equ`) which would require the need
 * to evaluate the expression, then resolving it to form a complete entry.
 * @param label The label (will be strcpy'd)
 * @param expr The expression to be evaluated
 * @param canRedefine Whether the symbol can be redefined. Only true if it is from `set`, false otherwise
 * @return An initialized incomplete symbol entry
 */
inc_sym_entry_t* initIncEntry(char* label, char* expr, bool canRedefine);

/**
 * Adds the given complete entry to the symbol table.
 * @param symTable The symbol table
 * @param entry The complete entry to add
 */
void addEntry(SymbolTable* symTable, sym_entry_t* entry);
/**
 * Adds the given incomplete entry to the symbol table.
 * @param symTable The symbol table
 * @param entry The incomplete entry to add
 */
void addIncEntry(SymbolTable* symTable, inc_sym_entry_t* entry);
/**
 * Checks whether the provided entry exists in the symbol table. This is used to check whether the provided
 * entry's symbol/label allowes for redefinition.
 * @param symTable The symbol table
 * @param label The entry's label to check
 * @return True if it exists, false otherwise
 */
// bool hasEntry(SymbolTable* symTable, char* label);
/**
 * Checks whether the provided incomplete entry exists in the symbol table. This is used to check whether the provided
 * incomplete entry's symbol allows for redefinition.
 * @param symTable The symbol table
 * @param label The incomplete entry's label to check
 * @return True if it exists, false otherwise
 */
// bool hasIncEntry(SymbolTable* symTable, char* label);
/**
 * Gets the entry specified by the label from the symbol table.
 * @param symTable The symbol table
 * @param label The target label
 * @return The entry, if it exists
 */
sym_entry_t* getEntry(SymbolTable* symTable, const char* label);
/**
 * Gets the incomplete entry specified by the label from the symbol table.
 * @param symTable The symbol table
 * @param label The target label
 * @return The incomplete entry, if it exists
 */
inc_sym_entry_t* getIncEntry(SymbolTable* symTable, char* label);

/**
 * Converts the given incomplete entry to a complete one, adding it to the table afterwards.
 * This should only happen when `entry.expr` no longer holds an expression but rather a number, represented as a string.
 * Deletes the incomplete entry.
 * @param symTable The symbol table
 * @param entry 
 */
void completeAndAdd(SymbolTable* symTable, inc_sym_entry_t* entry);

/**
 * Deletes the given symbol table, deleting its entries and its contents as well.
 * @param symTable The symbol table to delete
 */
void deleteTable(SymbolTable* symTable);

#endif