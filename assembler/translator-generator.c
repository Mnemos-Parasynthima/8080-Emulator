#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>

#include "translator-generator.h"
#include "utils.h"
#include "Error.h"

#define MAX_SIZE 30 * KB // Default max limit assigned for code+text

#define ASSIGNED_EXCEEDED(limits) (limits >> 4) // Get the truth value for whether the assigned memory was exceeded
#define MEM_EXCEEDED(limits) (limits & 0x01) // Get the truth value for whether the total memory was exceeded
#define SET_ASSIGNED_EXCEEDED(limits) (limits |= 0x10) // Set the value that assigned memory was exceeded
#define SET_MEM_EXCEEDED(limits) (limits |= 0x01) // Set the value that total memory was exceeded


struct insnbytes {
	uint8_t opcode; // The opcode for the instruction in binary
	uint8_t databyte0; // The first data byte in binary
	uint8_t databyte1; // The second data byte in binary
	bool useDatabyte0; // Whether the first data byte is used
	bool useDatabyte1; // Whether the second data byte is used (by proxy, first data byte is to be marked as used)
};


/**
 * Converts the given number represented as a string to a number. The number string may include information about its
 * base. Hexadecimals contain "h" at the end. Binary contain "b" at the end. Octals contain "O" at the end.
 * Decimal may contain "d" at the end.
 * It assumes that the hexadecimals have the appropriate format, that is, it begins with a number.
 * It also assumes any letters are lowercase.
 * It overall assumes that the given string represents a proper number, no matter the base.
 * @param numstr The string representing a number
 * @return A number
 */
static uint32_t toNumber(char* numstr) {
	// Detect what base the number is in
	int base = 10;

	size_t len = strlen(numstr);
	char end = *(numstr + len - 1);
	switch (end)	{
		case 'h':
			base = 16;
			*(numstr + len - 1) = '\0';
			break;
		case 'b':
			base = 2;
			*(numstr + len - 1) = '\0';
			break;
		case 'o':
			base = 8;
			*(numstr + len - 1) = '\0';
			break;
		case 'd':
			*(numstr + len - 1) = '\0';
		default:
			break;
	}

	// Since atoib works on the entire string, first "remove" any suffix before conversion
	// Afterwards, add the suffix back
	// Aka numstr is invariant

	uint32_t num = atoib(numstr, base);
	
	switch (end)	{
		case 'h':
			*(numstr + len - 1) = 'h';
			break;
		case 'b':
			*(numstr + len - 1) = 'b';
			break;
		case 'o':
			*(numstr + len - 1) = 'o';
			break;
		case 'd':
			*(numstr + len - 1) = 'd';
		default:
			break;
	}

	return num;
}

/**
 * Writes the given `_byte` to the memory address `memAddr` in the binary image. If the memory address passes
 * the assigned limit for code+data and/or the total available limit, it sets it in `limits`.
 * It also returns the new memory address to write at.
 * @param binImg The binary image to write to
 * @param memAddr The memory addres to write at
 * @param _byte The byte to write
 * @param limits The limits flag
 * @param size The size of the program
 * @return The next memory address to start writing at
 */
static uint16_t writeByte(aef_bin_img* binImg, uint16_t memAddr, uint8_t _byte, byte* limits, uint16_t* size) {
	binImg->mem[memAddr] = _byte;
	*size += 1;
	// printf("Wrote byte 0x%x at addr 0%x, new program size of %d\n", _byte, memAddr, *size);

	if (memAddr >= MAX_SIZE) {
		SET_ASSIGNED_EXCEEDED(*limits);

		if (memAddr >= MEM_SIZE) {
			SET_MEM_EXCEEDED(*limits);
		}
	}

	return ++memAddr;
}

