#ifndef _RSRC_TABLE_H_
#define _RSRC_TABLE_H_

#include <stdbool.h>

// Complete resource entry. It can either be from EQU or SET
typedef struct {
	char* label; // Resource label
	long data; // Resource data
	bool canRedefine; // Can reuse label or not (equ: 0, set: 1)
} rsrc_entry_t;

// Incomplete resource entry. Requires its expression to be evaluated. It can either be from EQU or SET
typedef struct {
	char* label; // Resource label
	char* expr; // Expression to be evaluated
	bool canRedefine; // Can reuse label or not (equ: 0, set: 1)
} inc_rsrc_entry_t;


typedef struct {
	rsrc_entry_t** entries;
	inc_rsrc_entry_t** incEntries;
	int size;
	int cap;
} ResourceTable;


ResourceTable* initTable(int size);
rsrc_entry_t* initEntry(char* label, long data, bool canRedefine);
inc_rsrc_entry_t* initIncEntry(char* label, char* expr, bool canRedefine);

void addEntry(ResourceTable* rsrcTable, rsrc_entry_t* entry);
void addIncEntry(ResourceTable* rsrcTable, inc_rsrc_entry_t* entry);
bool hasEntry(ResourceTable* rsrcTable, rsrc_entry_t* entry);
bool hasIncEntry(ResourceTable* rsrcTable, inc_rsrc_entry_t* entry);
rsrc_entry_t* getEntry(ResourceTable* rsrcTable, rsrc_entry_t* entry, char* label);
inc_rsrc_entry_t* getIncEntry(ResourceTable* rsrcTable, inc_rsrc_entry_t* entry, char* label);

void deleteTable(ResourceTable* rsrcTable);

#endif