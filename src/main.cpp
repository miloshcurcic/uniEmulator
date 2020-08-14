#include <iostream>
#include "linker.h"
#include "utility.h"
#include "emulator.h"
#include "terminal.h"
#include "eml_exception.h"

using namespace std;

#define UND_NDX 0

int main(int argc, char *argv[]) {
    try {
        vector<pair<string, Addr>> locs;
        
        for (int i=1; i<argc; i++) {
            if (argv[i][0] == '-') {
                auto cmd = string(argv[i] + 1);
                auto delimeter = cmd.find('=');
    
                if (delimeter == string::npos) {
                    throw EmulatorException(ERR_INVALID_INPUT_PARAMETER);
                }
    
                auto name = cmd.substr(0, delimeter);
    
                if (name == "place") {
                    auto position = cmd.substr(delimeter + 1, cmd.size() - delimeter - 1);
                    auto delimeter = position.find('@');
    
                    if (delimeter == string::npos) {
                        throw EmulatorException(ERR_INVALID_INPUT_PARAMETER);
                    }
    
                    auto section = position.substr(0, delimeter);
                    auto literal = position.substr(delimeter + 1, position.size() - delimeter - 1);
                    auto val = Utility::cast_literal(literal);
    
                    locs.push_back({section, val});
                } else {
                    string error_message = format_string(ERR_INVALID_INPUT_PARAMETER, name);
                    throw EmulatorException(error_message);
                }
            } else { 
                auto file = Linker::get_instance().read_input_file(argv[i]);
                Linker::get_instance().link_file(file);
                delete file;
            }
        }
    
        auto data_vector = Linker::get_instance().finalize_linking(locs);
        Emulator::load_data(data_vector);
        Emulator::initialize();
        Terminal::initialize_terminal();
        Emulator::run();
        Terminal::terminate();
    } catch (EmulatorException& e) {
        cout << "ERROR: " << e << endl;
    }
}