#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include "includes.h"
#include <thread>
#include <semaphore.h>

struct termios;

class Terminal {
public:
    static void input_run();
    static void start_terminal();
    static void terminate();
    static void initialize_terminal();
    static void cleanup_terminal();
    static void continue_input();
    static void write_output();

    static bool input_interrupt;
private:
    static struct termios orig_termios;
    static bool running;
    static std::thread* input_thread;
    static sem_t input_lock;
};

#endif