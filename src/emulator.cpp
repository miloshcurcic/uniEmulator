#include "emulator.h"
#include "eml_exception.h"
#include "terminal.h"
#include <unistd.h>
#include <chrono>

CPU_Context Emulator::cpu_context;
Byte Emulator::memory[MEMORY_SIZE];

void Emulator::check_terminal(void* src, void* dst) {
    // Check if anything was outputted to terminal
    if (dst == (Emulator::memory + TERM_DATA_OUT_ADDR)) {
        Terminal::write_output();
    }

    // Check if there was input to terminal
    if (src == (Emulator::memory + TERM_DATA_IN_ADDR)) {
        Terminal::continue_input();
    }
}

void Emulator::load_data(vector<pair<Addr, vector<Byte>>> data_vector) {
    for(auto& data : data_vector) {
        memcpy((char*)(memory + data.first), data.second.data(), data.second.size());
    }
}

void Emulator::initialize() {
    cpu_context.regs[PC_REG_NDX].addr = MEM_READ_DIR(PROGRAM_IVT_ENTRY * 2, Addr);
}

DecodedInsDescr Emulator::read_ins() {
    Byte instr_descr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Byte);
    cpu_context.regs[PC_REG_NDX].word++;
    
    return {(Operation)INS_OP_CODE(instr_descr), (OpBytes)INS_SIZE(instr_descr)};
}

Word Emulator::read_op_val_word(Word** addr) {
    Byte op_descr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Byte);
    
    cpu_context.regs[PC_REG_NDX].word++;
    
    switch (OP_ADDRESSING(op_descr)) {
        case AddressingMode::AM_IMMED: {
            Word res = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Word);
            cpu_context.regs[PC_REG_NDX].word += 2;
            
            *addr = nullptr;
            return res;
        }
        case AddressingMode::AM_REGDIR: {
            if (OP_REG(op_descr) == Register::R_PSW) {
                *addr = &cpu_context.psw.word;
            } else {
                *addr = &cpu_context.regs[OP_REG(op_descr)].word;
            }
            return **addr;
        }
        case AddressingMode::AM_REGIND: {
            if (OP_REG(op_descr) == Register::R_PSW) {
                throw InvalidInsException();
            }

            *addr = &MEM_READ_DIR(cpu_context.regs[OP_REG(op_descr)].addr, Word);
            return **addr;
        }
        case AddressingMode::AM_BASEREG: {
            if (OP_REG(op_descr) == Register::R_PSW) {
                throw InvalidInsException();
            }

            Offs offs = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Offs);
            cpu_context.regs[PC_REG_NDX].word += 2;
            
            *addr = &MEM_READ_DIR(cpu_context.regs[OP_REG(op_descr)].word + offs, Word);
            return **addr;
        }
        case AddressingMode::AM_MEMDIR: {
            Addr ad = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Addr);
            cpu_context.regs[PC_REG_NDX].word += 2;

            *addr = &MEM_READ_DIR(ad, Word);
            return **addr;
        }
    }

    throw InvalidInsException();
}

Byte Emulator::read_op_val_byte(Byte** addr) {
    Byte op_descr = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Byte);
    cpu_context.regs[PC_REG_NDX].word++;
    
    switch (OP_ADDRESSING(op_descr)) {
        case AddressingMode::AM_IMMED: {
            Word res = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Byte);
            cpu_context.regs[PC_REG_NDX].word += 1;
            
            *addr = nullptr;
            return res;
        }
        case AddressingMode::AM_REGDIR: {
            if (OP_REG(op_descr) == Register::R_PSW) {
                *addr = &cpu_context.psw.half[OP_REG_LH(op_descr)];
            } else {
                *addr = &cpu_context.regs[OP_REG(op_descr)].half[OP_REG_LH(op_descr)];
            }
            
            return **addr;
        }
        case AddressingMode::AM_REGIND: {
            if (OP_REG(op_descr) == Register::R_PSW) {
                throw InvalidInsException();
            }

            *addr = &MEM_READ_DIR(cpu_context.regs[OP_REG(op_descr)].addr, Byte);
            return **addr;
        }
        case AddressingMode::AM_BASEREG: {
            if (OP_REG(op_descr) == Register::R_PSW) {
                throw InvalidInsException();
            }

            Offs offs = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Offs);
            cpu_context.regs[PC_REG_NDX].word += 2;

            *addr = &MEM_READ_DIR(cpu_context.regs[OP_REG(op_descr)].word + offs, Byte);
            return **addr;
        }
        case AddressingMode::AM_MEMDIR: {
            Addr ad = MEM_READ_DIR(cpu_context.regs[PC_REG_NDX].word, Addr);
            cpu_context.regs[PC_REG_NDX].word += 2;
            
            *addr = &MEM_READ_DIR(ad, Byte);
            return **addr;
        }
    }

    throw InvalidInsException();
}

