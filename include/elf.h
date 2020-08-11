#ifndef _ELF_H_
#define _ELF_H_

#include "includes.h"

#define UND_NDX 0
#define ABS_NDX 0xffff

enum Elf16_File_Type : Byte {
    EFT_REL,
    EFT_EXEC
};

struct Elf16_Header {
    Elf16_File_Type type;
    Addr pentry;
    Offs phoffs;
    Word phentries;
    Offs shoffs;
    Word shentries;
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
    Word name;
    Elf16_Section_Type type;
    Byte flags;
    Offs offs;
    Word size;
    Word link;
};

enum Elf16_Sym_Link : Byte {
    ESL_LOCAL,
    ESL_GLOBAL
};

struct Elf16_ST_Entry {
    Word name;
    Word value;
    Elf16_Sym_Link link;
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