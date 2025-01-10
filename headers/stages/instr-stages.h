#ifndef _INSTR_STAGES_H
#define _INSTR_STAGES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	FETCH_STAGE,
	DECODE_STAGE,
	EXECUTE_STAGE

} proc_stage_t;

typedef enum {
	PLUS_OP,
	MINUS_OP,
	OR_OP,
	XOR_OP,
	AND_OP
} alu_op_t;

typedef struct ctrlSigs {
	bool INTE;
	bool HLDA;
	bool DBIN;
	bool _WR;
	bool WAIT;
} ctrl_sigs_t;

// Status information, to be sent out on databus
typedef struct statusSigs {
	bool INTA; // True to ack for interrupt req, used to gate a restart instr onto databus when active DBIN
	bool _WO; // True if read memory or input op (!WO = 1), false if write memory or output op (!WO = 0)
	bool STACK; // True if addrbus holds pushdown stack addr from SP
	bool HLTA; // True to ack signal for halt instr
	bool OUT; // True if addrbus contains addr of an output device, databus contains output data when active WR
	bool M1; // True if CPU is in fetch cycle for first byte of instr
	bool INP; // True if addrbus contains addr of an input device, input data to be placed on databus when active DBIN
	bool MEMR; // True if databus used for memory read data
} status_sigs_t;

typedef struct state {
	uint8_t intdatabus; // Internal data bus
	status_sigs_t statusSigs; // Signals to be used external of the CPU
	ctrl_sigs_t ctrSigs; // 
} state_t;


void fetch();

#endif