#ifndef _ELF_H_
#define _ELF_H_

#include "includes.h"

#define ELF_FORMAT_LEN 6
#define UND_NDX 0
#define ABS_NDX 0xffff

enum Elf16_File_Type : Byte {
    EFT_REL,
    EFT_EXEC
};

struct Elf16_Header {
    Byte format[ELF_FORMAT_LEN] = { 'E', 'L', 'F', '_', '1', '6' };
    Elf16_File_Type type;
    Offs phoffs;
    Word phentries;
    Offs shoffs;
    Word shentries;
    Word symtabndx;
    Word strndx;
    Word shstrndx;
};

enum Elf16_Section_Type : Byte {
    EST_UND,
    EST_PROGBITS,
    EST_SYMTAB,
    EST_STRTAB,
    EST_REL
};

enum Elf16_Section_Flag : Byte {
    ESF_WRITE,
    ESF_ALLOC,
    ESF_EXECINSTR
};

struct Elf16_SH_Entry {
    Offs name;
    Elf16_Section_Type type;
    Offs offs;
    Word size;
    Word rel;
};

enum Elf16_Sym_Bind : Byte {
    ESB_LOCAL,
    ESB_GLOBAL
};

enum Elf16_Sym_Type : Byte {
    EST_NOTYPE,
    EST_SECTION
};

struct Elf16_ST_Entry {
    Offs name;
    Word value;
    Elf16_Sym_Bind bind;
    Elf16_Sym_Type type;
    Word shndx;
};

enum Elf16_Rel_Type : Byte {
    ERT_8,
    ERT_16,
    ERT_PC16,
};

struct Elf16_RT_Entry {
    Offs offs;
    Elf16_Rel_Type type;
    Word stndx;
};

#endif