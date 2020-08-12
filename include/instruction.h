#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_

#include "includes.h"
#include "memory.h"
#include "cpu_context.h"
#include "emulator.h"

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

#endif