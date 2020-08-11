#include "emulator.h"
#include "cpu_context.h"
#include "../include/memory.h"
#include "instruction.h"

void run() {
    bool running = true;
    while (running) {
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
                auto dst = read_op_val_word();

                cpu_context.regs[SP_REG_NDX].addr -= 2;
                MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Addr, cpu_context.regs[PC_REG_NDX].addr);
              
                cpu_context.regs[SP_REG_NDX].addr -= 2;
                MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Word, cpu_context.psw.word);

                cpu_context.regs[PC_REG_NDX].addr = MEM_READ_DIR((dst % 8)*2, Addr);
                break;
            }
            case Operation::OP_CALL: {
                auto dst = read_op_val_word();
                
                cpu_context.regs[SP_REG_NDX].addr -= 2;
                MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Addr, cpu_context.regs[PC_REG_NDX].addr);
              
                cpu_context.regs[PC_REG_NDX].addr = dst;
                break;
            }
            case Operation::OP_JMP: {
                auto addr = read_op_addr();
                cpu_context.regs[PC_REG_NDX].addr = addr;

                break;
            }
            case Operation::OP_JEQ: {
                auto addr = read_op_addr();

                if (Z_FLAG) {
                    cpu_context.regs[PC_REG_NDX].addr = addr;
                }
                break;
            }
            case Operation::OP_JNE: {
                auto addr = read_op_addr();

                if (!Z_FLAG) {
                    cpu_context.regs[PC_REG_NDX].addr = addr;
                }
                break;
            }
            case Operation::OP_JGT: {
                auto addr = read_op_addr();

                if (~(N_FLAG^O_FLAG) & ~Z_FLAG) {
                    cpu_context.regs[PC_REG_NDX].addr = addr;
                }
                break;
            }
            case Operation::OP_PUSH: {
                auto src = read_op_val_word();
                
                cpu_context.regs[SP_REG_NDX].addr -= 2;
                MEM_WRITE_DIR(cpu_context.regs[SP_REG_NDX].addr, Word, src);
                break;
            }
            case Operation::OP_POP: {
                auto dst = read_op_ptr_word();

                *dst = MEM_READ_DIR(cpu_context.regs[SP_REG_NDX].addr, Word);
                cpu_context.regs[SP_REG_NDX].addr += 2;
                break;
            }
            case Operation::OP_XCHG: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_ptr_byte();
                    auto dst = read_op_ptr_byte();
                    auto temp = *src;
                    *dst = *src;
                    *src = temp;
                } else {
                    auto src = read_op_ptr_word();
                    auto dst = read_op_ptr_word();
                    auto temp = *src;
                    *dst = *src;
                    *src = temp;
                }

                break;
            }
            case Operation::OP_MOV: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte();
                    auto dst = read_op_ptr_byte();
                    *dst = src;
                    

                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                } else {
                    auto src = read_op_val_word();
                    auto dst = read_op_ptr_word();
                    *dst = src;


                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                }

                break;
            }
            case Operation::OP_ADD: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte();
                    auto dst = read_op_ptr_byte();
                    Byte res = src + *dst;

                    if (res < src) {
                        SET_O_FLAG;
                    } else {
                        CLR_O_FLAG;
                    }

                    if (((src ^ *dst ^ 0x80) & (res ^ src)) & 0x80) {
                        SET_C_FLAG;
                    } else {
                        CLR_C_FLAG;
                    }

                    *dst = res;

                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                } else {
                    auto src = read_op_val_word();
                    auto dst = read_op_ptr_word();
                    
                    Word res = src + *dst;
                    
                    if (res < src) {
                        SET_O_FLAG;
                    } else {
                        CLR_O_FLAG;
                    }

                    if (((src ^ *dst ^ 0x8000) & (res ^ src)) & 0x8000) {
                        SET_C_FLAG;
                    } else {
                        CLR_C_FLAG;
                    }
                    
                    *dst = res;

                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                }
                
                break;
            }
            case Operation::OP_SUB: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte();
                    auto dst = read_op_ptr_byte();
                    Byte res = *dst - src;

                    if ((src ^ *dst) & (*dst ^ res) & 0x80) {
                        SET_O_FLAG;
                    } else {
                        CLR_O_FLAG;
                    }
                    
                    if (*dst < src) {
                        SET_C_FLAG;
                    } else {
                        CLR_C_FLAG;
                    }

                    *dst = src;
                    
                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                } else {
                    auto src = read_op_val_word();
                    auto dst = read_op_ptr_word();
                    Word res = *dst - src;

                    if ((src ^ *dst) & (*dst ^ res) & 0x8000) {
                        SET_O_FLAG;
                    } else {
                        CLR_O_FLAG;
                    }
                    
                    if (*dst < src) {
                        SET_C_FLAG;
                    } else {
                        CLR_C_FLAG;
                    }

                    *dst = src;
                    
                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                }
                
                break;
            }
            case Operation::OP_MUL: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte();
                    auto dst = read_op_ptr_byte();
                    *dst *= src;
                    
                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                } else {
                    auto src = read_op_val_word();
                    auto dst = read_op_ptr_word();
                    *dst *= src;
                    
                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                }

                break;
            }
            case Operation::OP_DIV: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte();
                    auto dst = read_op_ptr_byte();
                    *dst /= src;

                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                } else {
                    auto src = read_op_val_word();
                    auto dst = read_op_ptr_word();
                    *dst /= src;
                    
                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                }

                break;
            }
            case Operation::OP_CMP: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte();
                    auto dst = read_op_val_byte();
                    Byte temp = dst - src;

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
                    auto src = read_op_val_word();
                    auto dst = read_op_val_word();
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
                    auto src = read_op_val_byte();
                    auto dst = read_op_ptr_byte();
                    *dst = ~src;

                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                } else {
                    auto src = read_op_val_word();
                    auto dst = read_op_ptr_word();
                    *dst = ~src;

                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                }

                break;
            }
            case Operation::OP_AND: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte();
                    auto dst = read_op_ptr_byte();
                    *dst &= src;

                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                } else {
                    auto src = read_op_val_word();
                    auto dst = read_op_ptr_word();
                    *dst &= src;

                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                }

                break;
            }
            case Operation::OP_OR: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte();
                    auto dst = read_op_ptr_byte();
                    *dst |= src;
                    
                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                } else {
                    auto src = read_op_val_word();
                    auto dst = read_op_ptr_word();
                    *dst |= src;
                    
                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                }

                break;
            }
            case Operation::OP_XOR: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte();
                    auto dst = read_op_ptr_byte();
                    *dst ^= src;
                    
                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                } else {
                    auto src = read_op_val_word();
                    auto dst = read_op_ptr_word();
                    *dst ^= src;
                    
                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                }

                break;
            }
            case Operation::OP_TEST: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte();
                    auto dst = read_op_val_byte();
                    auto temp = dst & src;

                    FILL_N_FLAG(temp);
                    FILL_Z_FLAG(temp);
                } else {
                    auto src = read_op_val_byte();
                    auto dst = read_op_val_byte();
                    auto temp = dst & src;

                    FILL_N_FLAG(temp);
                    FILL_Z_FLAG(temp);
                }
                
                break;
            }
            case Operation::OP_SHL: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto src = read_op_val_byte();
                    auto dst = read_op_ptr_byte();
                    
                    if ((src < 8) && ((*dst >> (8-src)) & 1)) {
                        SET_C_FLAG;
                    } else {
                        CLR_C_FLAG;
                    }

                    *dst <<= src;

                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                } else {
                    auto src = read_op_val_word();
                    auto dst = read_op_ptr_word();
                    
                    if ((src < 16) && ((*dst >> (16-src)) & 1)) {
                        SET_C_FLAG;
                    } else {
                        CLR_C_FLAG;
                    }

                    *dst <<= src;

                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                }

                break;
            }
            case Operation::OP_SHR: {
                if (ins_descr.op_bytes == OpBytes::OB_ONE) {
                    auto dst = read_op_ptr_byte();
                    auto src = read_op_val_byte();

                    if ((*dst >> (src - 1)) & 1) {
                        SET_C_FLAG;
                    } else {
                        CLR_C_FLAG;
                    }

                    *dst >>= src;

                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                } else {
                    auto src = read_op_val_word();
                    auto dst = read_op_ptr_word();

                    if ((*dst >> (src - 1)) & 1) {
                        SET_C_FLAG;
                    } else {
                        CLR_C_FLAG;
                    }

                    *dst >>= src;

                    FILL_N_FLAG(*dst);
                    FILL_Z_FLAG(*dst);
                }

                break;
            }
            default: {
                break;
            }
        }
    }
}