#ifndef _MEM_H_
#define _MEM_H_

#include <stdint.h>
#include <stdbool.h>

#define KB 1024
#define MAX_ADDR UINT16_MAX
#define WORD_SIZE 8

#define TEXTDATA_SIZE 30 * KB
#define NOACCES_SIZE 4 * KB
#define STACK_SIZE 30 * KB

typedef enum {
	TEXTDATA_SEG = 0,
	NOACCESS_SEG,
	STACK_SEG,
	ERROR_SEG = -1
} seg_t;

typedef struct mem {
	uint16_t maxAddr;
	uint8_t wordSize;
	uint16_t segStart[STACK_SEG+1];
	uint8_t ram[MAX_ADDR];
} mem_t;



void memRead();
void memWrite();

#endif