/**
 * Appplies the directives, if instr is one. This may either set the assembly location to another address or 
 * reserves and fill in data bytes. Binary size is also updated should data bytes be filled in.
 * In the case that the new memory
 * @param instr The pseudo-instruction or directive
 * @param operands The null-terminated operands array (in some cases, there may be only one, it is already evaluated)
 * @param binImg The binary image
 * @param memAddr The memory address to fill in
 * @param size The binary size
 * @param limits The limits flags
 * @param halt Whether to stop assembling
 * @return True if a directive was applied (skip the instruction encoding following this), false otherwise
 */
static bool applyDirectives(char* instr, char** operands, aef_bin_img* binImg, uint16_t* memAddr, uint16_t* size, byte* limits, bool* halt) {
	// Is a regular instruction
	if (contains(VALID_INS, instr)) return false;

	// For directives, it is just setting aside memory
	// Note that the operand(s) of db/dw for these are a list, which have to be evaluated to a number
	// Ie db 5*2,2fH-0aH -> db 0aH,25H :: operands: ["0aH", "25H", \0]
	// db 5abcH shr 8 -> db 5aH :: operands: ["5aH", \0]
	// ds has just an expression (which has to be evaluated at this point)
	// The data sheet does not indicate what type the expression should resolve to, so I'm going with an unsigned integer (uint32_t)

	// to do: better way to check what directive it is, not using strcmp
	// maybe have contains() parameter output some sort of constant indicating what it is
	// that should be configured in lexer.h
	if (strcmp(instr, "ds") == 0) {
		uint16_t check = *memAddr;

		// ds is just moving the memAddr
		uint32_t reserve = toNumber(operands[0]);
		// According to the data sheet, it should not be assumed that the values
		// created due to ds be zero or any other data, but I'm going to write 0s nonetheless
		// The programmer does not know this(?) so the assumption should still hold
		for (uint32_t i = 0; i < reserve; i++) {
			*memAddr = writeByte(binImg, *memAddr, 0, limits, size);
			if (MEM_EXCEEDED(*limits) == 0x1) return false;
		}

		// memAddr should still have increased by `reserve` bytes prior
		// Make sure of that
		assert((check + reserve) == *memAddr);

		return true;
	} else if (strcmp(instr, "db") == 0) {
		// Operands of db may either be a list of data bytes,
		// a single data byte, or an ascii string
		// Reminder that ascii strings are to be seperated by bytes
		// That is: 'HI' -> [0x48, 0x49]
		// So something like: 5*2,'HI' -> [0xa, 0x48, 0x49]
		char** temp = operands;
		while (*temp) {
			// loop through the operands array (null terminated array)
			uint8_t databyte = toNumber(*temp);

			*memAddr = writeByte(binImg, *memAddr, databyte, limits, size);
			if (MEM_EXCEEDED(*limits) == 0x1) return false;

			temp++;
		}

		return true;
	} else if (strcmp(instr, "dw") == 0) {
		// A more 'funky' directive
		// Its operand array contains quantities that should have been
		// evaluated to a single number (will not check for these assertions)
		// However, how its written to memory is funky
		// That is, for a number 0xFAED, 0xED is written first, then 0xFA

		char** temp = operands;
		while (*temp) {
			uint16_t databytes = toNumber(*temp);

			*memAddr = writeByte(binImg, *memAddr, databytes & 0x00FF, limits, size);
			if (MEM_EXCEEDED(*limits) == 0x1) return false;

			*memAddr = writeByte(binImg, *memAddr, databytes >> 8, limits, size);
			if (MEM_EXCEEDED(*limits) == 0x1) return false;

			temp++;
		}

		return true;
	}

	if (strcmp(instr, "org") == 0) {
		// "org" just sets the memory address to start writing from
		// However, chances are that the programmer may set it past the default limit for the code+text segment
		// Still perform the pseudo-instruction but set the proper flag
		uint16_t newAddr = toNumber(operands[0]);

		if (newAddr >= MAX_SIZE) SET_ASSIGNED_EXCEEDED(*limits);

		*memAddr = newAddr;

		return true;
	} else if (strcmp(instr, "end") == 0) {
		// Stop assembling
		*halt = true;
		return true; 
	}

	return false;
}


