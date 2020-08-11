#include "linker.h"
#include "includes.h"

Input_File* Linker::read_input_file(string path) {
    auto result = new Input_File();
    
    ifstream in_file(path, ios::in | ios::binary);
    
    if (in_file.is_open()) {
        in_file.read((char*)&result->header, sizeof(Elf16_Header));
    
        auto data = new Byte[result->header.shentries * sizeof(Elf16_SH_Entry)];
        in_file.seekg(result->header.shoffs);
        in_file.read((char*)data, result->header.shentries * sizeof(Elf16_SH_Entry));
        result->section_headers.insert(result->section_headers.end(), (Elf16_SH_Entry*)data, (Elf16_SH_Entry*)data + result->header.shentries);
        delete[] data;

        for (auto& section : result->section_headers) {
            auto data = new Byte[section.size];
            in_file.seekg(section.offs);
            in_file.read((char*)data, section.size);
            result->sections.push_back(vector<Byte>(data, data+section.size));
            delete[] data;
        }
    } else {
        cout << "Error";
    }

    in_file.close();
    return result;
}