#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "includes.h"
#include "cpu_context.h"
#include "instruction.h"

#define MEMORY_SIZE 65536
#define TERM_DATA_IN_ADDR 0xFF00
#define TERM_DATA_OUT_ADDR 0xFF02
#define TIMER_CFG_ADDR 0xFF10

#define PROGRAM_IVT_ENTRY 0
#define INVALID_INSTRUCTION_IVT_ENTRY 1
#define TIMER_IVT_ENTRY 2
#define TERMINAL_IVT_ENTRY 3

#define MEM_READ_DIR(addr, type) *(type*)(Emulator::memory + addr)
#define MEM_WRITE_DIR(addr, type, val) *(type*)(Emulator::memory + addr) = val

#define TIMER_INIT_NDX 0
#define TIMER_MAX_NDX 7

class InvalidInsException {};

const int timer_values[] = {
    500,
    1000,
    1500,
    2000,
    5000,
    10000,
    30000,
    60000
};

class Emulator {
public:
    static void run();
    static void initialize();
    static void load_data(vector<pair<Addr, vector<Byte>>> data_vector);

    static Byte memory[MEMORY_SIZE];
private:
    static CPU_Context cpu_context;

    static inline DecodedInsDescr read_ins();
    static inline Addr read_op_addr(Addr** addr);
    static inline Word read_op_val_word(Word** addr);
    static inline Byte read_op_val_byte(Byte** addr);
    static inline void check_terminal(void* src, void* dst);
};

#endif