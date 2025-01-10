#ifndef _INSTR_H_
#define _INSTR_H_




/**
 * Flag Word:
 * [S Z 0 AC 0 P 1 CY]
 * 
 * S: Sign
 * 	Set if the most significant bit of the result of the operation is 1
 * 
 * Z: Zero
 * 	Set when the result of an instruction is 0
 * 
 * AC: Auxiliary Carry
 * 	Set when an instruction caused a carry out of bit 3 into bit 4
 *  of the resulting value
 * 
 * P: Parity
 * 	Set if the modulo 2 sum of the bits of the result of the operation is 0
 * 
 * CY: Carry
 * 	Set if an instruction resulted in a carry (from addition) or a borrow
 * 	(from subtraction or a comparison)
 */
#define PACK_EFLAGS(Z,S,P,CY,AC) ((S<<7)|(Z<<6)|(0<<5)|(AC<<4)|(0<<3)|(P<<2)|(1<<1)|(CY<<0))
#define GET_S(eflags) ((eflags>>7) & 0x1)
#define GET_Z(efalgs) ((eflags>>6) & 0x1)
#define GET_AC(eflags) ((eflags>>4) & 0x1)
#define GET_P(eflags) ((eflags>>2) & 0x1)
#define GET_CY(eflags) ((eflags>>0) & 0x1)

typedef enum {
	OP_NOP
} opcode_t;

typedef enum {
	C_EQ
} cond_t;

typedef enum {
	STAT_OK,
	STAT_HLT,
	STAT_ADR,
	STAT_INS
} stat_t;


#endif