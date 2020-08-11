#include <iostream>
#include "linker.h"
#include "utility.h"

using namespace std;

#define UND_NDX 0

int main(int argc, char *argv[]) {
    for (int i=1; i<argc; i++) {
        auto test = Linker::get_instance().read_input_file(argv[i]);
    }
}