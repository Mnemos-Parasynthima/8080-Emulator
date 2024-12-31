#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ResourceTable.h"
#include "Error.h"


static void deleteIncEntry(inc_rsrc_entry_t* entry);

ResourceTable* initTable(int size) {
	ResourceTable* table = (ResourceTable*) malloc(sizeof(ResourceTable));
	if (!table) handleError(ERR_MEM, FATAL, "Could not allocate space for resource table!\n");

	table->entries = (rsrc_entry_t**) calloc(size, sizeof(rsrc_entry_t*));
	if (!table->entries) handleError(ERR_MEM, FATAL, "Could not allocate space for resource table entries!\n");

	table->size = 0;
	table->cap = size;

	return table;
}

rsrc_entry_t* initEntry(char* label, long data, bool canRedefine) {
	rsrc_entry_t* entry = (rsrc_entry_t*) malloc(sizeof(rsrc_entry_t));
	if (!entry) handleError(ERR_MEM, FATAL, "Could not allocate space for entry!\n");

	entry->label = (char*) malloc(strlen(label) + 1);
	if (!entry->label) handleError(ERR_MEM, FATAL, "Could not allocate space for entry label!\n");
	strcpy(entry->label, label);
	entry->data = data;
	entry->canRedefine = canRedefine;

	return entry;
}

inc_rsrc_entry_t* initIncEntry(char* label, char* expr, bool canRedefine) {
	inc_rsrc_entry_t* entry = (inc_rsrc_entry_t*) malloc(sizeof(rsrc_entry_t));
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

void addEntry(ResourceTable* rsrcTable, rsrc_entry_t* entry) {
	// Check that the entry does not exist
	rsrc_entry_t* _entry = getEntry(rsrcTable, entry, entry->label);

	if (_entry) {
		// In the case that it does exist, check whether it is able to be redefined
		if (_entry->canRedefine) {
			// The entry was made from SET, it can be redefined
			// Update _entry
			_entry->data = entry->data;
			return;
		} else {
			// The existing entry cannot be redefined
			handleError(ERR_REDEFINED, FATAL, "Label %s cannot be redefined!\n", entry->label);
		}
	}

	// Entry does not exist, add it
	// But check if no more space
	if (rsrcTable->size == rsrcTable->cap) {
		// Add more space
		rsrc_entry_t** temp = realloc(rsrcTable->entries, rsrcTable->cap * 2);
		if (!temp) handleError(ERR_MEM, FATAL, "Could not reallocate space for new entry!\n");

		rsrcTable->entries = temp;

		void* startingPoint = temp + rsrcTable->size;
		memset(startingPoint, 0x0, sizeof(rsrc_entry_t*) * 2);

		rsrcTable->cap *= 2;
	}

	// Loop until empty space to add
	for (int i = 0; i < rsrcTable->cap; i++) {
		if (rsrcTable->entries[i] == NULL) {
			rsrcTable->entries[i] = entry;
			rsrcTable->size++;
			return;
		}
	}
}

void addIncEntry(ResourceTable* rsrcTable, inc_rsrc_entry_t* entry) {
}

bool hasEntry(ResourceTable* rsrcTable, rsrc_entry_t* entry) {
	return false;
}

bool hasIncEntry(ResourceTable* rsrcTable, inc_rsrc_entry_t* entry) {
	return false;
}

rsrc_entry_t* getEntry(ResourceTable* rsrcTable, rsrc_entry_t* entry, char* label) {
	return NULL;
}

inc_rsrc_entry_t* getIncEntry(ResourceTable* rsrcTable, inc_rsrc_entry_t* entry, char* label) {
	return NULL;
}

static void deleteEntry(rsrc_entry_t* entry) {
	if (!entry) return;
	
	if (entry->label) free(entry->label);
	free(entry);
}

static void deleteIncEntry(inc_rsrc_entry_t* entry) {
	if (!entry) return;

	if (entry->label) free(entry->label);
	if (entry->expr) free(entry->expr);
	free(entry);
}

void deleteTable(ResourceTable* rsrcTable) {
	for (int i = 0; i < rsrcTable->size; i++) {
		deleteEntry(rsrcTable->entries[i]);
	}

	free(rsrcTable);
}
