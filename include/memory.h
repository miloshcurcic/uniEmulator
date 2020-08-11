#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "elf.h"

#define MEMORY_SIZE 65280

extern Byte memory[MEMORY_SIZE];

#define MEM_READ_DIR(addr, type) *(type*)(memory + addr)
#define MEM_READ_IND(addr, type) *(type*)(memory + *((Addr*)(memory + addr)))
#define MEM_WRITE_DIR(addr, type, val) *(type*)(memory + addr) = val
#define MEM_WRITE_IND(addr, type, val) *(type*)(memory + *((Addr*)(memory + addr))) = val

#endif