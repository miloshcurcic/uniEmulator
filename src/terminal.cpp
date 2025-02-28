#include "terminal.h"

#include <termios.h>
#include <unistd.h>
#include "emulator.h"
#include "instruction.h"

struct termios Terminal::orig_termios;
bool Terminal::running = false;
sem_t Terminal::input_lock;
std::thread* Terminal::input_thread = nullptr;
bool Terminal::input_interrupt = false;

void Terminal::cleanup_terminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &Terminal::orig_termios);
}

void Terminal::initialize_terminal() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    static struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
    raw.c_cflag &= ~(CSIZE | PARENB);
    raw.c_cflag |= CS8;
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;

    atexit(Terminal::cleanup_terminal);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    
    sem_init(&input_lock, 0, 0);
    input_thread = new std::thread(input_run);
}

void Terminal::input_run() {
    running = true;
    while (running) {
        char c;
        read(STDIN_FILENO, &c, 1);
        MEM_WRITE_DIR(TERM_DATA_IN_ADDR, Byte, (Byte)c);
        input_interrupt = true;
        sem_wait(&input_lock);
    }
}

void Terminal::continue_input() {
    // This can be spammed but that is a programming error, potentially throw an exception as that can cause terminal corruption?
    sem_post(&input_lock);
}

void Terminal::write_output() {
    char c = (char)MEM_READ_DIR(TERM_DATA_OUT_ADDR, Byte);
    write(STDOUT_FILENO, &c, 1);
}

void Terminal::terminate() {
    if (running) {
        running = false;
        sem_post(&input_lock);
        input_thread->join();
        delete input_thread;
        sem_destroy(&input_lock);
        Terminal::cleanup_terminal();
    }
}
