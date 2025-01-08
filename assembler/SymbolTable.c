#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "SymbolTable.h"
#include "Error.h"

static void deleteEntry(sym_entry_t* entry);
static void deleteIncEntry(inc_sym_entry_t* entry, bool deleteLabel, bool deleteExpr);

SymbolTable* initTable(int size, int incSize) {
	SymbolTable* table = (SymbolTable*) malloc(sizeof(SymbolTable));
	if (!table) handleError(ERR_MEM, FATAL, "Could not allocate space for symbol table!\n");

	table->entries = (sym_entry_t**) calloc(size, sizeof(sym_entry_t*));
	if (!table->entries) handleError(ERR_MEM, FATAL, "Could not allocate space for symbol table entries!\n");

	table->entriesSize = 0;
	table->entriesCap = size;

	table->incEntries = (inc_sym_entry_t**) calloc(size, sizeof(inc_sym_entry_t*));
	if (!table->incEntries) handleError(ERR_MEM, FATAL, "Could not allocate space for incomplete symbol table entries!\n");

	table->incEntriesSize = 0;
	table->incEntriesCap = incSize;

	return table;
}

sym_entry_t* initEntry(char* label, long data, bool canRedefine) {
	sym_entry_t* entry = (sym_entry_t*) malloc(sizeof(sym_entry_t));
	if (!entry) handleError(ERR_MEM, FATAL, "Could not allocate space for entry!\n");

	entry->label = (char*) malloc(strlen(label) + 1);
	if (!entry->label) handleError(ERR_MEM, FATAL, "Could not allocate space for entry label!\n");
	strcpy(entry->label, label);
	entry->data = data;
	entry->canRedefine = canRedefine;

	return entry;
}

inc_sym_entry_t* initIncEntry(char* label, char* expr, bool canRedefine) {
	inc_sym_entry_t* entry = (inc_sym_entry_t*) malloc(sizeof(sym_entry_t));
	if (!entry) handleError(ERR_MEM, FATAL, "Could not allocate space for entry!\n");

	entry->label = (char*) malloc(strlen(label) + 1);
	if (!entry->label) handleError(ERR_MEM, FATAL, "Could not allocate space for entry label!\n");
	strcpy(entry->label, label);
	entry->expr = (char*) malloc(strlen(expr) + 1);
	if (!entry->expr) handleError(ERR_MEM, FATAL, "Could not allocate space for entry expression!\n");
	strcpy(entry->expr, expr);
	entry->canRedefine = canRedefine;

	return entry;
}

void addEntry(SymbolTable* symTable, sym_entry_t* entry) {
	// Check that the entry does not exist
	sym_entry_t* _entry = getEntry(symTable, entry->label);

	if (_entry) {
		// In the case that it does exist, check whether it is able to be redefined
		if (_entry->canRedefine) {
			// The entry was made from SET, it can be redefined
			// Update _entry
			_entry->data = entry->data;

			// Since data was only updated, the entry object is not used, delete it
			deleteEntry(entry);
			return;
		} else {
			// The existing entry cannot be redefined
			handleError(ERR_REDEFINED, FATAL, "Label %s cannot be redefined!\n", entry->label);
		}
	}

	// Entry does not exist, add it
	// But check if no more space
	if (symTable->entriesSize == symTable->entriesCap) {
		// Add more space
		sym_entry_t** temp = realloc(symTable->entries, (symTable->entriesCap * 2) * sizeof(sym_entry_t*));
		if (!temp) handleError(ERR_MEM, FATAL, "Could not reallocate space for new entry!\n");

		symTable->entries = temp;

		void* startingPoint = temp + symTable->entriesSize;
		memset(startingPoint, 0x0, sizeof(sym_entry_t*) * 2);

		symTable->entriesCap *= 2;
	}

	int idx = symTable->entriesSize;
	symTable->entries[idx] = entry;
	symTable->entriesSize++;
}

void addIncEntry(SymbolTable* symTable, inc_sym_entry_t* entry) {
	// Same logic applies as addEntry except it works on incomplete entries

	inc_sym_entry_t* _entry = getIncEntry(symTable, entry->label);

	if (_entry) {
		if (_entry->canRedefine) {
			_entry->expr = entry->expr;

			deleteIncEntry(entry, true, false);
			return;
		} else {
			handleError(ERR_REDEFINED, FATAL, "Label %s cannot be redefined!\n", entry->label);
		}
	}

	if (symTable->incEntriesSize == symTable->incEntriesCap) {
		inc_sym_entry_t** temp = realloc(symTable->incEntries, (symTable->incEntriesCap * 2) * sizeof(inc_sym_entry_t*));
		if (!temp) handleError(ERR_MEM, FATAL, "Could not reallocate space for new entry!\n");

		symTable->incEntries = temp;

		void* startingPoint = temp + symTable->incEntriesSize;
		memset(startingPoint, 0x0, sizeof(sym_entry_t*) * 2);

		symTable->incEntriesCap *= 2;
	}

	int idx = symTable->incEntriesSize;
	symTable->incEntries[idx] = entry;
	symTable->incEntriesSize++;
}

// bool hasEntry(SymbolTable* symTable, char* label) {
// 	return false;
// }

// bool hasIncEntry(SymbolTable* symTable, char* label) {
// 	return false;
// }

sym_entry_t* getEntry(SymbolTable* symTable, const char* label) {
	sym_entry_t* entry;

	for (int i = 0; i < symTable->entriesSize; i++) {
		entry = symTable->entries[i];

		if (strcmp(entry->label, label) == 0) return entry;
	}

	return NULL;
}

inc_sym_entry_t* getIncEntry(SymbolTable* symTable, char* label) {
	inc_sym_entry_t* entry;

	for (int i = 0; i < symTable->incEntriesSize; i++) {
		entry = symTable->incEntries[i];

		if (strcmp(entry->label, label) == 0) return entry;
	}

	return NULL;
}

void completeAndAdd(SymbolTable* symTable, inc_sym_entry_t* entry) {
	char* label = entry->label;
	char* data = entry->expr;

	sym_entry_t* _entry = initEntry(label, atoi(data), entry->canRedefine);
	addEntry(symTable, _entry);

	deleteIncEntry(entry, true, true);
}

static void deleteEntry(sym_entry_t* entry) {
	if (!entry) return;
	
	if (entry->label) free(entry->label);
	free(entry);
}

static void deleteIncEntry(inc_sym_entry_t* entry, bool deleteLabel, bool deleteExpr) {
	if (!entry) return;

	if (entry->label && deleteLabel) free(entry->label);
	if (entry->expr && deleteExpr) free(entry->expr);
	free(entry);
}

void deleteTable(SymbolTable *symTable)
{
	if (!symTable) return;

	for (int i = 0; i < symTable->entriesSize; i++) {
		deleteEntry(symTable->entries[i]);
	}

	// No need to iterate through the incomplete entry array
	// as when through resolution, all incomplete entries will be converted to complete entries
	// then freed at the spot
	// If there are any leftover incomplete entires, something went wrong in resolution

	free(symTable);
}