#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>

#include "vm.h"

void load_args(int argc, const char* argv[]);

struct termios original_tio;

void disable_input_buffering();

void restore_input_buffering();

void handle_interrupt(int signal);

void setup();

void handle_trup(uint16_t instr);

int running = 1;

int main(int argc, const char* argv[])
{
    load_args(argc, argv);
    setup();

    reg[R_COND] = FL_ZRO;

    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    while (running)
    {
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op)
        {
            case OP_ADD:
                add(instr);
                break;
            case OP_AND:
                ando(instr);
                break;
            case OP_NOT:
                noto(instr);
                break;
            case OP_BR:
                br(instr);
                break;
            case OP_JMP:
                jmp(instr);
                break;
            case OP_JSR:
                jsr(instr);
                break;
            case OP_LD:
                ld(instr);
                break;
            case OP_LDI:
                ldi(instr);
                break;
            case OP_LDR:
                ldr(instr);
                break;
            case OP_LEA:
                lea(instr);
                break;
            case OP_ST:
                st(instr);
                break;
            case OP_STI:
                sti(instr);
                break;
            case OP_STR:
                str(instr);
                break;
            case OP_TRAP:
                trap(instr);
                break;
            case OP_RES:
                res(instr);
                break;
            case OP_RTI:
                rti(instr);
                break;
            default:
                break;
        }
    }


    return 0;
}


void load_args(int argc, const char* argv[])
{
    int j;
    if (argc < 2)
    {
        fprintf(stderr, "usage: lc3 [image-file1] ...\n");
        exit(2);
    }

    for (j = 1; j < argc; ++j) 
    {
        if (!read_image(argv[j]))
        {
            fprintf(stderr, "failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }
}

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

void setup()
{
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();
}
