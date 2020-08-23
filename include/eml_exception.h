#ifndef _EML_EXCEPTION_H_
#define _EML_EXCEPTION_H_

#include <iostream>
#include <string>
#include <memory>
#include <string>
#include <stdexcept>
#include "terminal.h"

using namespace std;

const string ERR_OPEN_IN_FILE_FAILED = "Opening input file failed.";
const string ERR_INVALID_INPUT_FORMAT = "Invalid input file format.";
const string ERR_DEFINED_SYBOL_REL_IN_INPUT = "Unsupported scenario: Encountered REL entry for a defined symbol in input.";
const string ERR_UND_REL_IN_INPUT = "Relocation of UND symbol encountered in input.";
const string ERR_SYMBOL_ALREADY_DEFINED = "Multiple definitions of symbol '%s' encountered.";
const string ERR_UNKNOWN_REL_TYPE = "Unknown relocation type encountered.";
const string ERR_INVALID_REL_TYPE = "Invalid relocation type encountered.";
const string ERR_UND_SYM_NOT_RESOLVED = "There are unresolved undefined symbols ['%s', ...].";
const string ERR_UNKNOWN_SEC_POS_INPUT = "Position defined for unknown section '%s'.";
const string ERR_OVERLAPPING_SECTIONS = "There are overlapping sections in input.";
const string ERR_SECTION_OVERFLOW = "Sections are overflowing available memory.";
const string ERR_UNKNOWN_INPUT_PARAMETER = "Unknown input parameter '%s'";
const string ERR_INVALID_INPUT_PARAMETER = "Invalid input parameter.";
const string ERR_TIMER_MISCONFIGURATION = "Invalid timer configuration.";
const string ERR_UNSYNCHRONIZED_TERMINAL_ACCESS = "Unsynchronized terminal access detected!";

class EmulatorException {
    string message;
public:
    EmulatorException(string message) : message(message) {
    }
    friend ostream& operator<<(ostream& os, const EmulatorException& error);
};

ostream& operator<<(ostream& os, const EmulatorException& error);

string format_string(const string& format, const string& text);

#endif