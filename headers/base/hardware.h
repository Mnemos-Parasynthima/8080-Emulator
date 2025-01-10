#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include "instr.h"
#include "instr-stages.h"

void regarray(bool wr, uint8_t src, uint8_t dst);

void alu(alu_op_t aluop, bool seteflags);

/**
 * Converts the status signals to bits, placing them on the data bus.
 */
void sendStatusToData();

void latchStatus();

void mem();

#endif