#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "../../include/macro.h"
int main()
{
	uint32_t IMM_MASK = ~(uint32_t)BITMASK(11);
	uint32_t REG_MASK = (uint32_t)BITMASK(11) & (~BITMASK(6));
	uint32_t opcode = 0x13;//li
	uint32_t inst = 0x0;
	uint32_t imm, reg;
	uint32_t IMM_START = 12, OP_LEN = 7;
	for (uint32_t i = 0; i < pow(2, 20); ++i) {
		inst = 0x0;
		imm = i << IMM_START;
		for (uint32_t j = 1; j < 32; ++j) {
			reg = j << OP_LEN;

			inst = 0x0 | opcode | reg | imm;
			printf("0x%08x\n",  inst);
		}
	}

	return 0;
}