static uint8_t getReg(char* regOperand, SymbolTable* symTable) {
	// In case regOperand is not completely evaluated
	// that is, it is still some expression and/or it has labels
	// Label register names (A-M) should be resolved to their numbers
	eval(regOperand, symTable);

	uint8_t num = (uint8_t) toNumber(regOperand);

	assert(num >= 0b000 && num <= 0b111);

	return num;
}


static uint16_t getImm(char* immOperand, SymbolTable* symTable) {
	eval(immOperand, symTable);

	uint16_t num = (uint16_t) toNumber(immOperand);

	return num;
}


static uint8_t getRP(char* rpOperand, SymbolTable* symTable) {
	eval(rpOperand, symTable);

	// VALID_REG[8] is SP, which is defined as a register pair
	// Same for VALID_REG[9] := PSW
	// Regs B, D, and H can be interpreted as a reg pair as well
	// However, that is not how it is encoded
	// B-C pair: 00
	// D-E pair: 01
	// H-L pair: 10
	// SP pair: 11
	// PSW pair: 11

	uint8_t num = (uint8_t) toNumber(rpOperand);

	// Only acceptable numbers (for now)
	// It should already be handle prior or just in case
	// 						reg b					reg d 					reg h							sp							psw
	assert(num == 0b00 || num == 0b10 || num == 0b100 || num == 0b1000 || num == 0b1001);

	// However, convert the numbers in accordance to the pairing prior

	if (num == 0b10) num = 0b01;
	else if (num == 0b100) num = 0b10;
	else if (num == 0b1000 || num == 0b1001) num = 0b11;
	// else num == 0b00: num = 0b00, no change

	return num;
}

static uint8_t translateCC(char* suffix) {
	uint8_t ccbits = 0b000;

	if (*suffix == 'z') ccbits = 0b001;
	else if (strcmp(suffix, "nc") == 0) ccbits = 0b010;
	else if (*suffix == 'c') ccbits = 0b011;
	else if (strcmp(suffix, "po") == 0) ccbits = 0b100;
	else if (strcmp(suffix, "pe") == 0) ccbits = 0b101;
	else if (*suffix == 'p') ccbits = 0b110;
	else if (*suffix == 'm') ccbits = 0b111;
	// else suffix == "nz": ccbits = 0b000, default

	return ccbits;
}