void Emulator::run() {
    bool running = true;
    long double timer_val = timer_values[TIMER_INIT_NDX];
    
    auto exec_start = chrono::high_resolution_clock::now();
    
    while (running) {
        void *src_addr = nullptr;
        void *dst_addr = nullptr;
        bool invalid_instruction = false;

        try {
            DecodedInsDescr ins_descr = read_ins();
            
            switch (ins_descr.operation) {
                case Operation::OP_HALT: {
                    running = false;
                    break;
                }
                case Operation::OP_IRET: {
                    cpu_context.psw.word = MEM_READ_DIR(cpu_context.regs[SP_REG_NDX].addr, Word);
                    cpu_context.regs[SP_REG_NDX].addr += 2;

                    cpu_context.regs[PC_REG_NDX].addr = MEM_READ_DIR(cpu_context.regs[SP_REG_NDX].addr, Addr);
                    cpu_context.regs[SP_REG_NDX].addr += 2;

                    break;
                }
                case Operation::OP_RET: {
                    cpu_context.regs[PC_REG_NDX].addr = MEM_READ_DIR(cpu_context.regs[SP_REG_NDX].addr, Addr);
                    cpu_context.regs[SP_REG_NDX].addr += 2;

                    break;
                }
                case Operation::OP_INT: {
                    auto dst = read_op_val_word((Word**)&dst_addr);

                    cpu_context.regs[SP_REG_NDX].addr -= 2;
                    MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Addr, cpu_context.regs[PC_REG_NDX].addr);

                    cpu_context.regs[SP_REG_NDX].addr -= 2;
                    MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Word, cpu_context.psw.word);

                    cpu_context.regs[PC_REG_NDX].addr = MEM_READ_DIR((dst % 8)*2, Addr);
                    cpu_context.psw.word &= ~I_FLAG_MASK;
                    break;
                }
                case Operation::OP_CALL: {
                    auto dst = read_op_val_word((Word**)&dst_addr);

                    cpu_context.regs[SP_REG_NDX].addr -= 2;
                    MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Addr, cpu_context.regs[PC_REG_NDX].addr);

                    cpu_context.regs[PC_REG_NDX].addr = dst;
                    break;
                }
                case Operation::OP_JMP: {
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    cpu_context.regs[PC_REG_NDX].addr = dst;

                    break;
                }
                case Operation::OP_JEQ: {
                    auto dst = read_op_val_word((Word**)&dst_addr);

                    if (Z_FLAG) {
                        cpu_context.regs[PC_REG_NDX].addr = dst;
                    }
                    break;
                }
                case Operation::OP_JNE: {
                    auto dst = read_op_val_word((Word**)&dst_addr);

                    if (!Z_FLAG) {
                        cpu_context.regs[PC_REG_NDX].addr = dst;
                    }
                    break;
                }
                case Operation::OP_JGT: {
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    
                    if (~(N_FLAG^O_FLAG) & ~Z_FLAG & 1) {
                        cpu_context.regs[PC_REG_NDX].addr = dst;
                    }
                    break;
                }
                case Operation::OP_PUSH: {
                    auto dst = read_op_val_word((Word**)&dst_addr);

                    cpu_context.regs[SP_REG_NDX].addr -= 2;
                    MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Word, dst);
                    break;
                }
                case Operation::OP_POP: {
                    read_op_val_word((Word**)&dst_addr);

                    if (dst_addr == nullptr) {
                        throw InvalidInsException();
                    }

                    *(Word*)dst_addr = MEM_READ_DIR(cpu_context.regs[SP_REG_NDX].addr, Word);
                    cpu_context.regs[SP_REG_NDX].addr += 2;
                    break;
                }
                case Operation::OP_XCHG: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        auto dst = read_op_val_byte((Byte**)&dst_addr);

                        if (src_addr == nullptr || dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Byte*)dst_addr = src;
                        *(Byte*)src_addr = dst;
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        auto dst = read_op_val_word((Word**)&dst_addr);

                        if (src_addr == nullptr || dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Word*)dst_addr = src;
                        *(Word*)src_addr = dst;
                    }

                    break;
                }
                case Operation::OP_MOV: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        read_op_val_byte((Byte**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Byte*)dst_addr = src;

                        FILL_N_FLAG_BYTE(*(Byte*)dst_addr);
                        FILL_Z_FLAG(*(Byte*)dst_addr);
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        read_op_val_word((Word**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Word*)dst_addr = src;

                        FILL_N_FLAG_WORD(*(Word*)dst_addr);
                        FILL_Z_FLAG(*(Word*)dst_addr);
                    }

                    break;
                }
                case Operation::OP_ADD: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        auto dst = read_op_val_byte((Byte**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        Byte res = src + dst;

                        if (res < src) {
                            SET_O_FLAG;
                        } else {
                            CLR_O_FLAG;
                        }

                        if (((src ^ dst ^ 0x80) & (res ^ src)) & 0x80) {
                            SET_C_FLAG;
                        } else {
                            CLR_C_FLAG;
                        }

                        *(Byte*)dst_addr = res;

                        FILL_N_FLAG_BYTE(*(Byte*)dst_addr);
                        FILL_Z_FLAG(*(Byte*)dst_addr);
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        auto dst = read_op_val_word((Word**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        Word res = src + dst;

                        if (res < src) {
                            SET_O_FLAG;
                        } else {
                            CLR_O_FLAG;
                        }

                        if (((src ^ dst ^ 0x8000) & (res ^ src)) & 0x8000) {
                            SET_C_FLAG;
                        } else {
                            CLR_C_FLAG;
                        }

                        *(Word*)dst_addr = res;

                        FILL_N_FLAG_WORD(*(Word*)dst_addr);
                        FILL_Z_FLAG(*(Word*)dst_addr);
                    }

                    break;
                }
                case Operation::OP_SUB: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        auto dst = read_op_val_byte((Byte**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        Byte res = dst - src;

                        if ((src ^ dst) & (dst ^ res) & 0x80) {
                            SET_O_FLAG;
                        } else {
                            CLR_O_FLAG;
                        }

                        if (dst < src) {
                            SET_C_FLAG;
                        } else {
                            CLR_C_FLAG;
                        }

                        *(Byte*)dst_addr = res;

                        FILL_N_FLAG_BYTE(*(Byte*)dst_addr);
                        FILL_Z_FLAG(*(Byte*)dst_addr);
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        auto dst = read_op_val_word((Word**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        Word res = dst - src;

                        if ((src ^ dst) & (dst ^ res) & 0x8000) {
                            SET_O_FLAG;
                        } else {
                            CLR_O_FLAG;
                        }

                        if (dst < src) {
                            SET_C_FLAG;
                        } else {
                            CLR_C_FLAG;
                        }

                        *(Word*)dst_addr = res;

                        FILL_N_FLAG_WORD(*(Word*)dst_addr);
                        FILL_Z_FLAG(*(Word*)dst_addr);
                    }

                    break;
                }
                case Operation::OP_MUL: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        read_op_val_byte((Byte**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Byte*)dst_addr *= src;

                        FILL_N_FLAG_BYTE(*(Byte*)dst_addr);
                        FILL_Z_FLAG(*(Byte*)dst_addr);
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        read_op_val_word((Word**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Word*)dst_addr *= src;

                        FILL_N_FLAG_WORD(*(Word*)dst_addr);
                        FILL_Z_FLAG(*(Word*)dst_addr);
                    }

                    break;
                }
                case Operation::OP_DIV: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        read_op_val_byte((Byte**)&dst_addr);

                        if (dst_addr == nullptr || src == 0) {
                            throw InvalidInsException();
                        }

                        *(Byte*)dst_addr /= src;

                        FILL_N_FLAG_BYTE(*(Byte*)dst_addr);
                        FILL_Z_FLAG(*(Byte*)dst_addr);
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        read_op_val_word((Word**)&dst_addr);

                        if (dst_addr == nullptr || src == 0) {
                            throw InvalidInsException();
                        }

                        *(Word*)dst_addr /= src;

                        FILL_N_FLAG_WORD(*(Word*)dst_addr);
                        FILL_Z_FLAG(*(Word*)dst_addr);
                    }

                    break;
                }
                case Operation::OP_CMP: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        auto dst = read_op_val_byte((Byte**)&dst_addr);

                        Byte temp = dst - src;

                        dst_addr = nullptr;

                        if ((src ^ dst) & (dst ^ temp) & 0x80) {
                            SET_O_FLAG;
                        } else {
                            CLR_O_FLAG;
                        }

                        if (dst < src) {
                            SET_C_FLAG;
                        } else {
                            CLR_C_FLAG;
                        }

                        FILL_N_FLAG_BYTE(temp);
                        FILL_Z_FLAG(temp);
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        auto dst = read_op_val_word((Word**)&dst_addr);

                        Word temp = dst - src;

                        if ((src ^ dst) & (dst ^ temp) & 0x8000) {
                            SET_O_FLAG;
                        } else {
                            CLR_O_FLAG;
                        }

                        if (dst < src) {
                            SET_C_FLAG;
                        } else {
                            CLR_C_FLAG;
                        }

                        FILL_N_FLAG_WORD(temp);
                        FILL_Z_FLAG(temp);
                    }

                    break;
                }
                case Operation::OP_NOT: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        read_op_val_byte((Byte**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Byte*)dst_addr = ~src;

                        FILL_N_FLAG_BYTE(*(Byte*)dst_addr);
                        FILL_Z_FLAG(*(Byte*)dst_addr);
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        read_op_val_word((Word**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Word*)dst_addr = ~src;

                        FILL_N_FLAG_WORD(*(Word*)dst_addr);
                        FILL_Z_FLAG(*(Word*)dst_addr);
                    }

                    break;
                }
                case Operation::OP_AND: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        read_op_val_byte((Byte**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Byte*)dst_addr &= src;

                        FILL_N_FLAG_BYTE(*(Byte*)dst_addr);
                        FILL_Z_FLAG(*(Byte*)dst_addr);
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        read_op_val_word((Word**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Word*)dst_addr &= src;

                        FILL_N_FLAG_WORD(*(Word*)dst_addr);
                        FILL_Z_FLAG(*(Word*)dst_addr);
                    }

                    break;
                }
                case Operation::OP_OR: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        read_op_val_byte((Byte**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Byte*)dst_addr |= src;

                        FILL_N_FLAG_BYTE(*(Byte*)dst_addr);
                        FILL_Z_FLAG(*(Byte*)dst_addr);
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        read_op_val_word((Word**)&dst_addr);
                        
                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Word*)dst_addr |= src;

                        FILL_N_FLAG_WORD(*(Word*)dst_addr);
                        FILL_Z_FLAG(*(Word*)dst_addr);
                    }

                    break;
                }
                case Operation::OP_XOR: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        read_op_val_byte((Byte**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Byte*)dst_addr ^= src;

                        FILL_N_FLAG_BYTE(*(Byte*)dst_addr);
                        FILL_Z_FLAG(*(Byte*)dst_addr);
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        read_op_val_word((Word**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        *(Word*)dst_addr ^= src;

                        FILL_N_FLAG_WORD(*(Word*)dst_addr);
                        FILL_Z_FLAG(*(Word*)dst_addr);
                    }

                    break;
                }
                case Operation::OP_TEST: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        auto dst = read_op_val_byte((Byte**)&dst_addr);

                        auto temp = dst & src;

                        dst_addr = nullptr;

                        FILL_N_FLAG_BYTE(temp);
                        FILL_Z_FLAG(temp);
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        auto dst = read_op_val_word((Word**)&dst_addr);

                        auto temp = dst & src;

                        dst_addr = nullptr;

                        FILL_N_FLAG_WORD(temp);
                        FILL_Z_FLAG(temp);
                    }

                    break;
                }
                case Operation::OP_SHL: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto src = read_op_val_byte((Byte**)&src_addr);
                        auto dst = read_op_val_byte((Byte**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        if ((src < 8) && ((dst >> (8-src)) & 1)) {
                            SET_C_FLAG;
                        } else {
                            CLR_C_FLAG;
                        }

                        *(Byte*)dst_addr <<= src;

                        FILL_N_FLAG_BYTE(*(Byte*)dst_addr);
                        FILL_Z_FLAG(*(Byte*)dst_addr);
                    } else {
                        auto src = read_op_val_word((Word**)&src_addr);
                        auto dst = read_op_val_word((Word**)&dst_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        if ((src < 16) && ((dst >> (16-src)) & 1)) {
                            SET_C_FLAG;
                        } else {
                            CLR_C_FLAG;
                        }

                        *(Word*)dst_addr <<= src;

                        FILL_N_FLAG_WORD(*(Word*)dst_addr);
                        FILL_Z_FLAG(*(Word*)dst_addr);
                    }

                    break;
                }
                case Operation::OP_SHR: {
                    if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                        auto dst = read_op_val_byte((Byte**)&dst_addr);
                        auto src = read_op_val_byte((Byte**)&src_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        if ((dst >> (src - 1)) & 1) {
                            SET_C_FLAG;
                        } else {
                            CLR_C_FLAG;
                        }

                        *(Byte*)dst_addr >>= src;

                        FILL_N_FLAG_BYTE(*(Byte*)dst_addr);
                        FILL_Z_FLAG(*(Byte*)dst_addr);
                    } else {
                        auto dst = read_op_val_word((Word**)&dst_addr);
                        auto src = read_op_val_word((Word**)&src_addr);

                        if (dst_addr == nullptr) {
                            throw InvalidInsException();
                        }

                        if ((dst >> (src - 1)) & 1) {
                            SET_C_FLAG;
                        } else {
                            CLR_C_FLAG;
                        }

                        *(Word*)dst_addr >>= src;

                        FILL_N_FLAG_WORD(*(Word*)dst_addr);
                        FILL_Z_FLAG(*(Word*)dst_addr);
                    }

                    break;
                }
                default: {
                    throw InvalidInsException();
                }
            }
        } catch (InvalidInsException e) {
            invalid_instruction = true;
        }

        check_terminal(src_addr, dst_addr);

        if (invalid_instruction) {
                cpu_context.regs[SP_REG_NDX].addr -= 2;
                MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Addr, cpu_context.regs[PC_REG_NDX].addr);

                cpu_context.regs[SP_REG_NDX].addr -= 2;
                MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Word, cpu_context.psw.word);
            
                cpu_context.regs[PC_REG_NDX].addr = MEM_READ_DIR(INVALID_INSTRUCTION_IVT_ENTRY * 2, Addr);
                CLR_I_FLAG;

                continue;
        }

        auto exec_end = chrono::high_resolution_clock::now();
        timer_val -= ((long double)chrono::duration_cast<chrono::nanoseconds>(exec_end - exec_start).count()) / 1000000;
        exec_start = chrono::high_resolution_clock::now();

        if (I_FLAG == 0) {
            if (timer_val <= 0 && TR_FLAG == 0) {
                auto timer_cfg = MEM_READ_DIR(TIMER_CFG_ADDR, Word);
                if (timer_cfg < TIMER_INIT_NDX || timer_cfg > TIMER_MAX_NDX) {
                    throw EmulatorException(ERR_TIMER_MISCONFIGURATION);
                }
                timer_val = timer_values[timer_cfg];

                cpu_context.regs[SP_REG_NDX].addr -= 2;
                MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Addr, cpu_context.regs[PC_REG_NDX].addr);

                cpu_context.regs[SP_REG_NDX].addr -= 2;
                MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Word, cpu_context.psw.word);
            
                cpu_context.regs[PC_REG_NDX].addr = MEM_READ_DIR(TIMER_IVT_ENTRY * 2, Addr);
                CLR_I_FLAG;
            } else if (Terminal::input_interrupt && TL_FLAG == 0) {
                Terminal::input_interrupt = false;

                cpu_context.regs[SP_REG_NDX].addr -= 2;
                MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Addr, cpu_context.regs[PC_REG_NDX].addr);

                cpu_context.regs[SP_REG_NDX].addr -= 2;
                MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Word, cpu_context.psw.word);
            
                cpu_context.regs[PC_REG_NDX].addr = MEM_READ_DIR(TERMINAL_IVT_ENTRY * 2, Addr);
                CLR_I_FLAG;
            }
        }
    }
}