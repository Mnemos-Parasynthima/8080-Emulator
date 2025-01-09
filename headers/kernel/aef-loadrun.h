#ifndef _AEF_LOADRUN_H
#define _AEF_LOADRUN_H

#include <stdint.h>

uint16_t loadAEF(const char* filename);
int runAEF(const uint16_t entry);

#endif