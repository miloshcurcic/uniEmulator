#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_

#include "includes.h"
#include "memory.h"
#include "cpu_context.h"

#define INS_SIZE_OFFSET 2
#define INS_OPERATION_CODE_OFFSET 3
#define OP_REGISTER_BYTE_OFFSET 0
#define OP_REGISTER_OFFSET 1
#define OP_ADDRESSING_OFFSET 5
#define INS_SIZE_BITS 1
#define INS_OPERATION_CODE_BITS 5
#define OP_REGISTER_BYTE_BITS 1
#define OP_REGISTER_BITS 4
#define OP_ADDRESSING_BITS 3

#define INS_OP_CODE(descr) (descr >> INS_OPERATION_CODE_OFFSET)
#define INS_SIZE(descr) ((descr >> INS_SIZE_OFFSET) & 0x1)
#define OP_ADDRESSING(descr) (descr >> OP_ADDRESSING_OFFSET)
#define OP_REG(descr) ((descr >> OP_REGISTER_OFFSET) & 0xf)
#define OP_REG_LH(descr) (descr & 0x1)

enum Operation : Byte {
    OP_HALT,
    OP_IRET,
    OP_RET,
    OP_INT,
    OP_CALL,
    OP_JMP,
    OP_JEQ,
    OP_JNE,
    OP_JGT,
    OP_PUSH,
    OP_POP,
    OP_XCHG,
    OP_MOV,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_CMP,
    OP_NOT,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_TEST,
    OP_SHL,
    OP_SHR
};

enum OpBytes : Byte {
    OB_ONE,
    OB_TWO
};

enum AddressingMode : Byte {
    AM_IMMED,
    AM_REGDIR,
    AM_REGIND,
    AM_BASEREG,
    AM_MEMDIR
};

enum Register : Byte {
    R_0,
    R_1,
    R_2,
    R_3,
    R_4,
    R_5,
    R_6,
    R_7,
    R_PSW = 0xF
};

enum RegisterByte : Byte {
    RB_LOW,
    RB_HIGH
};

struct DecodedInsDescr {
    Operation operation;
    OpBytes op_bytes;
};

struct CPU_Context;


static inline DecodedInsDescr read_ins() {
    Byte instr_descr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Byte);
    cpu_context.regs[PC_REG_NDX].word++;

    return {(Operation)INS_OP_CODE(instr_descr), (OpBytes)INS_SIZE(instr_descr)};
}

static inline Addr read_op_addr() {
    Byte op_descr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Byte);
    cpu_context.regs[PC_REG_NDX].word++;

    switch (OP_ADDRESSING(op_descr)) {
        case AddressingMode::AM_IMMED: {
            Addr res = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Addr);
            cpu_context.regs[PC_REG_NDX].word += 2;

            return res;
        }
        case AddressingMode::AM_REGDIR: {
            return (Addr)cpu_context.regs[OP_REG(op_descr)].word;
        }
        case AddressingMode::AM_REGIND: {
            return MEM_READ_DIR(OP_REG(op_descr), Addr);
        }
        case AddressingMode::AM_BASEREG: {
            Offs offs = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Offs);
            cpu_context.regs[PC_REG_NDX].word += 2;

            return MEM_READ_DIR(cpu_context.regs[OP_REG(op_descr)].word + offs, Addr);
        }
        case AddressingMode::AM_MEMDIR: {
            Addr addr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Addr);
            cpu_context.regs[PC_REG_NDX].word += 2;

            return MEM_READ_DIR(addr, Addr);
        }
    }
}

