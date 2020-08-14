#include "eml_exception.h"

ostream& operator<<(ostream& os, const EmulatorException& error) {
    return os << error.message;
}

string format_string(const string& format, const string& text)
{
    char* b;
    if (asprintf(&b, format.c_str(), text.c_str()) == -1) {
        throw runtime_error("Formatting failed.");
    }
    
    string result = string(b);

    return result;
}