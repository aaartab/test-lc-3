#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

#define MEMORY_MAX (1 << 16)

extern uint16_t memory[MEMORY_MAX];

enum 
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,
    R_COND,
    R_COUNT
};

extern uint16_t reg[R_COUNT];

enum
{
    OP_BR = 0,
    OP_ADD,
    OP_LD,
    OP_ST,
    OP_JSR,
    OP_AND,
    OP_LDR,
    OP_STR,
    OP_RTI,
    OP_NOT,
    OP_LDI,
    OP_STI,
    OP_JMP,
    OP_RES,
    OP_LEA,
    OP_TRAP
};

enum 
{
    FL_POS = 1 << 0,
    FL_ZRO = 1 << 1,
    FL_NEG = 1 << 2,
};

enum
{
    TRAP_GETC = 0x20,
    TRAP_OUT = 0x21,
    TRAP_PUTS = 0x22,
    TRAP_IN = 0x23,
    TRAP_PUTSP = 0x24,
    TRAP_HALT = 0x25
};

enum
{
    MR_KBSR = 0xFE00,
    MR_KBDR = 0xFE02
};

uint16_t sign_extend(uint16_t x, int bit_count);

void update_flags(uint16_t r);

void add(uint16_t instr);

void ldi(uint16_t instr);

void ld(uint16_t instr);

void st(uint16_t instr);

void jsr(uint16_t instr);

void ando(uint16_t instr);

void ldr(uint16_t instr);

void str(uint16_t instr);

void rti(uint16_t instr);

void res(uint16_t instr);

void noto(uint16_t instr);

void sti(uint16_t instr);

void jmp(uint16_t instr);

void lea(uint16_t instr);

void trap(uint16_t instr);

void br(uint16_t instr);

void trap_puts();

void trap_getc();

void trap_putsp();

void trap_out();

void trap_in();

void trap_halt();

void read_image_file(FILE* file);

int read_image(const char* image_path);

void mem_write(uint16_t address, uint16_t val);

uint16_t mem_read(uint16_t address);
