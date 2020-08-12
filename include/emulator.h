#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "includes.h"
#include "cpu_context.h"
#include "instruction.h"

#define MEMORY_SIZE 65536

#define MEM_READ_DIR(addr, type) *(type*)(Emulator::memory + addr)
#define MEM_READ_IND(addr, type) *(type*)(Emulator::memory + *((Addr*)(Emulator::memory + addr)))
#define MEM_WRITE_DIR(addr, type, val) *(type*)(Emulator::memory + addr) = val
#define MEM_WRITE_IND(addr, type, val) *(type*)(Emulator::memory + *((Addr*)(Emulator::memory + addr))) = val

class Emulator {
public:
    static void run();
    static void initialize();
    static void load_data(vector<pair<Addr, vector<Byte>>> data_vector);

private:
    static CPU_Context cpu_context;
    static Byte memory[MEMORY_SIZE];

    static DecodedInsDescr read_ins();
    static Addr read_op_addr();
    static Word read_op_val_word();
    static Byte read_op_val_byte();
    static Word* read_op_ptr_word();
    static Byte* read_op_ptr_byte();
};

#endif