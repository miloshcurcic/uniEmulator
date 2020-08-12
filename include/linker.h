#ifndef _LINKER_H_
#define _LINKER_H_

#include "includes.h"
#include "elf.h"

struct LST_Entry {
    Word value;
    string section_name;
};

struct LRT_Entry {
    Offs offs;
    Elf16_Rel_Type type;
    string section_name;
};

struct Input_File {
    unordered_map<string, vector<Byte>> data_sections;
    unordered_map<string, list<LRT_Entry>> section_relocations;
    unordered_map<string, LST_Entry> defined_symbols;
    unordered_map<string, list<LRT_Entry>> undefined_symbols;
    list<LRT_Entry> abs_relocations;
};

class Linker {
public:
    static Linker& get_instance() {
        static Linker instance;
        return instance;
    }

    Input_File* read_input_file(string path);
    void link_file(Input_File* file);
    void resolve_rel_entries(string sym_name, list<LRT_Entry> entries);
    void resolve_section_rels(string section_name, unordered_map<string, Addr> section_addresses);
    void resolve_abs_rels(unordered_map<string, Addr> section_addresses);
    vector<pair<Addr, vector<Byte>>> finalize_linking(vector<pair<string, Addr>> locs);
private:
    unordered_map<string, vector<Byte>> data_sections;
    unordered_map<string, list<LRT_Entry>> section_relocations;
    unordered_map<string, LST_Entry> defined_symbols;
    unordered_map<string, list<LRT_Entry>> undefined_symbols;
    list<LRT_Entry> abs_relocations;
};

#endif