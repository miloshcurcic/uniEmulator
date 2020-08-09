#ifndef _ELF_H_
#define _ELF_H_

#include "includes.h"

typedef unsigned short Elf16_Addr;
typedef unsigned short Elf16_Offs;
typedef short Elf16_Word;
typedef unsigned short Elf16_UWord;
typedef char Elf16_Half;
typedef unsigned char Elf16_UHalf;
typedef unsigned char Elf16_Byte;

enum Elf16_File_Type : Elf16_Byte {
    EFT_REL,
    EFT_EXEC
};

struct Elf16_Header {
    Elf16_File_Type type;
    Elf16_Addr pentry;
    Elf16_Offs phoffs;
    Elf16_UWord phentries;
    Elf16_Offs shoffs;
    Elf16_UWord shentries;
    Elf16_UWord shstrndx;
};

enum Elf16_Section_Type : Elf16_Byte {
    EST_UND,
    EST_PROGBITS,
    EST_SYMTAB,
    EST_STRTAB,
    EST_REL
};

enum Elf16_Section_Flag : Elf16_Byte {
    ESF_WRITE,
    ESF_ALLOC,
    ESF_EXECINSTR
};

struct Elf16_SH_Entry {
    Elf16_UWord name;
    Elf16_Section_Type type;
    Elf16_Byte flags;
    Elf16_Offs offs;
    Elf16_UWord size;
    Elf16_UWord rel;
};

enum Elf16_Sym_Link : Elf16_Byte {
    ESL_LOCAL,
    ESL_GLOBAL
};

struct Elf16_ST_Entry {
    Elf16_UWord name;
    Elf16_UWord value;
    Elf16_Sym_Link link;
    Elf16_UWord shndx;
};

enum Elf16_Rel_Type : Elf16_Byte {
    ERT_8,
    ERT_16,
    ERT_PC16,
};

struct Elf16_RT_Entry {
    Elf16_Addr offs;
    Elf16_Rel_Type type;
    Elf16_UWord stndx;
};

#endif