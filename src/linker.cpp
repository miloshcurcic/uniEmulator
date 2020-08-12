#include "linker.h"
#include "includes.h"

Input_File* Linker::read_input_file(string path) {
    auto result = new Input_File();
    
    ifstream in_file(path, ios::in | ios::binary);
    
    if (in_file.is_open()) {
        Elf16_Header header;
        vector<Elf16_SH_Entry> section_headers;

        in_file.read((char*)&header, sizeof(Elf16_Header));
    
        auto data = new Byte[header.shentries * sizeof(Elf16_SH_Entry)];
        in_file.seekg(header.shoffs);
        in_file.read((char*)data, header.shentries * sizeof(Elf16_SH_Entry));
        section_headers.insert(section_headers.end(), (Elf16_SH_Entry*)data, (Elf16_SH_Entry*)data + header.shentries);

        auto str_tab_header = section_headers[header.strndx];
        auto str_tab_data = new Byte[str_tab_header.size];
        in_file.seekg(str_tab_header.offs);
        in_file.read((char*)str_tab_data, str_tab_header.size);

        auto sh_str_tab_header = section_headers[header.shstrndx];
        auto sh_str_tab_data = new Byte[sh_str_tab_header.size];
        in_file.seekg(sh_str_tab_header.offs);
        in_file.read((char*)sh_str_tab_data, sh_str_tab_header.size);

        auto sym_tab_header = section_headers[header.symtabndx];
        auto sym_tab_data = new Byte[sym_tab_header.size];
        in_file.seekg(sym_tab_header.offs);
        in_file.read((char*)sym_tab_data, sym_tab_header.size);

        for (uint i=0; i < sym_tab_header.size / sizeof(Elf16_ST_Entry); i++) {
            auto entry = ((Elf16_ST_Entry*)sym_tab_data)[i];
            if (entry.bind == Elf16_Sym_Bind::ESB_LOCAL) {
                continue;
            }

            if (entry.shndx != UND_NDX) {
                LST_Entry l_entry = {entry.value, string((char*)(sh_str_tab_data + section_headers[entry.shndx].name))};
                result->defined_symbols[string((char*)str_tab_data + entry.name)] = l_entry;
            } else {
                LST_Entry l_entry = {entry.value, ""};
                result->defined_symbols[string((char*)str_tab_data + entry.name)] = l_entry;
            
            }
        }

        for (auto& section : section_headers) {
            if (section.type != Elf16_Section_Type::EST_PROGBITS) {
                continue;
            }

            auto data = new Byte[section.size];
            in_file.seekg(section.offs);
            in_file.read((char*)data, section.size);
            result->data_sections[string((char*)(sh_str_tab_data + section.name))].insert(result->data_sections[string((char*)(sh_str_tab_data + section.name))].end(), data, data + section.size);
            
            if (section.rel != UND_NDX) {
                auto rel_section_header = section_headers[section.rel];

                auto rel_data = new Byte[rel_section_header.size];
                in_file.seekg(rel_section_header.offs);
                in_file.read((char*)rel_data, rel_section_header.size);

                for (uint i=0; i < rel_section_header.size / sizeof(Elf16_RT_Entry); i++) {
                    auto entry = ((Elf16_RT_Entry*)rel_data)[i];

                    if (entry.stndx != UND_NDX) {
                        auto st_entry = ((Elf16_ST_Entry*)sym_tab_data)[entry.stndx];
                        
                        if(st_entry.type == Elf16_Sym_Type::EST_SECTION) {
                            result->section_relocations[string((char*)str_tab_data + st_entry.name)].push_back({entry.offs, entry.type, string((char*)(sh_str_tab_data + section.name))});
                        } else {
                            if (st_entry.shndx != UND_NDX) {
                                // Error
                            } else {
                                result->undefined_symbols[string((char*)str_tab_data + st_entry.name)].push_back({entry.offs, entry.type, string((char*)(sh_str_tab_data + section.name))});
                            }
                        }
                    } else {
                        abs_relocations.push_back({entry.offs, entry.type, string((char*)(sh_str_tab_data + section.name))});    
                    }
                }

                delete[] rel_data;
            }
            
            delete[] data;
        }

        delete[] str_tab_data;
        delete[] sym_tab_data;
        delete[] sh_str_tab_data;
    } else {
        cout << "Error";
    }

    in_file.close();
    return result;
}

void Linker::resolve_rel_entries(string sym_name, list<LRT_Entry> entries) {
    auto symbol = defined_symbols[sym_name];

    for (auto entry : entries) {
        if (entry.type == Elf16_Rel_Type::ERT_PC16) {
            if (symbol.section_name == entry.section_name) {
                (Addr&)*(data_sections[entry.section_name].data() + entry.offs) += symbol.value - entry.offs;
            } else {
                (Addr&)*(data_sections[entry.section_name].data() + entry.offs) += symbol.value;

                if (symbol.section_name != "") {
                    section_relocations[symbol.section_name].push_back({entry.offs, entry.type, entry.section_name});
                } else {
                    abs_relocations.push_back({entry.offs, entry.type, ""});
                }
            }
        } else {
            if (symbol.section_name != "") {
                section_relocations[symbol.section_name].push_back({entry.offs, entry.type, entry.section_name});
            }

            if (entry.type == Elf16_Rel_Type::ERT_8) {
                (Byte&)*(data_sections[entry.section_name].data() + entry.offs) += symbol.value;
            } else {
                (Word&)*(data_sections[entry.section_name].data() + entry.offs) += symbol.value;
            }
        }
    }
}

