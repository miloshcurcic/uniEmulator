#ifndef _LINKER_H_
#define _LINKER_H_

#include "includes.h"
#include "elf.h"

struct Input_File {
    Elf16_Header header;
    vector<vector<Byte>> sections;
    vector<Elf16_SH_Entry> section_headers;
};

class Linker {
    public:
    static Linker& get_instance() {
        static Linker instance;
        return instance;
    }

    Input_File* read_input_file(string path);
};

#endif