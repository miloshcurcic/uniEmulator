#include "utility.h"
#include <iostream>

Elf16_Word Utility::cast_literal(string literal) {
    /* Range check */
    if (literal[0] == '0') {
        if (literal.length() == 1) {
            return 0;
        } else if (literal[1] == 'x') {
            return stoi(literal.substr(2), nullptr, 16);
        } else if (literal[1] == 'b') {
            return stoi(literal.substr(2), nullptr, 2);
        } else {
            return stoi(literal.substr(1), nullptr, 8);
        }
    } else if (literal[0] == '\'') {
        return (Elf16_Word)literal[1];
    } else {
        return stoi(literal);
    }
}

bool Utility::is_literal(string value) {
    if ((value[0] >= '0' && value[0] <= '9') || value[0] == '\'') {
        return true;
    } else {
        return false;
    }
}

void Utility::print_section_headers(vector<Elf16_SH_Entry>& headers) {
    printf("\n#sections\n");
    // flags temp removed
    printf("#%6s %10s %6s %5s %6s %5s\n", "name", "type", "offs", "size", "rel", "ndx");
    
    const string type_names[] = { "UND", "PROGBITS", "SYMTAB", "STRTAB", "REL" };

    for (uint i = 0; i < headers.size(); i++) {
        printf(" 0x%04x %10s 0x%04x %5d 0x%04x %5d\n", headers[i].name, type_names[headers[i].type].c_str(), headers[i].offs, headers[i].size, headers[i].rel, i);
    }
    
    printf("\n");
}

void Utility::print_sym_tab(string name, vector<Elf16_ST_Entry>& section) {
    printf("\n#%s\n", name.c_str());
    printf("#%6s %6s %4s %5s %5s\n","name", "value", "link", "shndx", "ndx");
    for (uint i = 0; i < section.size(); i++) {
        printf(" 0x%04x 0x%04x %4c %5s %5d\n", section[i].name, section[i].value, section[i].link ? 'g' : 'l', section[i].shndx ? to_string(section[i].shndx).c_str() : "UND", i);
    }
    printf("\n");
}

void Utility::print_section(string name, vector<Elf16_Byte>& section) {
    printf("\n#%s\n", name.c_str());
    for (uint i = 0; i < section.size(); i++) {
        if (i != 0 && i % 12 == 0) {
            printf("\n");
        }

        if (i % 12 == 0) {
            printf("0x%04x: ", i);
        }

        printf("%02x ", section[i]);
    }
    printf("\n");
}

void Utility::print_str_tab(string name, vector<Elf16_Byte>& section) {
    printf("\n#%s\n", name.c_str());
    for (uint i = 0; i < section.size(); i++) {
        if (i != 0 && i % 25 == 0) {
            printf("\n");
        }

        if (i % 25 == 0) {
            printf("0x%04x: ", i);
        }

        printf("%c", section[i]);
    }
    printf("\n");
}

void Utility::print_rel_section(string name, vector<Elf16_RT_Entry>& section) {
    printf("\n#%s\n", name.c_str());
    printf("#%6s %8s %5s %5s\n", "offset", "type", "stndx", "ndx");
    
    const string type_names[] = { "ERT_8", "ERT_16", "ERT_PC16" };
    
    for (uint i = 0; i < section.size(); i++) {
        printf(" 0x%04x %8s %5d %5d\n", section[i].offs, type_names[section[i].type].c_str(), section[i].stndx, i);
    }
    printf("\n");
}

void Utility::print_file_header(string name, Elf16_Header& header) {
    printf("\n#%s\n", name.c_str());
    printf("#%6s %11s %11s\n", "sh_offs", "sh_entries", "sh_str_ndx");
    printf("  0x%04x %11d %11d\n", header.shoffs, header.shentries, header.shstrndx);
    printf("\n");
}