void Linker::link_file(Input_File* file) {
    unordered_map<string, Offs> section_offsets;

    for(auto& section : file->data_sections) {
        if (data_sections.find(section.first) != data_sections.end()) {
            section_offsets[section.first] = data_sections[section.first].size();
            data_sections[section.first].insert(data_sections[section.first].end(), section.second.begin(), section.second.end());
        } else {
            data_sections[section.first] = section.second;
        }
    }

    for (auto& symbol : file->defined_symbols) {
        if (defined_symbols.find(symbol.first) != defined_symbols.end()) {
            // Error
        } else {
            defined_symbols[symbol.first] = symbol.second;

            if (section_offsets.find(symbol.second.section_name) != section_offsets.end()) {
                defined_symbols[symbol.first].value += section_offsets[symbol.first];
            }

            if (undefined_symbols.find(symbol.first) != undefined_symbols.end()) {
                resolve_rel_entries(symbol.first, undefined_symbols[symbol.first]);
                
                undefined_symbols.erase(symbol.first);
            }
        }
    }

    for (auto& symbol : file->undefined_symbols) {
        if (defined_symbols.find(symbol.first) != defined_symbols.end()) {
            resolve_rel_entries(symbol.first, symbol.second);
        } else {
            if (undefined_symbols.find(symbol.first) != undefined_symbols.end()) {
                undefined_symbols[symbol.first].insert(undefined_symbols[symbol.first].end(), symbol.second.begin(), symbol.second.end());
            } else {
                undefined_symbols[symbol.first] = symbol.second;
            }
        }
    }

    for (auto& section_rel : file->section_relocations) {
        if (section_offsets.find(section_rel.first) != section_offsets.end()) {
            for (auto& rel : section_rel.second) {
                rel.offs += section_offsets[section_rel.first];
            }
            
            section_relocations[section_rel.first].insert(section_relocations[section_rel.first].end(), section_rel.second.begin(), section_rel.second.end());
        } else {
            section_relocations[section_rel.first] = section_rel.second;
        }
    }

    for (auto& abs_rel : file->abs_relocations) {
        if (section_offsets.find(abs_rel.section_name) != section_offsets.end()) {
            abs_rel.offs += section_offsets[abs_rel.section_name];
        }
        
        abs_relocations.push_back(abs_rel);
    }
}


void Linker::resolve_section_rels(string section_name, unordered_map<string, Addr> section_addresses) {
    for (auto& entry : section_relocations[section_name]) {
        switch (entry.type) {
            case Elf16_Rel_Type::ERT_PC16: {
                (Addr&)*(data_sections[entry.section_name].data() + entry.offs) += section_addresses[section_name] - entry.offs - section_addresses[entry.section_name];               
                break;
            }
            case Elf16_Rel_Type::ERT_16: {
                 (Word&)*(data_sections[entry.section_name].data() + entry.offs) += section_addresses[section_name];               
                break;
            }
            case Elf16_Rel_Type::ERT_8: {
                (Byte&)*(data_sections[entry.section_name].data() + entry.offs) += section_addresses[section_name];               
                break;
            }
        }
    }

    section_relocations.erase(section_name);
}

void Linker::resolve_abs_rels(unordered_map<string, Addr> section_addresses) {
    for (auto& entry : abs_relocations) {
        (Addr&)*(data_sections[entry.section_name].data() + entry.offs) -= entry.offs + section_addresses[entry.section_name];                      
    }
}

bool compare_locs(pair<string, Addr> p1, pair<string, Addr> p2) {
    return (p1.second < p2.second);
}

vector<pair<Addr, vector<Byte>>> Linker::finalize_linking(vector<pair<string, Addr>> locs) {
    if (undefined_symbols.size() > 0) {
        // Error
    }

    unordered_map<string, Addr> section_addresses;

    sort(locs.begin(), locs.end(), compare_locs);
    Addr last_addr = 0;

    for (auto& loc : locs) {
        if (data_sections.find(loc.first) == data_sections.end()) {
            // Error
        }
        
        if (last_addr <= loc.second) {
            last_addr = loc.second;
        } else {
            // Error overlapping
        }

        section_addresses[loc.first] = last_addr;

        if (last_addr + data_sections[loc.first].size() > 0xffff) {
            last_addr += data_sections[loc.first].size();
        } else {
            // Error overflow
        }        
    }

    for (auto& section: data_sections) {
        if (section_addresses.find(section.first) == section_addresses.end()) {
            if (last_addr + section.second.size() > 0xffff) {
                // Error overflow
            }

            section_addresses[section.first] = last_addr;
            last_addr += section.second.size();
        }
    }

    for (auto& section : data_sections) {
        resolve_section_rels(section.first, section_addresses);
    }

    resolve_abs_rels(section_addresses);

    vector<pair<Addr, vector<Byte>>> res;

    for (auto &section : data_sections) {
        res.push_back({section_addresses[section.first], section.second});
    }

    return res;
}