static void encode(SymbolTable* symTable, char* instr, char** operands, struct insnbytes* insn, uint16_t* memAddr) {
	// todo later, better way to lookup/compare instructions, refer to to do comment in applyDirectives()

	// assumption is that instr is in VALID_INS
	// further assumption is that operands array hold the appropriate operands
	// but cannot assume that each operand is completely evaluated

	uint8_t opcode = 0b00000000;
	uint8_t databyte0 = 0b00000000;
	uint8_t databyte1 = 0b00000000;
	bool useDatabyte0 = false;
	bool useDatabyte1 = false;

	uint8_t reg = 0b0; // Single register
	uint8_t dst = 0b0; // Destination register
	uint8_t src = 0b0; // Source register
	uint8_t rp = 0b0; // Register pair
	uint16_t imm = 0b0; // Immediate (either 1 byte or 2 bytes)

	// FIXME: Fix the left shift sizes!!

	if (strcmp(instr, "inr") == 0) {
		opcode = 0b00000100;

		reg = getReg(operands[0], symTable);

		opcode |= (reg << 3);
	} else if (strcmp(instr, "dcr") == 0) {
		opcode = 0b00000101;

		reg = getReg(operands[0], symTable);

		opcode |= (reg << 3);
	} else if (strcmp(instr, "cma") == 0) {
		opcode = 0b00101111;
		assert(operands[0] == NULL);
	} else if (strcmp(instr, "mov") == 0) {
		opcode = 0b01000000;

		dst = getReg(operands[0], symTable);
		src = getReg(operands[1], symTable);

		opcode |= (dst << 3) | src;
	} else if (strcmp(instr, "stax") == 0) {
		opcode = 0b00000010;

		rp = getRP(operands[0], symTable);

		// This should be taken care of beforehand but make sure it is
		// Only pair B or pair D
		assert(rp == 0b00 || rp == 0b01);

		opcode |= (rp << 4);
	} else if (strcmp(instr, "ldax") == 0) {
		opcode = 0b00001010;

		rp = getRP(operands[0], symTable);

		assert(rp == 0b00 || rp == 0b01);

		opcode |= (rp << 4);
	} else if (strcmp(instr, "add") == 0) {
		opcode = 0b10000000;

		reg = getReg(operands[0], symTable);

		opcode |= reg;
	} else if (strcmp(instr, "adc") == 0) {
		opcode = 0b10001000;

		reg = getReg(operands[0], symTable);

		opcode |= reg;
	} else if (strcmp(instr, "sub") == 0) {
		opcode = 0b10010000;

		reg = getReg(operands[0], symTable);

		opcode |= reg;
	} else if (strcmp(instr, "ana") == 0) {
		opcode = 0b10100000;

		reg = getReg(operands[0], symTable);

		opcode |= reg;
	} else if (strcmp(instr, "xra") == 0) {
		opcode = 0b10101000;

		reg = getReg(operands[0], symTable);

		opcode |= reg;
	} else if (strcmp(instr, "ora") == 0) {
		opcode = 0b10110000;

		reg = getReg(operands[0], symTable);

		opcode |= reg;
	} else if (strcmp(instr, "cmp") == 0) {
		opcode = 0b10111000;

		reg = getReg(operands[0], symTable);

		opcode |= reg;
	} else if (strcmp(instr, "push") == 0) {
		opcode = 0b11000101;

		rp = getRP(operands[0], symTable);

		// For 0b11, it is to be interpreted to be PSW and not SP
		// That info is lost in rp but not in operands
		// Will assert with the assumption that the operand is in its number form
		// That is, either "0", "2", "4", or "9" (PSW)
		if (rp == 0b11)	assert(strcmp(operands[0], VALID_REGS[9]) == 0);
		// If this fails, then the assumption is wrong

		opcode |= (rp << 5);
	} else if (strcmp(instr, "pop") == 0) {
		opcode = 0b11000001;

		rp = getRP(operands[0], symTable);

		// See comment for "push"
		assert(strcmp(operands[0], VALID_REGS[9]) == 0);

		opcode |= (rp << 5);
	} else if (strcmp(instr, "dad") == 0) {
		opcode = 0b00001001;

		rp = getRP(operands[0], symTable);

		// See comment for "push", but instead of psw, it's sp
		if (rp == 0b11)	assert(strcmp(operands[0], VALID_REGS[8]) == 0);

		opcode |= (rp << 5);
	} else if (strcmp(instr, "inx") == 0) {
		opcode = 0b00000011;

		rp = getRP(operands[0], symTable);

		// see comment on "dad"
		if (rp == 0b11)	assert(strcmp(operands[0], VALID_REGS[8]) == 0);

		opcode |= (rp << 5);
	} else if (strcmp(instr, "dcx") == 0) {
		opcode = 0b00001011;

		rp = getRP(operands[0], symTable);

		// see comment on "dad"
		if (rp == 0b11)	assert(strcmp(operands[0], VALID_REGS[8]) == 0);

		opcode |= (rp << 5);
	} else if (strcmp(instr, "xchg") == 0) {
		opcode = 0b11101011;
		assert(operands[0] == NULL);
	} else if (strcmp(instr, "xthl") == 0) {
		opcode = 0b11100011;
		assert(operands[0] == NULL);
	} else if (strcmp(instr, "sphl") == 0) {
		opcode = 0b11111001;
		assert(operands[0] == NULL);
	} else if (strcmp(instr, "mvi") == 0) {
		opcode = 0b00000110;
		useDatabyte0 = true;

		reg = getReg(operands[0], symTable);

		opcode |= (reg << 3);

		imm = getImm(operands[1], symTable);

		databyte0 = imm & 0x00FF;
	} else if (strcmp(instr, "adi") == 0) {
		opcode = 0b11000110;
		useDatabyte0 = true;

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
	} else if (strcmp(instr, "aci") == 0) {
		opcode = 0b11001110;
		useDatabyte0 = true;

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
	} else if (strcmp(instr, "sui") == 0) {
		opcode = 0b11010110;
		useDatabyte0 = true;

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
	} else if (strcmp(instr, "sbi") == 0) {
		opcode = 0b11011110;
		useDatabyte0 = true;

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
	} else if (strcmp(instr, "ani") == 0) {
		opcode = 0b11100110;
		useDatabyte0 = true;

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
	} else if (strcmp(instr, "xri") == 0) {
		opcode = 0b11101110;
		useDatabyte0 = true;

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
	} else if (strcmp(instr, "ori") == 0) {
		opcode = 0b11110110;
		useDatabyte0 = true;

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
	} else if (strcmp(instr, "cpi") == 0) {
		opcode = 0b11111110;
		useDatabyte0 = true;

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
	} else if (strcmp(instr, "sta") == 0) {
		opcode = 0b00110010;
		useDatabyte0 = true;
		useDatabyte1 = true;

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
		databyte1 = imm >> 8;
	} else if (strcmp(instr, "lda") == 0) {
		opcode = 0b00111010;
		useDatabyte0 = true;
		useDatabyte1 = true;

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
		databyte1 = imm >> 8;
	} else if (strcmp(instr, "shld") == 0) {
		opcode = 0b00100010;
		useDatabyte0 = true;
		useDatabyte1 = true;

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
		databyte1 = imm >> 8;
	} else if (strcmp(instr, "lhld") == 0) {
		opcode = 0b00101010;
		useDatabyte0 = true;
		useDatabyte1 = true;

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
		databyte1 = imm >> 8;
	} else if (strcmp(instr, "pchl") == 0) {
		opcode = 0b11101001;
		assert(operands[0] == NULL);
	} else if (strcmp(instr, "rst") == 0) {
		opcode = 0b11000111;

		imm = getImm(operands[0], symTable);

		// According to the data sheet, the expression should eval to 0b000 to 0b111
		// This should have been checked prior but will still assert
		assert(imm >= 0b000 && imm <= 0b111);

		opcode |= (imm << 3);
	} else if (strncmp(instr, "j", 1) == 0) { // all jump instructions
		useDatabyte0 = true;
		useDatabyte1 = true;

		if (strcmp(instr+1, "mp") == 0) opcode = 0b11000011; // normal "jmp"
		else opcode = 0b11000010; // cond jumps

		opcode |= translateCC(instr+1);

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
		databyte1 = imm >> 8;
	} else if (strncmp(instr, "c", 1) == 0) { // all call instructions
		useDatabyte0 = true;
		useDatabyte1 = true;

		if (strcmp(instr+1, "all") == 0) opcode = 0b11001101; // normal "call"
		else opcode = 0b11000100; // cond calls

		opcode |= translateCC(instr+1);

		imm = getImm(operands[0], symTable);

		databyte0 = imm & 0x00FF;
		databyte1 = imm >> 8;
	} else if (strncmp(instr, "r", 1) == 0) { // all return instructions
		assert(operands[0] == NULL);
		if (strcmp(instr+1, "et") == 0) opcode = 0b11001001; // normal "ret"
		else opcode = 0b11000000; // cond rets

		opcode |= translateCC(instr+1);
	} else if (strcmp(instr, "hlt") == 0) {
		opcode = 0b01110110;
		assert(operands[0] == NULL);
	} else { } // nop is 0b00000000, default

	// printf("Single reg:0x%x; Destination reg:0x%x; Source reg:0x%x; Reg pair:0x%x; Imm:0x%x\n", reg, dst, src, rp, imm);

	insn->opcode = opcode;
	insn->databyte0 = databyte0;
	insn->databyte1 = databyte1;
	insn->useDatabyte0 = useDatabyte0;
	insn->useDatabyte1 = useDatabyte1;
}

