#ifndef _AEF_H
#define _AEF_H

#include <stdint.h>

typedef struct {
	unsigned char ident[8]; // 8B
	uint16_t entry; // Entry point physical address 2B
	// uint8_t ahsize; // AEF header size 1B
} aef_hdr;

#define AI_MAGIC0 0
#define AEF_MAGIC0 0xae

#define AI_MAGIC1 1
#define AEF_MAGIC1 'A'

#define AI_MAGIC2 2
#define AEF_MAGIC2 'E'

#define AI_MAGIC3 3
#define AEF_MAGIC3 'F'

#define AI_PAD 4

#endif