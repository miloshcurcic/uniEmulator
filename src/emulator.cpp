#include "emulator.h"
#include "terminal.h"
#include <unistd.h>

CPU_Context Emulator::cpu_context;
Byte Emulator::memory[MEMORY_SIZE];

void Emulator::check_terminal(void* src, void* dst) {
    if (dst == (Emulator::memory + TERM_DATA_OUT_ADDR)) {
        Terminal::write_output();
    }
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
    cpu_context.regs[PC_REG_NDX].addr = MEM_READ_DIR(0, Addr);
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
            *addr = &cpu_context.regs[OP_REG(op_descr)].word;
            return **addr;
        }
        case AddressingMode::AM_REGIND: {
            *addr = &MEM_READ_DIR(OP_REG(op_descr), Word);
            return **addr;
        }
        case AddressingMode::AM_BASEREG: {
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
            *addr = &cpu_context.regs[OP_REG(op_descr)].half[OP_REG_LH(op_descr)];
            return **addr;
        }
        case AddressingMode::AM_REGIND: {
            *addr = &MEM_READ_DIR(OP_REG(op_descr), Byte);
            return **addr;
        }
        case AddressingMode::AM_BASEREG: {
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
}

void Emulator::run() {
    bool running = true;
    while (running) {
        DecodedInsDescr ins_descr = read_ins();
        void *src_addr = nullptr;
        void *dst_addr = nullptr;

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

                if (~(N_FLAG^O_FLAG) & ~Z_FLAG) {
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
                auto dst = read_op_val_word((Word**)&dst_addr);

                *(Word*)dst_addr = MEM_READ_DIR(cpu_context.regs[SP_REG_NDX].addr, Word);
                cpu_context.regs[SP_REG_NDX].addr += 2;
                break;
            }
            case Operation::OP_XCHG: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);

                    *(Byte*)dst_addr = src;
                    *(Byte*)src_addr = dst;
                } else {
                    auto src = read_op_val_word((Word**)&src_addr);
                    auto dst = read_op_val_word((Word**)&dst_addr);

                    *(Word*)dst_addr = src;
                    *(Word*)src_addr = dst;
                }

                break;
            }
            case Operation::OP_MOV: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
                    *(Byte*)dst_addr = src;

                    FILL_N_FLAG(*(Byte*)dst_addr);
                    FILL_Z_FLAG(*(Byte*)dst_addr);
                } else {
                    auto src = read_op_val_word((Word**)&src_addr);
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    *(Word*)dst_addr = src;

                    FILL_N_FLAG(*(Word*)dst_addr);
                    FILL_Z_FLAG(*(Word*)dst_addr);
                }

                break;
            }
            case Operation::OP_ADD: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
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

                    FILL_N_FLAG(*(Byte*)dst_addr);
                    FILL_Z_FLAG(*(Byte*)dst_addr);
                } else {
                    auto src = read_op_val_word((Word**)&src_addr);
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    
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
                    
                    FILL_N_FLAG(*(Word*)dst_addr);
                    FILL_Z_FLAG(*(Word*)dst_addr);
                }
                
                break;
            }
            case Operation::OP_SUB: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
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
                    
                    FILL_N_FLAG(*(Byte*)dst_addr);
                    FILL_Z_FLAG(*(Byte*)dst_addr);
                } else {
                    auto src = read_op_val_word((Word**)&src_addr);
                    auto dst = read_op_val_word((Word**)&dst_addr);
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
                    
                    FILL_N_FLAG(*(Word*)dst_addr);
                    FILL_Z_FLAG(*(Word*)dst_addr);
                }
                
                break;
            }
            case Operation::OP_MUL: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
                    *(Byte*)dst_addr *= src;
                    
                    FILL_N_FLAG(*(Byte*)dst_addr);
                    FILL_Z_FLAG(*(Byte*)dst_addr);
                } else {
                    auto src = read_op_val_word((Word**)&src_addr);
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    *(Word*)dst_addr *= src;
                    
                    FILL_N_FLAG(*(Word*)dst_addr);
                    FILL_Z_FLAG(*(Word*)dst_addr);
                }

                break;
            }
            case Operation::OP_DIV: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
                    *(Byte*)dst_addr /= src;

                    FILL_N_FLAG(*(Byte*)dst_addr);
                    FILL_Z_FLAG(*(Byte*)dst_addr);
                } else {
                    auto src = read_op_val_word((Word**)&src_addr);
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    *(Word*)dst_addr /= src;
                    
                    FILL_N_FLAG(*(Word*)dst_addr);
                    FILL_Z_FLAG(*(Word*)dst_addr);
                }

                break;
            }
            case Operation::OP_CMP: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
                    Byte temp = dst - src;
                    dst_addr == nullptr;
                    
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
            
                    FILL_N_FLAG(temp);
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

                    FILL_N_FLAG(temp);
                    FILL_Z_FLAG(temp);
                }

                break;
            }
            case Operation::OP_NOT: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
                    *(Byte*)dst_addr = ~src;

                    FILL_N_FLAG(*(Byte*)dst_addr);
                    FILL_Z_FLAG(*(Byte*)dst_addr);
                } else {
                    auto src = read_op_val_word((Word**)&src_addr);
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    *(Word*)dst_addr = ~src;

                    FILL_N_FLAG(*(Word*)dst_addr);
                    FILL_Z_FLAG(*(Word*)dst_addr);
                }

                break;
            }
            case Operation::OP_AND: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
                    *(Byte*)dst_addr &= src;
                    
                    FILL_N_FLAG(*(Byte*)dst_addr);
                    FILL_Z_FLAG(*(Byte*)dst_addr);
                } else {
                    auto src = read_op_val_word((Word**)&src_addr);
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    *(Word*)dst_addr &= src;
                    
                    FILL_N_FLAG(*(Word*)dst_addr);
                    FILL_Z_FLAG(*(Word*)dst_addr);
                }

                break;
            }
            case Operation::OP_OR: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
                    *(Byte*)dst_addr |= src;
                    
                    FILL_N_FLAG(*(Byte*)dst_addr);
                    FILL_Z_FLAG(*(Byte*)dst_addr);
                } else {
                    auto src = read_op_val_word((Word**)&src_addr);
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    *(Word*)dst_addr |= src;
                    
                    FILL_N_FLAG(*(Word*)dst_addr);
                    FILL_Z_FLAG(*(Word*)dst_addr);
                }

                break;
            }
            case Operation::OP_XOR: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
                    *(Byte*)dst_addr ^= src;
                    
                    FILL_N_FLAG(*(Byte*)dst_addr);
                    FILL_Z_FLAG(*(Byte*)dst_addr);
                } else {
                    auto src = read_op_val_word((Word**)&src_addr);
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    *(Word*)dst_addr ^= src;
                    
                    FILL_N_FLAG(*(Word*)dst_addr);
                    FILL_Z_FLAG(*(Word*)dst_addr);
                }

                break;
            }
            case Operation::OP_TEST: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
                    auto temp = dst & src;
                    
                    dst_addr == nullptr;

                    FILL_N_FLAG(temp);
                    FILL_Z_FLAG(temp);
                } else {
                    auto src = read_op_val_word((Word**)&src_addr);
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    auto temp = dst & src;
                    
                    dst_addr == nullptr;
                    
                    FILL_N_FLAG(temp);
                    FILL_Z_FLAG(temp);
                }
                
                break;
            }
            case Operation::OP_SHL: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte((Byte**)&src_addr);
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
                    
                    if ((src < 8) && ((dst >> (8-src)) & 1)) {
                        SET_C_FLAG;
                    } else {
                        CLR_C_FLAG;
                    }

                    *(Byte*)dst_addr <<= src;
                   
                    FILL_N_FLAG(*(Byte*)dst_addr);
                    FILL_Z_FLAG(*(Byte*)dst_addr);
                } else {
                    auto src = read_op_val_word((Word**)&src_addr);
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    
                    if ((src < 16) && ((dst >> (16-src)) & 1)) {
                        SET_C_FLAG;
                    } else {
                        CLR_C_FLAG;
                    }

                    *(Word*)dst_addr <<= src;

                    FILL_N_FLAG(*(Word*)dst_addr);
                    FILL_Z_FLAG(*(Word*)dst_addr);
                }

                break;
            }
            case Operation::OP_SHR: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto dst = read_op_val_byte((Byte**)&dst_addr);
                    auto src = read_op_val_byte((Byte**)&src_addr);

                    if ((dst >> (src - 1)) & 1) {
                        SET_C_FLAG;
                    } else {
                        CLR_C_FLAG;
                    }

                    *(Byte*)dst_addr >>= src;

                    FILL_N_FLAG(*(Byte*)dst_addr);
                    FILL_Z_FLAG(*(Byte*)dst_addr);
                } else {
                    auto dst = read_op_val_word((Word**)&dst_addr);
                    auto src = read_op_val_word((Word**)&src_addr);

                    if ((dst >> (src - 1)) & 1) {
                        SET_C_FLAG;
                    } else {
                        CLR_C_FLAG;
                    }

                    *(Word*)dst_addr >>= src;

                    FILL_N_FLAG(*(Word*)dst_addr);
                    FILL_Z_FLAG(*(Word*)dst_addr);
                }

                break;
            }
            default: {
                break;
            }
        }

        check_terminal(src_addr, dst_addr);

        if (I_FLAG == 0) {  
            if (Terminal::input_interrupt && TR_FLAG == 0) {
                cpu_context.regs[SP_REG_NDX].addr -= 2;
                MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Addr, cpu_context.regs[PC_REG_NDX].addr);

                cpu_context.regs[SP_REG_NDX].addr -= 2;
                MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Word, cpu_context.psw.word);
            
                cpu_context.regs[PC_REG_NDX].addr = MEM_READ_DIR(6, Addr);
                cpu_context.psw.word &= ~I_FLAG_MASK;
            }
        }
    }
}