static inline Word read_op_val_word() {
    Byte op_descr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Byte);
    cpu_context.regs[PC_REG_NDX].word++;

    switch (OP_ADDRESSING(op_descr)) {
        case AddressingMode::AM_IMMED: {
            Word res = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Word);
            cpu_context.regs[PC_REG_NDX].word += 2;

            return res;
        }
        case AddressingMode::AM_REGDIR: {
            return cpu_context.regs[OP_REG(op_descr)].word;
        }
        case AddressingMode::AM_REGIND: {
            return MEM_READ_DIR(OP_REG(op_descr), Word);
        }
        case AddressingMode::AM_BASEREG: {
            Offs offs = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Offs);
            cpu_context.regs[PC_REG_NDX].word += 2;

            return MEM_READ_DIR(cpu_context.regs[OP_REG(op_descr)].word + offs, Word);
        }
        case AddressingMode::AM_MEMDIR: {
            Addr addr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Addr);
            cpu_context.regs[PC_REG_NDX].word += 2;

            return MEM_READ_DIR(addr, Word);
        }
    }
}

static inline Byte read_op_val_byte() {
    Byte op_descr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Byte);
    cpu_context.regs[PC_REG_NDX].word++;

    switch (OP_ADDRESSING(op_descr)) {
        case AddressingMode::AM_IMMED: {
            Byte res = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Byte);
            cpu_context.regs[PC_REG_NDX].word += 1;

            return res;
        }
        case AddressingMode::AM_REGDIR: {
            return cpu_context.regs[OP_REG(op_descr)].half[OP_REG_LH(op_descr)];
        }
        case AddressingMode::AM_REGIND: {
            return MEM_READ_DIR(OP_REG(op_descr), Byte);
        }
        case AddressingMode::AM_BASEREG: {
            Offs offs = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Offs);
            cpu_context.regs[PC_REG_NDX].word += 2;

            return MEM_READ_DIR(cpu_context.regs[OP_REG(op_descr)].word + offs, Byte);
        }
        case AddressingMode::AM_MEMDIR: {
            Addr addr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Addr);
            cpu_context.regs[PC_REG_NDX].word += 2;

            return MEM_READ_DIR(addr, Byte);
        }
        default: {

        }
    }
}

static inline Word* read_op_ptr_word() {
    Byte op_descr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Byte);
    cpu_context.regs[PC_REG_NDX].word++;

    switch (OP_ADDRESSING(op_descr)) {
        case AddressingMode::AM_IMMED: {
            // Error
            return 0;
        }
        case AddressingMode::AM_REGDIR: {
            return &cpu_context.regs[OP_REG(op_descr)].word;
        }
        case AddressingMode::AM_REGIND: {
            return &MEM_READ_DIR(OP_REG(op_descr), Word);
        }
        case AddressingMode::AM_BASEREG: {
            Offs offs = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Offs);
            cpu_context.regs[PC_REG_NDX].word += 2;

            return &MEM_READ_DIR(cpu_context.regs[OP_REG(op_descr)].word + offs, Word);
        }
        case AddressingMode::AM_MEMDIR: {
            Addr addr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Addr);
            cpu_context.regs[PC_REG_NDX].word += 2;

            return &MEM_READ_DIR(addr, Word);
        }
        default: {

        }
    }
}

static inline Byte* read_op_ptr_byte() {
    Byte op_descr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Byte);
    cpu_context.regs[PC_REG_NDX].word++;

    switch (OP_ADDRESSING(op_descr)) {
        case AddressingMode::AM_IMMED: {
            // Error
            return 0;
        }
        case AddressingMode::AM_REGDIR: {
            return &cpu_context.regs[OP_REG(op_descr)].half[OP_REG_LH(op_descr)];
        }
        case AddressingMode::AM_REGIND: {
            return &MEM_READ_DIR(OP_REG(op_descr), Byte);
        }
        case AddressingMode::AM_BASEREG: {
            Offs offs = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Offs);
            cpu_context.regs[PC_REG_NDX].word += 2;

            return &MEM_READ_DIR(cpu_context.regs[OP_REG(op_descr)].word + offs, Byte);
        }
        case AddressingMode::AM_MEMDIR: {
            Addr addr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Addr);
            cpu_context.regs[PC_REG_NDX].word += 2;

            return &MEM_READ_DIR(addr, Byte);
        }
        default: {

        }
    }
}

#endif