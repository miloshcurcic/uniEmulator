#ifndef _CPU_CONTEXT_H_
#define _CPU_CONTEXT_H_

#include "includes.h"
#include "elf.h"

#define NUM_REGS 8
#define SP_REG_NDX 6
#define PC_REG_NDX 7
#define PSW_REG_NDX 15
#define REG_HIGH_NDX 1
#define REG_LOW_NDX 0

#define Z_FLAG_MASK 1
#define O_FLAG_MASK 2
#define C_FLAG_MASK 4
#define N_FLAG_MASK 8
#define TR_FLAG_MASK 8192
#define TL_FLAG_MASK 16384
#define I_FLAG_MASK 32768

#define Z_FLAG_OFFS 0
#define O_FLAG_OFFS 1
#define C_FLAG_OFFS 2
#define N_FLAG_OFFS 3
#define TR_FLAG_OFFS 13
#define TL_FLAG_OFFS 14
#define I_FLAG_OFFS 15

#define Z_FLAG ((Emulator::cpu_context.psw.word & Z_FLAG_MASK) >> Z_FLAG_OFFS)
#define O_FLAG ((Emulator::cpu_context.psw.word & O_FLAG_MASK) >> O_FLAG_OFFS)
#define C_FLAG ((Emulator::cpu_context.psw.word & C_FLAG_MASK) >> C_FLAG_OFFS)
#define N_FLAG ((Emulator::cpu_context.psw.word & N_FLAG_MASK) >> N_FLAG_OFFS)
#define TR_FLAG ((Emulator::cpu_context.psw.word & TR_FLAG_MASK) >> TR_FLAG_OFFS)
#define TL_FLAG ((Emulator::cpu_context.psw.word & TL_FLAG_MASK) >> TL_FLAG_OFFS)
#define I_FLAG ((Emulator::cpu_context.psw.word & I_FLAG_MASK) >> I_FLAG_OFFS)

#define SET_Z_FLAG Emulator::cpu_context.psw.word |= Z_FLAG_MASK
#define SET_O_FLAG Emulator::cpu_context.psw.word |= O_FLAG_MASK
#define SET_C_FLAG Emulator::cpu_context.psw.word |= C_FLAG_MASK
#define SET_N_FLAG Emulator::cpu_context.psw.word |= N_FLAG_MASK
#define SET_TR_FLAG Emulator::cpu_context.psw.word |= TR_FLAG_MASK
#define SET_TL_FLAG Emulator::cpu_context.psw.word |= TL_FLAG_MASK
#define SET_I_FLAG Emulator::cpu_context.psw.word |= I_FLAG_MASK

#define CLR_Z_FLAG Emulator::cpu_context.psw.word &= ~Z_FLAG_MASK
#define CLR_O_FLAG Emulator::cpu_context.psw.word &= ~O_FLAG_MASK
#define CLR_C_FLAG Emulator::cpu_context.psw.word &= ~C_FLAG_MASK
#define CLR_N_FLAG Emulator::cpu_context.psw.word &= ~N_FLAG_MASK
#define CLR_TR_FLAG Emulator::cpu_context.psw.word &= ~TR_FLAG_MASK
#define CLR_TL_FLAG Emulator::cpu_context.psw.word &= ~TL_FLAG_MASK
#define CLR_I_FLAG Emulator::cpu_context.psw.word &= ~I_FLAG_MASK

#define FILL_Z_FLAG(val) if ((val) == 0) { SET_Z_FLAG; } else { CLR_Z_FLAG; }
#define FILL_O_FLAG(op0, op1, op, res) 
#define FILL_C_FLAG(val)
#define FILL_N_FLAG_BYTE(val) if (val & 0x80) { SET_N_FLAG; } else { CLR_N_FLAG; }
#define FILL_N_FLAG_WORD(val) if (val & 0x8000) { SET_N_FLAG; } else { CLR_N_FLAG; }

union GenReg16 {
    Word word;
    Addr addr;
    Byte half[2];
};

struct CPU_Context {
    GenReg16 regs[NUM_REGS];
    GenReg16 psw;
};

#endif