aef_bin_img* translateGenerate(SymbolTable* symTable, src_obj_list_t* srcObjs) {
	// printf("Translating and generating for %d lines\n", srcObjs->count);

	aef_bin_img* binImg = (aef_bin_img*) malloc(sizeof(aef_bin_img));
	if (!binImg) handleError(ERR_MEM, FATAL, "Could not allocate space for binary image!\n");

	binImg->header.ident[AI_MAGIC0] = AEF_MAGIC0;
	binImg->header.ident[AI_MAGIC1] = AEF_MAGIC1;
	binImg->header.ident[AI_MAGIC2] = AEF_MAGIC2;
	binImg->header.ident[AI_MAGIC3] = AEF_MAGIC3;
	binImg->header.entry = 0x00;

	uint16_t* size = &binImg->size;
	uint16_t memAddr = 0x0;

	// If code+data is greater than 30KB, still write but issue a warning
	// that it exceeds default limit for code+data segment

	// [assigned-limit-exceeded; mem-limit-exceeded]
	byte limits = 0x00;
	bool halt = false;

	struct insnbytes insn = {0x0, 0x0, 0x0, false, false};

	for (int i = 0; i < srcObjs->count; i++) {
		src_obj_t* srcObj = srcObjs->arr[i];

		// Make sure the current srcObj to work with is an instruction/pseudo/directive
		// Aka it skips label only lines
		if (!srcObj->instr) continue;

		// printf("Instruction %s::\n", srcObj->instr);

		if (strcmp(srcObj->instr, VALID_PSEUDO[1]) == 0 || strcmp(srcObj->instr, VALID_PSEUDO[2]) == 0) continue;

		// Instructions can be up to 3 bytes long
		// opcode byte (1) is LSB
		// Byte 2 (data/addr[LSB]) is 2nd LSB/MSB
		// Byte 3 (data/addr[MSB]) is MSB

		bool dirc = applyDirectives(srcObj->instr, srcObj->operands, binImg, &memAddr, size, &limits, &halt);
		if (MEM_EXCEEDED(limits) == 0x1 || halt) break;
		if (dirc) continue;
		
		encode(symTable, srcObj->instr, srcObj->operands, &insn, &memAddr);

		// printf("Encoding is: opcode::0x%x  databyte0::0x%x  databyte1:0x%x\n", insn.opcode, insn.databyte0, insn.databyte1);

		memAddr = writeByte(binImg, memAddr, insn.opcode, &limits, size);
		// printf("New memAddr to write: 0x%x\n", memAddr);
		if (MEM_EXCEEDED(limits) == 0x1) break;

		if (insn.useDatabyte0) memAddr = writeByte(binImg, memAddr, insn.databyte0, &limits, size);
		if (MEM_EXCEEDED(limits) == 0x1) break;

		if (insn.useDatabyte1) memAddr = writeByte(binImg, memAddr, insn.databyte1, &limits, size);
		if (MEM_EXCEEDED(limits) == 0x1) break;
	}

	if (ASSIGNED_EXCEEDED(limits) == 0x1) {
		handleError(ERR_MEM, WARNING, "Code and data exceed the default limit. Make sure to increase limit before attempting to run!\n");
	} else if (MEM_EXCEEDED(limits) == 0x1) {
		handleError(ERR_MEM, WARNING, "Code and data exceed total memory limit. Stopped assembling at limit!\n");
	}

	return binImg;
}