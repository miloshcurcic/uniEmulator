#include <iostream>
#include "linker.h"
#include "utility.h"

using namespace std;

#define UND_NDX 0

int main(int argc, char *argv[]) {
    for (int i=1; i<argc; i++) {
        auto test = Linker::get_instance().read_input_file(argv[i]);
        
        Utility::print_file_header("out.o", test->header);
        Utility::print_section_headers(test->section_headers);

        for (uint i = 0; i<test->sections.size(); i++) {
            if (test->section_headers[i].type == Elf16_Section_Type::EST_UND) {
                continue;
            } else if (test->section_headers[i].type == Elf16_Section_Type::EST_SYMTAB) {
                Utility::print_sym_tab(string((char*)test->sections[test->header.shstrndx].data() + test->section_headers[i].name), (vector<Elf16_ST_Entry>&)test->sections[i]);
            } else if (test->section_headers[i].type == Elf16_Section_Type::EST_STRTAB) {
                Utility::print_str_tab(string((char*)test->sections[test->header.shstrndx].data() + test->section_headers[i].name), test->sections[i]);
            } else if (test->section_headers[i].type == Elf16_Section_Type::EST_REL) {
                Utility::print_rel_section(string((char*)test->sections[test->header.shstrndx].data() + test->section_headers[i].name), (vector<Elf16_RT_Entry>&)test->sections[i]);
            } 
            else {
                Utility::print_section(string((char*)test->sections[test->header.shstrndx].data() + test->section_headers[i].name), test->sections[i]);
            }
        }
    }
}